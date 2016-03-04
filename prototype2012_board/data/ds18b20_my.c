



#define F_CPU 7372800UL  /* 7.3728 MHz Internal Oscillator */
#include <avr/io.h>
#include <inttypes.h>
#include <util/delay.h>
#include "uart.h"
#include "debug.h"

#define ONEWIRE_MAX_DEV		5
typedef struct
{
	uint8_t address[8];
	unsigned int temp;
}ds18b20_data;


ds18b20_data ds18b20[ONEWIRE_MAX_DEV];



#define SET_DQ() PORTC |= _BV(PC4);
#define CLR_DQ() PORTC &= ~_BV(PC4);
#define OUT_DQ() DDRC  |= 0b00010000;	//1 = output, 0 = input
#define IN_DQ()  {DDRC  &= 0b11101111; /*SET_DQ();*/}
//#define GET_DQ() (GPIO_ReadValue(1) & (1<<26))
#define GET_DQ()	(PINC & 0b00010000)


/* function for long delay */
void delay_ms(uint16_t ms)
{
	while ( ms )
    {
		_delay_ms(1);
		ms--;
	}
}
void delay_us (unsigned long zms)
{
	while ( zms )
    {
		_delay_us(1);
		zms--;
	}
}
static void resetOnewire()
{

	OUT_DQ();
    CLR_DQ();
    delay_us(750);
    SET_DQ();
	IN_DQ();
	while(GET_DQ());
	while(!(GET_DQ()));
	OUT_DQ();
}

void wOnewire(uint8_t data)
{
    uint8_t i=0;
	OUT_DQ();
    for(i=0;i<8;i++)
    {
        if(data&0x01)
		{
            CLR_DQ();
            delay_us(5);
            SET_DQ();
            delay_us(85); //65
		}
   	 	else
   	 	{
       		CLR_DQ();
        	delay_us(90);  //65
        	SET_DQ();
        	delay_us(5);
    	}
    	data=data>>1;
    }
}

uint8_t rOnewire(void)
{
    uint8_t data=0,i=0;
    for(i=0;i<8;i++)
    {
		data=data>>1;
		OUT_DQ();
        CLR_DQ();
        delay_us(5);
		SET_DQ();
		delay_us(15);
		IN_DQ();
        if(GET_DQ()) data|=0x80;
        else while(!(GET_DQ()));
		delay_us(60);
		SET_DQ();
	}
	return(data);
}

static void readID(uint8_t *ID)
{
	uint8_t i;

	resetOnewire();
	wOnewire(0x33);

	for(i=0;i<8;i++)
	{
		ID[i]=rOnewire();
	}
	//p=ID;
	//return p;
	return;

}

void convertDs18b20(void)
{
    resetOnewire();
    wOnewire(0xcc);
    wOnewire(0x44);
}

unsigned int readTemp(void)
{
	uint8_t temp1,temp2;
	convertDs18b20();
    resetOnewire();
    wOnewire(0xcc);
    wOnewire(0xbe);
    temp1=rOnewire();
    temp2=rOnewire();
	return (((temp2<<8)|temp1)*6.25); //0.0625=xx, 0.625=xx.x, 6.25=xx.xx
}

uint8_t rbit_oneWire()
{
	uint8_t data=0;

	OUT_DQ();
    CLR_DQ();
    delay_us(5);
	SET_DQ();
	delay_us(15);
	IN_DQ();
    if(GET_DQ()) data=0x01;
    else while(!(GET_DQ()));
	delay_us(60);
	SET_DQ();

	return data;
}

void wbit_oneWire(uint8_t data)
{
	OUT_DQ();
	if(data&0x01)
	{
		CLR_DQ();
		delay_us(5);
		SET_DQ();
		delay_us(85); //65
	}
	else
	{
		CLR_DQ();
		delay_us(90);  //65
		SET_DQ();
		delay_us(5);
	}
}


int ds18b20_SearchRom(ds18b20_data *ds18b20)
{
	uint8_t data;
	uint8_t i,j;
	uint8_t ID[ONEWIRE_MAX_DEV][8];
	uint8_t another_iButton;
	uint8_t count;
	uint8_t ic,jc;
	uint8_t last_i,last_j;

	//vTaskDelay(5000 / portTICK_RATE_MS);


	count=0;
	memset(ID, 0, 8*ONEWIRE_MAX_DEV);
	ic=0xFF;
	jc=0xFF;
	last_i = 0xFF;
	last_j = 0xFF;
    do
    {
    	resetOnewire();
    	wOnewire(0xF0);	//SEARCH ROM [F0h]

    	another_iButton = 0;
		for(i=0;i<8;i++)
		{
			for(j=0;j<8;j++)
			{
				data = rbit_oneWire();
				data = data<<1;
				data |= rbit_oneWire();

				_DBH(data);_DBG("-------\n");

				switch (data)
				{
				case 0x00:	/*00 - conflict send bit*/
					if((last_i==i) && (last_j==j))
					{
						uart_puts("conflict select 1\n");

						wbit_oneWire(1);
						ID[count][i] |=1<<j;
						ds18b20[count].address[i] |=1<<j;
						if((ic==0xFF) && (jc==0xFF))
							another_iButton = 0;
					}
					else
					{
						uart_puts("conflict select 0\n");
					//	_DBH(ic);_DBG("---");_DBH(jc);_DBG("---\n");
						wbit_oneWire(0);	// if there is confilice always select 0
						//if((ic==0xFF) && (jc==0xFF))
						{
						ic=i;
						jc=j;
						}
						another_iButton = 1;
					}
					break;
				case 0x01:	/* all device have 0 in this bit position */
					wbit_oneWire(0);
					break;
				case 0x02:	/* bin 10 all device have 1 in this bit position */
					wbit_oneWire(1);
					ID[count][i] |=1<<j;
					ds18b20[count].address[i] |=1<<j;
					break;
				default:
				//	_DBG("Error\n");
					break;
				}
			}
		}
		last_i = ic;
		last_j = jc;
		ic=0xFF;
		jc=0xFF;
	//	_DBG("Detected_ID:");
	//	_DBGH_(ds18b20[count].address,8);
	//	_DBG("\n");
		count++;
    }while ((another_iButton) && (count<ONEWIRE_MAX_DEV));

//	_DBG("Detected_ID:");
//	_DBGH_(ID[0],8);
//	_DBG("---------------------\n");

    memset(&ID[count][0], 0, 8);


#if (LPC_DEBUG==1)
//#error dfdsfdsfsd
#endif

//	_DBG("Detected_ID:");
//	_DBGH_(ID[0],8);
//	_DBG("\n");
//
//	_DBG("Detected_ID:");
//		_DBGH_(ID[1],8);
//		_DBG("\n");

		return count;
}

void wOneWire_data(uint8_t *data, uint8_t count)
{
	uint8_t i;

	for(i=0;i<count;i++)
		wOnewire(data[i]);


}


void zos_ds18b20_init()
{
	int	sens_num;
	char buffer[7];

	DDRC  = 0b11001111; //1 = output, 0 = input
	PORTC = 0b00110000; //Enable pin 5 and 4 internal pullup

	sens_num = ds18b20_SearchRom(ds18b20);

    itoa( sens_num, buffer, 10);   // convert interger into string (decimal format)
    uart_puts(buffer);        // and transmit string to UART
    uart_puts("\nds18b20 Done\n");

}
