/*
   DS18x20 Demo-Program

   V 0.9.2, 2/2011

   by Martin Thomas <eversmith@heizung-thomas.de>
   http://www.siwawi.arubi.uni-kl.de/avr-projects

   features:
   - DS18X20 and 1-Wire code is based on an example from Peter
     Dannegger
   - uses Peter Fleury's uart-library which is very portable
   - additional functions not found in the  uart-lib available
     in uart.h/.c
   - CRC-check based on code from Colin O'Flynn
   - accesses multiple sensors on multiple 1-Wire busses
   - example how to address every sensor in the bus by ROM-code
   - independant of system-timers (more portable) but some
     (very short) delays used
   - avr-libc's stdint.h in use
   - no central include-file, parts of the code can be used as
     "library" easily
   - verbose output example
   - one-wire-bus can be changed at runtime if OW_ONE_BUS
     is not defined in onewire.h. There are still minor timing
     issues when using the dynamic bus-mode
   - example on read/write of DS18x20 internal EEPROM
*/


/* This example has been tested with ATmega324P at 3.6864MHz and 16Mhz */


#include <avr/version.h>
#if __AVR_LIBC_VERSION__ < 10606UL
#error "please update to avrlibc 1.6.6 or newer, not tested with older versions"
#endif


#define F_CPU 7372800UL  /* 7.3728 MHz Internal Oscillator */
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <string.h>
#include <stdint.h>

#include "uart.h"
#include "uart_addon.h"
#include "onewire.h"
#include "ds18x20.h"
#include "gpio_hw.h"
#include "fw.h"

#define BAUD 9600
// 2400 for 1MHz and 2MHz internal RC
// #define BAUD 2400



#define NEWLINESTR "\r\n"


uint8_t gSensorIDs[MAXSENSORS][OW_ROMCODE_SIZE];




#define FW_STATE_SENSOR_SEARCH			0
#define FW_STATE_SENSOR_START_MEAS		1
#define FW_STATE_SENSOR_DELAY_750ms		2
#define FW_STATE_SENSOR_READ_I			3
#define FW_STATE_SENSOR_DELAY_5s		4
#define FW_STATE_READ_COMM				5




_sensor_data sensor_fw;




static uint8_t search_sensors(void)
{
	uint8_t i;
	uint8_t id[OW_ROMCODE_SIZE];
	uint8_t diff, nSensors;

	//uart_puts_P( NEWLINESTR "Scanning Bus for DS18X20" NEWLINESTR );

	ow_reset();

	nSensors = 0;

	diff = OW_SEARCH_FIRST;
	while ( diff != OW_LAST_DEVICE && nSensors < MAXSENSORS ) {
		DS18X20_find_sensor( &diff, &id[0] );

		if( diff == OW_PRESENCE_ERR ) {
			uart_puts_P( "No Sensor found" NEWLINESTR );
			break;
		}

		if( diff == OW_DATA_ERR ) {
			uart_puts_P( "Bus Error" NEWLINESTR );
			break;
		}

		for ( i=0; i < OW_ROMCODE_SIZE; i++ )
			sensor_fw.sensors[nSensors].id[i] = id[i];
			//gSensorIDs[nSensors][i] = id[i];

		nSensors++;
	}

	return nSensors;
}

static void uart_put_temp(int16_t decicelsius)
{
	char s[10];

	uart_put_int( decicelsius );
	uart_puts_P(" deci°C, ");
	DS18X20_format_from_decicelsius( decicelsius, s, 10 );
	uart_puts( s );
	uart_puts_P(" °C");
}


#if DS18X20_MAX_RESOLUTION

static void uart_put_temp_maxres(int32_t tval)
{
	char s[10];

	uart_put_longint( tval );
	uart_puts_P(" °Ce-4, ");
	DS18X20_format_from_maxres( tval, s, 10 );
	uart_puts( s );
	uart_puts_P(" °C");
}

#endif /* DS18X20_MAX_RESOLUTION */


#if DS18X20_EEPROMSUPPORT

