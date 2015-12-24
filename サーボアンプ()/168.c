#include <avr/io.h>
#include <stdio.h>
#define F_CPU 8000000UL
#include <util/delay.h>

void pwm(int c){
	int s;
	for(s = 0;s < 1870;s++){
		if(s < c)
			PORTD |= 0b00000010;
		else
			PORTD &= 0b11111101;
		_delay_us(10);
	}
}		

int main(void)
{
	int cnt = 215;
	DDRD = 0b11111111;
	DDRB = 0b00111111;
	DDRC = 0xf0;	
	while(1)
	{
		if(bit_is_set(PINB,PB7)){
			cnt++;
			if(cnt > 215)
				cnt = 215;
		}
		else if(bit_is_set(PINB,PB6)){
			cnt--;
			if(cnt < 50)
				cnt = 50;
		}
		pwm(cnt);
	}
	return 0;
}
