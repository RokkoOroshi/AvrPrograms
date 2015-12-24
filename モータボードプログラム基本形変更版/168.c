/*
これはDCモーターを制御するためのプログラム。
TPIPのPWMとDCモーターのPWMでは波形の周期が違うので、それを変換するためのもの。
最初の方は必要なのかいまいちよくわからない
シリアル通信するときに必要なものだろうか？

AVRにはPWM波形出力機能がある
詳しくは以下のサイトでも
http://usicolog.nomaki.jp/engineering/avr/avrPWM.html

PWM波形出力機能を利用するには、
TCCR0AとTCCR0B、TCCR1AとTCCR1Bの設定が必要

コンペアマッチという概念があり、指定値とタイマの値が一致した時に波形をHIGH、LOW、反転させることでPWM制御ができる
指定値はOCR○A,Bで設定される
その値を超えたらTOPになるまでPWM信号は0を出力し、その後、タイマは0に戻る

分周　…　電波の周波数を1/nにすること（nは整数）

1つのマイコンで2つのモーターを制御する
PWMの出力端子は決まっていて、
1つ目はOCR0A(PD6),OCR0B(PD5)
2つ目はOCR1A(PB1),OCR1B(PB2)
それぞれモータードライバの入力端子と繋がっている
以下は入力によるDCモーターの挙動
H	H	ブレーキ
H	L	正転(逆転)
L	H	逆転(正転)
L	L	ストップ



*/
#include <avr/io.h>
#include <stdio.h>
#define F_CPU 8000000UL
#include <util/delay.h>
//FOSC	DDSの発振周波数データ、４バイト 今回は8MHz
#define FOSC 8000000
/*
BAUD	ボーレートのこと
ボーレートとは、デジタルデータを1秒間に何回だけ変復調できるかを示す値
ボーレートの値が大きければ大きいほど、同じ時間内により多くの情報を転送することが可能なことを意味する
*/
#define BAUD 19200
//よくわからない
#define MYUBRR FOSC/16/BAUD-1

//自前で定義したもの
#define SVPN1 250
#define SVPN2 250
#define SVPO  30
FILE *fp;

/*
マクロ使用例
PORTB = ( 1 << PINB5 ); 　　　// PB5ピンだけをHighにし、残りをLowにする
例）
PORTB = 0xFF; 　　　　　　　//元 #1111 1111　(0xFF
PORTB = ( 1 << PINB5 ); 　→　　#0010 0000　(0x20
*/

void print_chr(unsigned char c)
{
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = c;
}


//タイマー0の初期化・設定関数
void TMR0_Init(void)
{
	/*
	1を左にCOM0A1ビット分(つまり7bit、そのようにAVRで定義されている)シフトしている
	COM0B1は5bit、WGM00は0bitなのでTCCR0A = 0b10100011と同じ意味
	今回はコンペアマッチでLow、高速PWM、分周1/8、TOP値0xffの設定
	コンペアマッチの指定値は以下の通り
	*/
	TCCR0A = (1 << COM0A1) | (1 << COM0B1) | (3 << WGM00);
	TCCR0B = (1 << CS01);
	OCR0A = 0xff;
	OCR0B = 0xff;
}

//タイマー1の初期化・設定関数
void TMR1_Init(void)
{
	/*
	コンペアマッチでLow、(8bit)高速PWM、分周1/8、TOP値0xff
	要は上と同じ設定
	*/
	TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM10);
	TCCR1B = (1 << WGM12) | (1 << CS11);
	OCR1A = 0xff;
	OCR1B = 0xff;
}

void USART_Init(int baud)
{
	UBRR0H = (char)(baud >> 8);
	UBRR0L = (char)baud;
	UCSR0C = (3 << UCSZ00);
	UCSR0B = (1 << TXEN0);
}