static void th_tl_dump(uint8_t *sp)
{
	DS18X20_read_scratchpad( &gSensorIDs[0][0], sp, DS18X20_SP_SIZE );
	uart_puts_P( "TH/TL in scratchpad of sensor 1 now : " );
	uart_put_int( sp[DS18X20_TH_REG] );
	uart_puts_P( " / " );
	uart_put_int( sp[DS18X20_TL_REG] );
	uart_puts_P( NEWLINESTR );
}

static void eeprom_test(void)
{
	uint8_t sp[DS18X20_SP_SIZE], th, tl;

	uart_puts_P( NEWLINESTR "DS18x20 EEPROM support test for first sensor" NEWLINESTR );
	// DS18X20_eeprom_to_scratchpad(&gSensorIDs[0][0]); // already done at power-on
	th_tl_dump( sp );
	th = sp[DS18X20_TH_REG];
	tl = sp[DS18X20_TL_REG];
	tl++;
	th++;
	DS18X20_write_scratchpad( &gSensorIDs[0][0], th, tl, DS18B20_12_BIT );
	uart_puts_P( "TH+1 and TL+1 written to scratchpad" NEWLINESTR );
	th_tl_dump( sp );
	DS18X20_scratchpad_to_eeprom( DS18X20_POWER_PARASITE, &gSensorIDs[0][0] );
	uart_puts_P( "scratchpad copied to DS18x20 EEPROM" NEWLINESTR );
	DS18X20_write_scratchpad( &gSensorIDs[0][0], 0, 0, DS18B20_12_BIT );
	uart_puts_P( "TH and TL in scratchpad set to 0" NEWLINESTR );
	th_tl_dump( sp );
	DS18X20_eeprom_to_scratchpad(&gSensorIDs[0][0]);
	uart_puts_P( "DS18x20 EEPROM copied back to scratchpad" NEWLINESTR );
	DS18X20_read_scratchpad( &gSensorIDs[0][0], sp, DS18X20_SP_SIZE );
	if ( ( th == sp[DS18X20_TH_REG] ) && ( tl == sp[DS18X20_TL_REG] ) ) {
		uart_puts_P( "TH and TL verified" NEWLINESTR );
	} else {
		uart_puts_P( "verify failed" NEWLINESTR );
	}
	th_tl_dump( sp );
}
#endif /* DS18X20_EEPROMSUPPORT */


void search_bus()
{
	uint8_t i;

	while(1)
	{
		sensor_fw.sensor_num = search_sensors();
		if (sensor_fw.sensor_num > 0)
			break;
		_delay_ms(1000);
	}
	/* get power status */
	for ( i = 0; i < sensor_fw.sensor_num; i++ )
	{
		if ( DS18X20_get_power_status(&sensor_fw.sensors[i].id[0] ) == DS18X20_POWER_PARASITE )
		{
			sensor_fw.sensors[i].power = POWER_PARASITE;
		}
		else
		{
			sensor_fw.sensors[i].power = POWER_EXTERNAL;
		}
	}
}


