/*
 * RCtoPWM.c
 *
 * Created: 09.01.2015 17:16:14
 *  Author: Администратор
 */ 


//PWM Output to PB7 OC0A
//RC input on PC7 ICP1

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>

//volatile union twobyte { uint16_t word; uint8_t byte[2]; } mesured_width;
//volatile uint16_t mesured_width;
volatile uint16_t pulse_hi;
volatile uint16_t pulse_low;
volatile uint16_t pwm_hi;

//Interrupt on RC signal lost (timer1 overflow)
ISR(TIMER1_OVF_vect) {
	pulse_low = 0;
	pulse_hi = 0;
}

//Interrupt on RC signal edge
ISR(TIMER1_CAPT_vect) {
	//Capture current values
	//mesured_width.byte[0] = ICR1L;	// grab captured timer1 low byte
	//mesured_width.byte[1] = ICR1H;	// grab captured timer1 high byte
	//mesured_width = ICR1;
	
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
	//SYSTEM CLOCK
	//Prescaler div by 1
	//Enable changing prescaler
	//CLKPR = (1 << CLKPCE);
	//Prescaler to 1
	//CLKPR |= (0 << CLKPS0 ) | (0 << CLKPS1 ) | (0 << CLKPS2 );
	//Set for max freq
	//CLKSEL1 = (1 << EXCKSEL0 ) | (1 << EXCKSEL1 ) | (1 << EXCKSEL2 );
	//Turn on external OSC
	//CLKSEL0 = (1 << EXTE) | (1 << CLKS);

	
	//TIMER0
	
	//Start PWM
	//Out to PB7 (OC0A)
	DDRB |= (1<<PINB7 );
		
	//Timer 0 to Fast PWM with output to OC0A
	TCCR0A = (1 << COM0A1) | (1 << COM0A0) | (1 << WGM00) | (1 << WGM01) | (0 << WGM02);
		
	//Set prescaler to div by 1
	TCCR0B |= (0 << CS02) | (1 << CS01) | (1 << CS00);
			
	TCNT0 = 0;
	
	//PWM duty 50%
	OCR0A = 127;
		
		
	//TIMER1
	//In on PC7 (ICP1)
	DDRC = 0;
	
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
	
		
    while(1)
    {
		//Reset watchdog
		wdt_reset(); 
		
		//Calculate new PWN duty
		pwm_hi = ((pulse_hi - 2000) / 8);
		
		if (pwm_hi > 255)
			pwm_hi = 255;
		
		//Check for signal valid (1520 us standard)
		//Not less 18ms and not longer than 20ms
		if ((pulse_low < 36000) || (pulse_low > 40000))
			pwm_hi = 0;
		
		OCR0A = 255 - pwm_hi;
		//OCR0A = pulse_hi - 2000;

    }
}