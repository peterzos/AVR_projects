#ifndef GPIO_HW_H_
#define GPIO_HW_H_


#define F_CPU 7372800UL  /* 7.3728 MHz Internal Oscillator */
#define   TICKS_PER_SEC (F_CPU/1024)   /* ticks/sec with prescale /1024 */
#define   TIMEOUT_TIME (1 * TICKS_PER_SEC) /* timeout: 1 second */
#define   reset_timeout() do { TCNT1 = 0; } while (0)
#define   timeout_event() (TCNT1 >= TIMEOUT_TIME)

void hw_init();
void led_on();
void led_off();

#endif /* GPIO_HW_H_ */
