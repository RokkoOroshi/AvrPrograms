/*
DOサーボ用のプログラム
回路上には入力端子が10, 出力端子が5ある
このプログラムでは、入力された2つのデジタル信号を変換してサーボのPWM出力を行っている
*/


//AVRの基本的なヘッダファイル
#include <avr/io.h>
//delay関係の関数(_delay_us, _delay_ms)を使用するのに必要
#include <util/delay.h>
//CPUのクロック数を指定する この値を元にdelay関数の待ち時間が決定する ULはunsigned long型のこと　つまり1回の処理に50ナノ秒(5*10^-8)
#define F_CPU 20000000UL
/*
/DDR○ = 0x△△; 
指定したポートのビットを入力か出力かを決定する
0を入れれば入力に、1を入れれば出力になる
*/

/*
PORT○ = 0x△△;
指定したポートの出力でHを出すかLを出すか決定する
DDRで出力に設定されているビットに1を入れるとHを出力、0を入れるとLを出力する
また、DDRで入力に設定されているビットに1を入れると、そのピンはプルアップされる
プルアップ…そのラインに何も接続されていない状態になった時に、そのラインの電位を「Hレベル」に固定すること
*/

/*
bit_is_set(register, bit)	指定レジスタの指定のビットがセット(1)されているかどうかチェック
bit_is_clear(register, bit)	セット(1)されてされていないかチェック
*/

/*
a &= b … a = a & b
a |= b … a = a | b
*/

/*
変数の解説
define
INI,MAX,MIN　…　それぞれ初期値、最大値、最小値　s[]はこの範囲の値をとる
SPD　…　s[]が変化していくスピード
*/

#define INI {150,150,150,75,200}
#define SPD {1,1,1,1,1}
#define MAX {225,225,225,180,200}
#define MIN {75,75,100,75,75}

int s[5] = INI;
int spd[5] = SPD;
int max[5] = MAX;
int min[5] = MIN;
int i,j;

/*
s[]の値を元にPWM信号を出力する関数
初めにPD0～4をすべてHにした後、for文でs[]の値に応じてそれぞれLにしていくことでPWMの波形を表現している
100usごとにこれを実行している?

*/
void pwm(void){
	/*
	for(i = 0;i < 920;i++){
		if(i < s[0])
			PORTD |= 0b00000001;
		else
			PORTD &= 0b11111110;
		if(i < s[1])
			PORTD |= 0b00000010;
		else
			PORTD &= 0b11111101;
		if(i < s[2])
			PORTD |= 0b00000100;
		else
			PORTD &= 0b11111011;
		if(i < s[3])
			PORTD |= 0b00001000;
		else
			PORTD &= 0b11110111;
		if(i < s[4])
			PORTD |= 0b00010000;
		else
			PORTD &= 0b11101111;
		_delay_us(5);
	}*/
	PORTD = 0b00011111;
	for(i = 0;i < 1840;i++){
		if(i == s[0])
			PORTD &= 0b11111110;
		if(i == s[1])
			PORTD &= 0b11111101;
		if(i == s[2])
			PORTD &= 0b11111011;
		if(i == s[3])
			PORTD &= 0b11110111;
		if(i == s[4])
			PORTD &= 0b11101111;
	}
}	

/*
s[]の値を増減させる関数
MAX,MINの範囲で、aに入力があれば増加し、bに入力があれば減少する
*/
void move(int n,int a,int b){
	if(a){
		s[n] += spd[n];
		if(s[n] > max[n])
			s[n] = max[n];
	}
	if(b){
		s[n] -= spd[n];
		if(s[n] < min[n])
			s[n] = min[n];
	}
}

int main(void){
	//PD0～4を出力に、他は入力に
	DDRB = 0b00000000;
	DDRD = 0b00011111;
	DDRA = 0;
	//無限ループ 以下の処理を実行し続ける
	while(1){
		move(0,bit_is_set(PINB,PB7),bit_is_set(PINB,PB6));
		move(1,bit_is_set(PINB,PB5),bit_is_set(PINB,PB4));
		move(2,bit_is_set(PINB,PB3),bit_is_set(PINB,PB2));
		move(3,bit_is_set(PINB,PB1),bit_is_set(PINB,PB0));
		move(4,bit_is_set(PIND,PD6),bit_is_set(PIND,PD5));
		pwm();
	}
	return 0;
}
