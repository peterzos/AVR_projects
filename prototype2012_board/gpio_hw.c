#include <avr/io.h>

void timeout_init(void)
{   /* call at startup, let it run until needed */
   TCCR1A = 0;      /* "Normal" mode, */
   TCCR1B = (1 << CS12) | (0 << CS11) | (1 << CS10); /* prescale /1024 */


   //TCCR2 = (1 << CS22) | (1 << CS21) | (1 << CS20); /* prescale /1024 */
   return;
}

void hw_init()
{
	timeout_init();

    DDRC  = 0b11111101; //1 = output, 0 = input
  // PORTC =  0b00100001; //Enable pin 5 internal pullup
    //PORTC =  0b00100000; //Enable pin 5 internal pullup

}

void led_g_on()
{
	//PORTC &= ~_BV(PC5);
	PORTC |= _BV(PC5);
}
void led_g_off()
{
	// PORTC |= _BV(PC5);
	PORTC &= ~_BV(PC5);
}

void led_y_on()
{
	//PORTC &= ~_BV(PC4);
	PORTC |= _BV(PC4);
}
void led_y_off()
{
	// PORTC |= _BV(PC4);
	PORTC &= ~_BV(PC4);
}
void led_r_on()
{
	//PORTC &= ~_BV(PC3);
	PORTC |= _BV(PC3);
}
void led_r_off()
{
	// PORTC |= _BV(PC3);
	PORTC &= ~_BV(PC3);
}


