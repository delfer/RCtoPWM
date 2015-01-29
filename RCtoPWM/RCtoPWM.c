/*
 * RCtoPWM.c
 *
 * Created: 09.01.2015 17:16:14
 *  Author: Администратор
 */ 


//PWM Output to PB7 OC0A
//Rotate direction to PB6
//RC input on PC7 ICP1


//***DEFENITIONS***
//TIMES for 16MHz core in 0.5us
//Example 2000us equals 4000 ticks

#define RC_min 2000
#define RC_mid 3000
#define RC_max 4000
#define enable_reverse 1

//**END***

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>

volatile uint16_t pulse_hi;
volatile uint16_t pulse_low;
volatile uint16_t pwm_x2;
volatile uint8_t  pwm_hi;

//Interrupt on RC signal lost (timer1 overflow)
ISR(TIMER1_OVF_vect) {
	pulse_low = 0;
	pulse_hi = 0;
}

//Interrupt on RC signal edge
ISR(TIMER1_CAPT_vect) {
	//Check state of input
	if (PINC & (1<<PC7))
	{
		//New state is HI
		//Previous is LOW
		
		//Set trigger to falling edge
		TCCR1B &= ~(1 << ICES1 );
		
		//Capture measured value
		pulse_low = ICR1;
	}
	else
	{
		//New state is LOW
		//Previous is HI
		
		//Set trigger to rising edge
		TCCR1B |= (1 << ICES1 );
		
		//Capture measured value
		pulse_hi = ICR1;
	}
	//Reset counter
	TCNT1 = 0;
	ICR1 = 0;
	
	//Reset interrupt flag
	TIFR1 |= (1 << ICF1);
}

int main(void)
{
	//Rotation direction
	//Out to PB6
	DDRB |= (1<<PINB6);
	//Default 0 - forward
	PORTB &= ~(1<<PINB6);
	
	//TIMER0
	
	//Start PWM
	//Out to PB7 (OC0A)
	DDRB |= (1<<PINB7 );
		
	//Timer 0 to Fast PWM with output to OC0A
	TCCR0A = (1 << COM0A1) | (1 << COM0A0) | (1 << WGM00) | (1 << WGM01) | (0 << WGM02);
		
	//Set prescaler to div by 1
	TCCR0B |= (0 << CS02) | (1 << CS01) | (1 << CS00);
			
	TCNT0 = 0;
	
	//PWM duty 0% (out inverted)
	OCR0A = 255;
		
	//TIMER1
	//In on PC7 (ICP1)
	DDRC &= ~(1 << PINC7);
	//Enable internal pull up
	PORTC |= (1 << PINC7);
	
	//Normal mode
	TCCR1A = 0;
	
	//Nose canceler
	//Timer Capture on Falling Edge
	//Timer speed 16Mhz div 8
	TCCR1B = (1 << ICNC1) | (0 << ICES1 ) | (0 << CS12) | (1 << CS11) | (0 << CS10);
	
	//Reset counter
	TCNT1 = 0;
	
	//Reset capture
	ICR1 = 0x0000;
		
	//Turn on interrupts
	sei();
	
	//Turn on interrupt by ICP and overflow
	TIMSK1 |= (1 << ICIE1) | (1 << TOIE1);
	
	//Reset interrupt flag
	TIFR1 |= (1 << ICF1);
	
		
	//Main program loop
    while(1)
    {
		//Reset watchdog
		wdt_reset(); 
		
		//Calculate RC signal to PWMx2 scale
		//MIN 1000us -> 0    - full backward for reverse, neutral for noreverse
		//MID 1500us -> 255  - neutral for reverse, moddle for noreverse
		//MAX 3000us -> 511  - full forward  (for reverse and not)
		//check signal for validity
		if (pulse_hi < RC_min)
			pwm_x2 = 0;
		else if (pulse_hi > RC_max)
			pwm_x2 = 511;
		else if ((pulse_low < 36000) || (pulse_low > 40000))
			pwm_x2 = 0;
		else
			//Calculate new PWN duty
			pwm_x2 = ((pulse_hi - RC_min)  >> 2); // (>> 2) = (div 4) = (2000 div 256)
			
		//Check after div
		if (pwm_x2 > 511)
			pwm_x2 = 511;
			
		#if enable_reverse
			if (pwm_x2 > 255)
			{
				pwm_hi = pwm_x2 - 256;
				PORTB &= ~(1<<PINB6); //FORWARD
			}
			else
			{
				pwm_hi = pwm_x2;
				PORTB |= (1<<PINB6); //BACKWARD
			}
		#else
			pwm_hi = pwm_x2 >> 1; //div 2					
		#endif
		
		//PWM output inverted
		OCR0A = 255 - pwm_hi;
    }
}