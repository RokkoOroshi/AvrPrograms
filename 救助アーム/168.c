#include <avr/io.h>
#include <stdio.h>
#define F_CPU 8000000UL
#include <util/delay.h>
#define FOSC 8000000
#define BAUD 19200
#define MYUBRR FOSC/16/BAUD-1
#define SVPN1 250
#define SVPN2 250
#define SVPO  50
FILE *fp;


void print_chr(unsigned char c)
{
	while(!(UCSR0A & (1<<UDRE0)));
	UDR0 = c;
}

void TMR0_Init(void)
{
	TCCR0A = (1<<COM0A1) | (1<<COM0B1) | (3<<WGM00);
	TCCR0B = (1<<CS01);
	OCR0A = 0xff;
	OCR0B = 0xff;
}

void TMR1_Init(void)
{
	TCCR1A = (1<<COM1A1) | (1<<COM1B1) | (1<<WGM10);
	TCCR1B = (1<<WGM12) | (1<<CS11);
	OCR1A = 0xff;
	OCR1B = 0xff;
}

void USART_Init(int baud)
{
	UBRR0H =(char)(baud >> 8);
	UBRR0L = (char) baud;
	UCSR0C = (3<<UCSZ00);
	UCSR0B = (1<<TXEN0);
}

int main(void)
{
	long t1,t2;
	int cnt1 = 0, cnt2 = 0;
	int m1d = 0, m2d = 0;
	DDRD = 0b11111111;
	DDRB = 0b00111111;
	DDRC = 0xf0;
	TMR0_Init();
	TMR1_Init();
//	USART_Init(MYUBRR);
//	fp = fdevopen(print_chr, NULL);
//	printf("Out the cntdata.\r\n");			
	while(1)
	{
		cnt1 = 0;
		cnt2 = 0;
		t1 = 0;
		t2 = 0;

		//SVP1
	if(bit_is_set(PINB,PB6))
		while(bit_is_set(PINB,PB6));
	while(bit_is_clear(PINB,PB6));
	while(bit_is_set(PINB,PB6)){	//HIGH‚ÌŽžŠÔ‚ð‘ª’è
		t1++;
	}
	while(bit_is_clear(PINB,PB6)){		//LOW‚ÌŽžŠÔ‚ð‘ª’è
		t2++;
	}
	cnt1 = 500*t1/(t1+t2)*40/3-250;

	t1 = 0;
	t2 = 0;
	if(bit_is_set(PINB,PB7))
		while(bit_is_set(PINB,PB7));
	while(bit_is_clear(PINB,PB7));
	while(bit_is_set(PINB,PB7)){	//HIGH‚ÌŽžŠÔ‚ð‘ª’è
		t1++;
	}
	while(bit_is_clear(PINB,PB7)){		//LOW‚ÌŽžŠÔ‚ð‘ª’è
		t2++;
	}
	cnt2 = 500*t1/(t1+t2)*40/3-250;
/*
		while(bit_is_set(PINB,PB6));	//pwm wait
		while(bit_is_clear(PINB,PB6));	//pulse
		_delay_ms(1);					//1ms wait
		while(bit_is_set(PINB,PB6))		//0<cnt1<500~
		{
			cnt1++;
			_delay_us(2);
		}

		//SVP2
		while(bit_is_set(PINB,PB7));
		while(bit_is_clear(PINB,PB7));
		_delay_ms(1);
		while(bit_is_set(PINB,PB7))
		{
			cnt2++;
			_delay_us(2);
		}*/

		//printf("%d %d %d %d %d %d\r\n",cnt1,OCR0A,OCR0B,cnt2,OCR1A,OCR1B);	

		//Motor1
		//CW
		if(bit_is_set(PINC,PC3)){
			OCR0A = 150;
			OCR0B = 0;
		}
		else if(cnt1 > SVPN1 + SVPO)		//280<cnt1<500
		{
			if(m1d == 2)			//CCW--Brake
			{
				OCR0A = 0xff;
				OCR0B = 0xff;				
			}

			cnt1 = cnt1 - SVPN1;	//30<cnt1<250
			if(cnt1 > 0xff)			//cnt1=max
				cnt1 = 0xff;
			OCR0A = cnt1;
			OCR0B = 0;
			m1d = 1;
		}

		//CCW
		else if(cnt1 < SVPN1 - SVPO)	//0<cnt1<220
		{
			if(m1d == 1)
			{
				OCR0A = 0xff;
				OCR0B = 0xff;
			}
			cnt1 = SVPN1 - cnt1;
			if(cnt1 > 0xff)
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
		if(bit_is_set(PINC,PC1)){
			OCR1A = 150;
			OCR1B = 0;
		}		
		else if(cnt2 > SVPN2 + SVPO)
		{
			if(m2d == 2)
			{
				OCR1A = 0xff;
				OCR1B = 0xff;				
			}
			cnt2 = cnt2 - SVPN2;
			if(cnt2 > 0xff)
				cnt2 = 0xff;
			OCR1A = cnt2;
			OCR1B = 0;
			m2d = 1;
		}
		else if(cnt2 < SVPN2 - SVPO)
		{
			if(m2d == 1)
			{
				OCR1A = 0xff;
				OCR1B = 0xff;				
			}
			cnt2 = SVPN2 - cnt2;
			if(cnt2 > 0xff)
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