int main( void )
{
	uint8_t i=0;
	int16_t decicelsius;
	uint8_t error;
	uint8_t delayCounter=0;

	hw_init();
	uart_init((UART_BAUD_SELECT((BAUD),F_CPU)));
	memset(&sensor_fw, 0, sizeof(_sensor_data));


	/* init 485 write */
	   DDRB  |= 0b0000001; //1 = output, 0 = input
	  //PORTB |=  0b00000001; //Enable pin 5 internal pullup
	   PORTB &=   0b11111110; //Enable pin 5 internal pullup read
	   //PORTB |=  0b00000001; //Enable pin 5 internal pullup 485 write


	   led_g_on();
	   _delay_ms(1000);
	   led_y_on();
	   _delay_ms(1000);
	   led_r_on();
	   _delay_ms(1000);
	   led_r_off();
	   _delay_ms(1000);
	   led_y_off();
	   _delay_ms(1000);
	   led_g_off();

#ifndef OW_ONE_BUS
	ow_set_bus(&PIND,&PORTD,&DDRD,PD6);
#endif

	led_g_on();
	search_bus();
	led_g_off();

	sei();

	sensor_fw.fw_state = FW_STATE_SENSOR_START_MEAS;

	for(;;)
	{   // main loop

		switch (sensor_fw.fw_state)
		{
		case FW_STATE_SENSOR_SEARCH:
			led_g_on();
			search_bus();
			led_g_off();
			uart_puts_P("FW_STATE_SENSOR_SEARCH? =0\n");
			sensor_fw.fw_state = FW_STATE_SENSOR_START_MEAS;
			break;
		case FW_STATE_SENSOR_START_MEAS:
			if ( sensor_fw.sensor_num == 0 )
			{
				sensor_fw.fw_state = FW_STATE_SENSOR_SEARCH;
				uart_puts_P("error sensor num =0\n");
				break;
			}
			if ( DS18X20_start_meas( DS18X20_POWER_PARASITE, NULL )	== DS18X20_OK)
			{
				sensor_fw.fw_state = FW_STATE_SENSOR_DELAY_750ms;
				//reset_timeout();
			}
			else
			{
				sensor_fw.fw_state = FW_STATE_SENSOR_SEARCH;
				uart_puts_P("error start mes faul =0\n");
			}
			break;

		case FW_STATE_SENSOR_DELAY_750ms:
			//if (TCNT1 > 5400)	//750 ms
			_delay_ms( DS18B20_TCONV_12BIT );
			{
				sensor_fw.fw_state = FW_STATE_SENSOR_READ_I;
				i = 0;
			}
			break;

		case FW_STATE_SENSOR_READ_I:
			if ( DS18X20_read_decicelsius(&sensor_fw.sensors[i].id[0], &decicelsius ) == DS18X20_OK )
			{
				sensor_fw.sensors[i].temp = decicelsius;

				uart_puts_P( "Sensor# " );
				uart_put_int( (int)i + 1 );
				uart_puts_P(" = ");
				uart_put_temp( decicelsius );
				uart_puts_P( NEWLINESTR );
			}
			else
			{
				//uart_puts_P( "CRC Error (lost connection?)" );
				sensor_fw.fw_state = FW_STATE_SENSOR_SEARCH;
				uart_puts_P("error lost connection? =0\n");
				break;
			}
			i++;
			if (i >= sensor_fw.sensor_num)
			{
				sensor_fw.fw_state = FW_STATE_SENSOR_DELAY_5s;
				delayCounter = 0;
				//reset_timeout();
			}
			//uart_puts_P( NEWLINESTR );
			break;

		case FW_STATE_SENSOR_DELAY_5s:
			//if (TCNT1 > (5 * TICKS_PER_SEC))	/*5s*/
			_delay_ms( 1000 );
			{
				if(delayCounter > 5)
					sensor_fw.fw_state = FW_STATE_SENSOR_START_MEAS;
				delayCounter++;
			}
			break;

		case FW_STATE_READ_COMM:
			{
				//uart_puts_P( "Communication cmd rx\n" );
				switch(sensor_fw.comm.rxbuff[2])
				{
				case 0x01:
					sensor_fw.comm.txbuff[0] = 0x7E;
					sensor_fw.comm.txbuff[1] = 0x01;	// dev_id
					sensor_fw.comm.txbuff[2] = 0x02;	// len
					sensor_fw.comm.txbuff[3] = 0x00;	// len
					sensor_fw.comm.txbuff[4] = 0xBB;	// data
					sensor_fw.comm.txbuff[5] = 0xBB;	// data

					sensor_fw.comm.txbuff[6] = 0x00;	// crc
					sensor_fw.comm.txbuff[7] = 0x00;	// crc


					uart_putData(sensor_fw.comm.txbuff,8);
					break;
				}

				sensor_fw.comm.rxlen = 0;
				sensor_fw.comm.valid_cmd = 0;
				sensor_fw.fw_state = FW_STATE_SENSOR_START_MEAS;
			}
			break;
		}

		if(sensor_fw.comm.valid_cmd)
		{
			sensor_fw.fw_state = FW_STATE_READ_COMM;
		}

		//_delay_ms(3000);
	}
}
