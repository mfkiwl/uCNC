/*
	Name: boardmap_mks_dlc32.h
	Description: Contains all MCU and PIN definitions for Arduino WeMos D1 to run µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 11/10/2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BOARDMAP_MKS_DLC32_H
#define BOARDMAP_MKS_DLC32_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "MKS DLC32"
#endif

// Setup limit pins
#define LIMIT_Z_BIT 34	// assigns LIMIT_Z pin
// #define LIMIT_Z_ISR		// assigns LIMIT_Z ISR
#define LIMIT_Y_BIT 35	// assigns LIMIT_Y pin
// #define LIMIT_Y_ISR		// assigns LIMIT_Y ISR
#define LIMIT_X_BIT 36	// assigns LIMIT_X pin
// #define LIMIT_X_ISR  	// assigns LIMIT_X ISR

// Setup com pins
#define RX_BIT 3
#define TX_BIT 1
// only uncomment this if other port other then 0 is used
// #define COM_PORT 0

#define PWM0_BIT 32

// configure the 74HC595 modules
#define DOUT4_BIT 21
#define DOUT5_BIT 16
#define DOUT6_BIT 17
// uses 3 x 74HS595
#define IC74HC595_COUNT 1

#define STEP0_EN_IO_OFFSET 0
#define STEP0_IO_OFFSET 1
#define DIR0_IO_OFFSET 2
#define STEP1_IO_OFFSET 3
#define DIR1_IO_OFFSET 4
#define STEP2_IO_OFFSET 5
#define DIR2_IO_OFFSET 6

	// Setup the Step Timer used has the heartbeat for µCNC
	// Timer 1 is used by default
	//#define ITP_TIMER 1

	// Setup the RTC Timer used by µCNC to provide an (mostly) accurate time base for all time dependent functions
	// Timer 0 is set by default
	//#define RTC_TIMER 0

#define ONESHOT_TIMER 2

#define SPI_CLK_BIT 14
#define SPI_SDO_BIT 12
#define SPI_SDI_BIT 13
#define SPI_CS_BIT 15

//software I2C
#define DIN30_BIT 4
#define DIN31_BIT 0

// include the IO expander
#include "../../../modules/ic74hc595.h"

#ifdef __cplusplus
}
#endif

#endif
