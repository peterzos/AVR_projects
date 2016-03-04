/*
 * fw.h
 *
 *  Created on: Aug 17, 2012
 *      Author: peterz
 */

#ifndef FW_H_
#define FW_H_
#include "onewire.h"

#define MAXSENSORS 6


enum power_type
{
	POWER_PARASITE,
	POWER_EXTERNAL
};

typedef struct
{
	struct
	{
		uint8_t id[OW_ROMCODE_SIZE];
		enum power_type power;		/* 0- parasite; 1-external*/
		int16_t temp;
	}sensors[MAXSENSORS];

	uint8_t sensor_num;

	uint8_t fw_state;

	struct
	{
		volatile uint8_t rxbuff[32];
		volatile uint32_t rxlen;

		volatile uint8_t txbuff[32];
		volatile uint32_t txlen;

		volatile uint8_t valid_cmd;

	}comm;

}_sensor_data;


#endif /* FW_H_ */
