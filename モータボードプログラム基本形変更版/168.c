/*
�����DC���[�^�[�𐧌䂷�邽�߂̃v���O�����B
TPIP��PWM��DC���[�^�[��PWM�ł͔g�`�̎������Ⴄ�̂ŁA�����ϊ����邽�߂̂��́B
�ŏ��̕��͕K�v�Ȃ̂����܂����悭�킩��Ȃ�
�V���A���ʐM����Ƃ��ɕK�v�Ȃ��̂��낤���H

AVR�ɂ�PWM�g�`�o�͋@�\������
�ڂ����͈ȉ��̃T�C�g�ł�
http://usicolog.nomaki.jp/engineering/avr/avrPWM.html

PWM�g�`�o�͋@�\�𗘗p����ɂ́A
TCCR0A��TCCR0B�ATCCR1A��TCCR1B�̐ݒ肪�K�v

�R���y�A�}�b�`�Ƃ����T�O������A�w��l�ƃ^�C�}�̒l����v�������ɔg�`��HIGH�ALOW�A���]�����邱�Ƃ�PWM���䂪�ł���
�w��l��OCR��A,B�Őݒ肳���
���̒l�𒴂�����TOP�ɂȂ�܂�PWM�M����0���o�͂��A���̌�A�^�C�}��0�ɖ߂�

�����@�c�@�d�g�̎��g����1/n�ɂ��邱�Ɓin�͐����j

1�̃}�C�R����2�̃��[�^�[�𐧌䂷��
PWM�̏o�͒[�q�͌��܂��Ă��āA
1�ڂ�OCR0A(PD6),OCR0B(PD5)
2�ڂ�OCR1A(PB1),OCR1B(PB2)
���ꂼ�ꃂ�[�^�[�h���C�o�̓��͒[�q�ƌq�����Ă���
�ȉ��͓��͂ɂ��DC���[�^�[�̋���
H	H	�u���[�L
H	L	���](�t�])
L	H	�t�](���])
L	L	�X�g�b�v



*/
#include <avr/io.h>
#include <stdio.h>
#define F_CPU 8000000UL
#include <util/delay.h>
//FOSC	DDS�̔��U���g���f�[�^�A�S�o�C�g �����8MHz
#define FOSC 8000000
/*
BAUD	�{�[���[�g�̂���
�{�[���[�g�Ƃ́A�f�W�^���f�[�^��1�b�Ԃɉ��񂾂��ϕ����ł��邩�������l
�{�[���[�g�̒l���傫����Α傫���قǁA�������ԓ��ɂ�葽���̏���]�����邱�Ƃ��\�Ȃ��Ƃ��Ӗ�����
*/
#define BAUD 19200
//�悭�킩��Ȃ�
#define MYUBRR FOSC/16/BAUD-1

//���O�Œ�`��������
#define SVPN1 250
#define SVPN2 250
#define SVPO  30
FILE *fp;

/*
�}�N���g�p��
PORTB = ( 1 << PINB5 ); �@�@�@// PB5�s��������High�ɂ��A�c���Low�ɂ���
��j
PORTB = 0xFF; �@�@�@�@�@�@�@//�� #1111 1111�@(0xFF
PORTB = ( 1 << PINB5 ); �@���@�@#0010 0000�@(0x20
*/

void print_chr(unsigned char c)
{
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = c;
}


//�^�C�}�[0�̏������E�ݒ�֐�
void TMR0_Init(void)
{
	/*
	1������COM0A1�r�b�g��(�܂�7bit�A���̂悤��AVR�Œ�`����Ă���)�V�t�g���Ă���
	COM0B1��5bit�AWGM00��0bit�Ȃ̂�TCCR0A = 0b10100011�Ɠ����Ӗ�
	����̓R���y�A�}�b�`��Low�A����PWM�A����1/8�ATOP�l0xff�̐ݒ�
	�R���y�A�}�b�`�̎w��l�͈ȉ��̒ʂ�
	*/
	TCCR0A = (1 << COM0A1) | (1 << COM0B1) | (3 << WGM00);
	TCCR0B = (1 << CS01);
	OCR0A = 0xff;
	OCR0B = 0xff;
}

//�^�C�}�[1�̏������E�ݒ�֐�
void TMR1_Init(void)
{
	/*
	�R���y�A�}�b�`��Low�A(8bit)����PWM�A����1/8�ATOP�l0xff
	�v�͏�Ɠ����ݒ�
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
	//t1,t2��TPIP�����PWM��H,L�����ԂƂ��đ������ϐ�
	long t1, t2;

	//cnt1,cnt2��t1,t2�̒l�����ɁADC���[�^�[��PWM�ɕϊ������l���i�[����ϐ�
	int cnt1 = 0, cnt2 = 0;
	//m1d,m2d�̓��[�^�[�̐��]�A�t�]��1,2�ŕ\���ϐ�
	int m1d = 0, m2d = 0;
	//PB6,PB7����͂ɂ��� ����2��TPIP��PWM����͂���
	DDRD = 0b11111111;
	DDRB = 0b00111111;
	//PC0�`PC3����͂ɂ��� ���~�b�g�X�C�b�`�̓��͗p
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
		//��3�s�̏�����PWM��r�����瑪�肵�Ă��܂����Ƃ�h�����߂̂���
		if (bit_is_set(PINB, PB6))
			while (bit_is_set(PINB, PB6));
		while (bit_is_clear(PINB, PB6));
		while (bit_is_set(PINB, PB6)){	//HIGH�̎��Ԃ𑪒�
			t1++;
		}
		while (bit_is_clear(PINB, PB6)){		//LOW�̎��Ԃ𑪒�
			t2++;
		}
		//�ϊ�
		cnt1 = ((((500 * t1) / (t1 + t2)) * 40) / 3) - 250;

		t1 = 0;
		t2 = 0;
		if (bit_is_set(PINB, PB7))
			while (bit_is_set(PINB, PB7));
		while (bit_is_clear(PINB, PB7));
		while (bit_is_set(PINB, PB7)){	//HIGH�̎��Ԃ𑪒�
			t1++;
		}
		while (bit_is_clear(PINB, PB7)){		//LOW�̎��Ԃ𑪒�
			t2++;
		}
		cnt2 = ((((500 * t1) / (t1 + t2)) * 40) / 3) - 250;

		//Motor1
		/*
		���~�b�g�X�C�b�`�̎g�p��
		���~�b�g�X�C�b�`�������ꂽ���A����ȏ�i�܂Ȃ��悤�ɂ���(cnt1����𒴂������Acnt1 = SVPN1�A�܂�X�g�b�v��Ԃɂ���)
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
			//���[�^�[�̉�]���������͒l�Ɣ��΂������ꍇ�A��U�u���[�L��������
			if (m1d == 2)			//CCW--Brake
			{
				OCR0A = 0xff;
				OCR0B = 0xff;
			}

			cnt1 = cnt1 - SVPN1;	//30<cnt1<250
			//cnt���ő�l�𒴂��Ȃ��悤�ɂ���(OCR0A,B��8bit�Ȃ̂�)
			if (cnt1 > 0xff)			//cnt1=max
				cnt1 = 0xff;
			//�v�Z���ʂ�������@OCR0A,B�̒l������PWM����������A���[�^�[�h���C�o�ɏo�͂����
			OCR0A = cnt1;
			OCR0B = 0;
			//���݂̉�]�������L�^����
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