int main(void)
{
	//t1,t2はTPIPからのPWMのH,Lを時間として代入する変数
	long t1, t2;

	//cnt1,cnt2はt1,t2の値を元に、DCモーターのPWMに変換した値を格納する変数
	int cnt1 = 0, cnt2 = 0;
	//m1d,m2dはモーターの正転、逆転を1,2で表す変数
	int m1d = 0, m2d = 0;
	//PB6,PB7を入力にする この2つにTPIPのPWMを入力する
	DDRD = 0b11111111;
	DDRB = 0b00111111;
	//PC0～PC3を入力にする リミットスイッチの入力用
	DDRC = 0xf0;
	TMR0_Init();
	TMR1_Init();
	//	USART_Init(MYUBRR);
	//	fp = fdevopen(print_chr, NULL);
	//	printf("Out the cntdata.\r\n");			
	while (1)
	{

		cnt1 = 0;
		cnt2 = 0;

		t1 = 0;
		t2 = 0;

		//SVP1
		//下3行の処理はPWMを途中から測定してしまうことを防ぐためのもの
		if (bit_is_set(PINB, PB6))
			while (bit_is_set(PINB, PB6));
		while (bit_is_clear(PINB, PB6));
		while (bit_is_set(PINB, PB6)){	//HIGHの時間を測定
			t1++;
		}
		while (bit_is_clear(PINB, PB6)){		//LOWの時間を測定
			t2++;
		}
		//変換
		cnt1 = ((((500 * t1) / (t1 + t2)) * 40) / 3) - 250;

		t1 = 0;
		t2 = 0;
		if (bit_is_set(PINB, PB7))
			while (bit_is_set(PINB, PB7));
		while (bit_is_clear(PINB, PB7));
		while (bit_is_set(PINB, PB7)){	//HIGHの時間を測定
			t1++;
		}
		while (bit_is_clear(PINB, PB7)){		//LOWの時間を測定
			t2++;
		}
		cnt2 = ((((500 * t1) / (t1 + t2)) * 40) / 3) - 250;

		//Motor1
		/*
		リミットスイッチの使用例
		リミットスイッチが押された時、それ以上進まないようにする(cnt1が基準を超えた時、cnt1 = SVPN1、つまりストップ状態にする)
		*/
		if (bit_is_clear(PINC, PC3)){
			if (cnt1 < SVPN1 - SVPO)
				cnt1 = SVPN1;
		}
		if (bit_is_clear(PINC, PC2)){
			if (cnt1 > SVPN1 - SVPO)
				cnt1 = SVPN1;
		}
		//CW
		if (cnt1 > SVPN1 + SVPO)		//280<cnt1<500
		{
			//モーターの回転方向が入力値と反対だった場合、一旦ブレーキをかける
			if (m1d == 2)			//CCW--Brake
			{
				OCR0A = 0xff;
				OCR0B = 0xff;
			}

			cnt1 = cnt1 - SVPN1;	//30<cnt1<250
			//cntが最大値を超えないようにする(OCR0A,Bは8bitなので)
			if (cnt1 > 0xff)			//cnt1=max
				cnt1 = 0xff;
			//計算結果を代入する　OCR0A,Bの値を元にPWMが生成され、モータードライバに出力される
			OCR0A = cnt1;
			OCR0B = 0;
			//現在の回転方向を記録する
			m1d = 1;
		}

		//CCW
		else if (cnt1 < SVPN1 - SVPO)	//0<cnt1<220
		{
			if (m1d == 1)
			{
				OCR0A = 0xff;
				OCR0B = 0xff;
			}
			cnt1 = SVPN1 - cnt1;
			if (cnt1 > 0xff)
				cnt1 = 0xff;
			OCR0A = 0;
			OCR0B = cnt1;
			m1d = 2;
		}

		//brake 230<cnt1<280
		else
		{
			cnt1 = 0;
			OCR0A = 0;
			OCR0B = 0;
		}


		//Motor2
		if (bit_is_clear(PINC, PC1)){
			if (cnt2 < SVPN2 - SVPO)
				cnt2 = SVPN2;
		}
		if (bit_is_clear(PINC, PC0)){
			if (cnt2 > SVPN2 - SVPO)
				cnt2 = SVPN2;
		}
		if (cnt2 > SVPN2 + SVPO)
		{
			if (m2d == 2)
			{
				OCR1A = 0xff;
				OCR1B = 0xff;
			}
			cnt2 = cnt2 - SVPN2;
			if (cnt2 > 0xff)
				cnt2 = 0xff;
			OCR1A = cnt2;
			OCR1B = 0;
			m2d = 1;
		}
		else if (cnt2 < SVPN2 - SVPO)
		{
			if (m2d == 1)
			{
				OCR1A = 0xff;
				OCR1B = 0xff;
			}
			cnt2 = SVPN2 - cnt2;
			if (cnt2 > 0xff)
				cnt2 = 0xff;
			OCR1A = 0;
			OCR1B = cnt2;
			m2d = 2;
		}
		else
		{
			cnt2 = 0;
			OCR1A = 0;
			OCR1B = 0;
		}

	}

	return 0;
}
