/*
	Name: mcumap_grbl.h
	Description: Contains all MCU and PIN definitions for Arduino UNO to run µCNC.
	
	Copyright: Copyright (c) João Martins 
	Author: João Martins
	Date: 04/02/2020

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BOARDMAP_BLUEPILL_H
#define BOARDMAP_BLUEPILL_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef STM32F103x6
#define STM32F103x6
#endif

#ifndef STM32F10X_MD
#define STM32F10X_MD
#endif

#ifndef F_CPU
#define F_CPU 72000000UL
#endif

//Setup COM pins (must match selected COM port)
//Comment these to use USB virtual port
#ifndef USB_VCP
#define COM_PORT 1
#endif

//Setup step pins
#define STEP0_BIT 0	 //assigns STEP0 pin
#define STEP0_PORT A //assigns STEP0 port
#define STEP1_BIT 1	 //assigns STEP1 pin
#define STEP1_PORT A //assigns STEP1 port
#define STEP2_BIT 2	 //assigns STEP2 pin
#define STEP2_PORT A //assigns STEP2 port
#define STEP3_BIT 3	 //assigns STEP3 pin
#define STEP3_PORT A //assigns STEP3 port

//Setup dir pins
#define DIR0_BIT 4	//assigns DIR0 pin
#define DIR0_PORT A //assigns DIR0 port
#define DIR1_BIT 5	//assigns DIR1 pin
#define DIR1_PORT A //assigns DIR1 port
#define DIR2_BIT 6	//assigns DIR2 pin
#define DIR2_PORT A //assigns DIR2 port
#define DIR3_BIT 7	//assigns DIR3 pin
#define DIR3_PORT A //assigns DIR3 port

//Setup limit pins
#define LIMIT_X_BIT 12 //assigns LIMIT_X pin
#define LIMIT_X_PORT B //assigns LIMIT_X port
#define LIMIT_Y_BIT 13 //assigns LIMIT_Y pin
#define LIMIT_Y_PORT B //assigns LIMIT_Y port
#define LIMIT_Z_BIT 14 //assigns LIMIT_Z pin
#define LIMIT_Z_PORT B //assigns LIMIT_Z port
#define LIMIT_A_BIT 15 //assigns LIMIT_A pin
#define LIMIT_A_PORT B //assigns LIMIT_A port

//Enable limits switch weak pull-ups
#define LIMIT_X_PULLUP
#define LIMIT_Y_PULLUP
#define LIMIT_Z_PULLUP
#define LIMIT_A_PULLUP
//Enable limits switch interrupt
#define LIMIT_X_ISR
#define LIMIT_Y_ISR
#define LIMIT_Z_ISR
#define LIMIT_A_ISR

//Setup control input pins
#define ESTOP_BIT 5
#define ESTOP_PORT B
#define FHOLD_BIT 6
#define FHOLD_PORT B
#define CS_RES_BIT 7
#define CS_RES_PORT B
#define SAFETY_DOOR_BIT 8
#define SAFETY_DOOR_PORT B
//Setup probe pin
#define PROBE_BIT 9
#define PROBE_PORT B

//Enable controls switch weak pull-ups
#define ESTOP_PULLUP
#define FHOLD_PULLUP
#define CS_RES_PULLUP
#define SAFETY_DOOR_PULLUP

//Enable controls switch interrupt
#define ESTOP_ISR
#define FHOLD_ISR
#define CS_RES_ISR
#define SAFETY_DOOR_ISR

//On the STM32 always use sync TX UART (async doesn't work well)
#ifdef COM_PORT
#define TX_BIT 9
#define TX_PORT A
#define RX_BIT 10
#define RX_PORT A
#else
#define USB_DM_BIT 11
#define USB_DM_PORT A
#define USB_DP_BIT 12
#define USB_DP_PORT A
#define STEP4_BIT 3	   //assigns STEP4 pin
#define STEP4_PORT A   //assigns STEP4 port
#define DIR4_BIT 7	   //assigns DIR4 pin
#define DIR4_PORT A	   //assigns DIR4 port
#define LIMIT_B_BIT 11 //assigns LIMIT_A pin
#define LIMIT_B_PORT B //assigns LIMIT_A port
#define LIMIT_B_PULLUP
#define LIMIT_B_ISR
#endif

//Setup PWM
#define PWM0_BIT 8	//assigns PWM0 pin
#define PWM0_PORT A //assigns PWM0 pin
#define PWM0_CHANNEL 1
#define PWM0_TIMER 1

//Setup generic IO Pins
//Functionalities are set in cnc_hal_config.h file

//spindle dir
#define DOUT0_BIT 0
#define DOUT0_PORT B

//coolant and mist
#define DOUT1_BIT 4
#define DOUT1_PORT B
#define DOUT2_BIT 3
#define DOUT2_PORT B

//stepper enable
#define DOUT3_BIT 15
#define DOUT3_PORT A

//led pin
#define DOUT15_BIT 13
#define DOUT15_PORT C

//Stepper enable pin. For Grbl on Uno board a single pin is used
#define STEP0_EN_BIT 15
#define STEP0_EN_PORT A

//analog input
#define ANALOG0_BIT 1
#define ANALOG0_PORT B
#define ANALOG0_CHANNEL 9

	//Setup the Step Timer used has the heartbeat for µCNC
	//#define ITP_TIMER 2

#ifdef __cplusplus
}
#endif

#endif