/*
	Name: graphic_lcd.c
	Description: Graphic LCD module for µCNC using u8g2 lib.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 08-09-2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../cnc.h"
#include "u8g2.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifndef UCNC_MODULE_VERSION_1_5_0_PLUS
#error "This module is not compatible with the current version of µCNC"
#endif

// used with graphic_lcd module

#ifndef U8X8_MSG_GPIO_SPI_CLOCK_PIN
#define U8X8_MSG_GPIO_SPI_CLOCK_PIN DOUT8
#endif
#ifndef U8X8_MSG_GPIO_SPI_DATA_PIN
#define U8X8_MSG_GPIO_SPI_DATA_PIN DOUT9
#endif
#ifndef U8X8_MSG_GPIO_CS_PIN
#define U8X8_MSG_GPIO_CS_PIN DOUT10
#endif

#ifndef U8X8_MSG_GPIO_I2C_CLOCK_PIN
#define U8X8_MSG_GPIO_I2C_CLOCK_PIN DOUT9
#endif
#ifndef U8X8_MSG_GPIO_I2C_DATA_PIN
#define U8X8_MSG_GPIO_I2C_DATA_PIN DOUT10
#endif

static u8g2_t u8g2;

/**
 *
 * can also be done via hardware SPI and I2C libraries of µCNC
 * but is not needed
 *
 * */
// #include "../softspi.h"
// uint8_t u8x8_byte_ucnc_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
// {
// 	uint8_t *data;
// 	switch (msg)
// 	{
// 	case U8X8_MSG_BYTE_SEND:
// 		data = (uint8_t *)arg_ptr;
// 		while (arg_int > 0)
// 		{
// 			softspi_xmit(NULL, (uint8_t)*data);
// 			data++;
// 			arg_int--;
// 		}
// 		break;
// 	case U8X8_MSG_BYTE_INIT:
// 		mcu_set_output(U8X8_MSG_GPIO_CS_PIN);
// 		break;
// 	case U8X8_MSG_BYTE_SET_DC:
// 		u8x8_gpio_SetDC(u8x8, arg_int);
// 		break;
// 	case U8X8_MSG_BYTE_START_TRANSFER:
//      softspi_config(u8x8->display_info->spi_mode, u8x8->display_info->sck_clock_hz);
// 		/* SPI mode has to be mapped to the mode of the current controller, at least Uno, Due, 101 have different SPI_MODEx values */
// 		u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_enable_level);
// 		u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, u8x8->display_info->post_chip_enable_wait_ns, NULL);
// 		break;
// 	case U8X8_MSG_BYTE_END_TRANSFER:
// 		u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, u8x8->display_info->pre_chip_disable_wait_ns, NULL);
// 		u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);
// 		break;
// 	default:
// 		return 0;
// 	}
// 	return 1;
// }

// #include "../softi2c.h"
// uint8_t u8x8_byte_ucnc_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
// {
// 	static uint8_t buffer[32]; /* u8g2/u8x8 will never send more than 32 bytes between START_TRANSFER and END_TRANSFER */
// 	static uint8_t buf_idx;
// 	uint8_t *data;

// 	switch (msg)
// 	{
// 	case U8X8_MSG_BYTE_SEND:
// 		data = (uint8_t *)arg_ptr;
// 		while (arg_int > 0)
// 		{
// 			buffer[buf_idx++] = *data;
// 			data++;
// 			arg_int--;
// 		}
// 		break;
// 	case U8X8_MSG_BYTE_INIT:
// 		/* add your custom code to init i2c subsystem */
// 		break;
// 	case U8X8_MSG_BYTE_SET_DC:
// 		/* ignored for i2c */
// 		break;
// 	case U8X8_MSG_BYTE_START_TRANSFER:
// 		buf_idx = 0;
// 		break;
// 	case U8X8_MSG_BYTE_END_TRANSFER:
// 		softi2c_send(NULL, u8x8_GetI2CAddress(u8x8) >> 1, buffer, (int)buf_idx);
// 		break;
// 	default:
// 		return 0;
// 	}

// 	return 1;
// }

uint8_t u8x8_gpio_and_delay_ucnc(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
	switch (msg)
	{
	case U8X8_MSG_GPIO_AND_DELAY_INIT: // called once during init phase of u8g2/u8x8
		break;						   // can be used to setup pins
	case U8X8_MSG_DELAY_NANO:		   // delay arg_int * 1 nano second
		while (arg_int--)
			;
		break;
	case U8X8_MSG_DELAY_100NANO: // delay arg_int * 100 nano seconds
		arg_int = (uint8_t)(arg_int / 10) + 1;
		while (arg_int--)
			mcu_delay_us(1);
		break;
	case U8X8_MSG_DELAY_10MICRO: // delay arg_int * 10 micro seconds
		while (arg_int--)
			mcu_delay_us(10);
		break;
	case U8X8_MSG_DELAY_MILLI: // delay arg_int * 1 milli second
		cnc_delay_ms(arg_int);
		break;
	case U8X8_MSG_DELAY_I2C: // arg_int is the I2C speed in 100KHz, e.g. 4 = 400 KHz
		if (arg_int == 1)
		{
			mcu_delay_us(5);
		}
		else
		{
			mcu_delay_us(1);
		}
		break; // arg_int=1: delay by 5us, arg_int = 4: delay by 1.25us
#ifndef U8X8_MSG_GPIO_SPI_CLOCK_PIN
	case U8X8_MSG_GPIO_D0: // D0 or SPI clock pin: Output level in arg_int
#else
	case U8X8_MSG_GPIO_SPI_CLOCK:
#endif
#ifdef U8X8_MSG_GPIO_SPI_CLOCK_PIN
		if (arg_int)
		{
			mcu_set_output(U8X8_MSG_GPIO_SPI_CLOCK_PIN);
		}
		else
		{
			mcu_clear_output(U8X8_MSG_GPIO_SPI_CLOCK_PIN);
		}
#endif
		break;
#ifndef U8X8_MSG_GPIO_SPI_DATA_PIN
	case U8X8_MSG_GPIO_D1: // D1 or SPI data pin: Output level in arg_int
#else
	case U8X8_MSG_GPIO_SPI_DATA:
#endif
#ifdef U8X8_MSG_GPIO_SPI_DATA_PIN
		if (arg_int)
		{
			mcu_set_output(U8X8_MSG_GPIO_SPI_DATA_PIN);
		}
		else
		{
			mcu_clear_output(U8X8_MSG_GPIO_SPI_DATA_PIN);
		}
#endif
		break;
	case U8X8_MSG_GPIO_D2: // D2 pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_D2_PIN
		io_set_output(U8X8_MSG_GPIO_D2_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_D3: // D3 pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_D3_PIN
		io_set_output(U8X8_MSG_GPIO_D3_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_D4: // D4 pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_D4_PIN
		io_set_output(U8X8_MSG_GPIO_D4_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_D5: // D5 pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_D5_PIN
		io_set_output(U8X8_MSG_GPIO_D5_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_D6: // D6 pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_D6_PIN
		io_set_output(U8X8_MSG_GPIO_D6_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_D7: // D7 pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_D7_PIN
		io_set_output(U8X8_MSG_GPIO_D7_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_E: // E/WR pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_E_PIN
		io_set_output(U8X8_MSG_GPIO_E_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_CS: // CS (chip select) pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_CS_PIN
		if (arg_int)
		{
			mcu_set_output(U8X8_MSG_GPIO_CS_PIN);
		}
		else
		{
			mcu_clear_output(U8X8_MSG_GPIO_CS_PIN);
		}
#endif
		break;
	case U8X8_MSG_GPIO_DC: // DC (data/cmd, A0, register select) pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_DC_PIN
		io_set_output(U8X8_MSG_GPIO_DC_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_RESET: // Reset pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_RESET_PIN
		io_set_output(U8X8_MSG_GPIO_RESET_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_CS1: // CS1 (chip select) pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_CS1_PIN
		io_set_output(U8X8_MSG_GPIO_CS1_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_CS2: // CS2 (chip select) pin: Output level in arg_int
#ifdef U8X8_MSG_GPIO_CS2_PIN
		io_set_output(U8X8_MSG_GPIO_CS2_PIN, (bool)arg_int);
#endif
		break;
	case U8X8_MSG_GPIO_I2C_CLOCK: // arg_int=0: Output low at I2C clock pin
#ifdef U8X8_MSG_GPIO_I2C_CLOCK_PIN
		io_set_output(U8X8_MSG_GPIO_I2C_CLOCK_PIN, (bool)arg_int);
#endif
		break;					 // arg_int=1: Input dir with pullup high for I2C clock pin
	case U8X8_MSG_GPIO_I2C_DATA: // arg_int=0: Output low at I2C data pin
#ifdef U8X8_MSG_GPIO_I2C_DATA_PIN
		io_set_output(U8X8_MSG_GPIO_I2C_DATA_PIN, (bool)arg_int);
#endif
		break; // arg_int=1: Input dir with pullup high for I2C data pin
	case U8X8_MSG_GPIO_MENU_SELECT:
#ifdef U8X8_MSG_GPIO_MENU_SELECT_PIN
		u8x8_SetGPIOResult(u8x8, io_get_pinvalue(U8X8_MSG_GPIO_MENU_SELECT_PIN));
#else
		u8x8_SetGPIOResult(u8x8, /* get menu next pin state */ 0);
#endif
		break;
	case U8X8_MSG_GPIO_MENU_NEXT:
#ifdef U8X8_MSG_GPIO_MENU_NEXT_PIN
		u8x8_SetGPIOResult(u8x8, io_get_pinvalue(U8X8_MSG_GPIO_MENU_NEXT_PIN));
#else
		u8x8_SetGPIOResult(u8x8, /* get menu next pin state */ 0);
#endif
		break;
	case U8X8_MSG_GPIO_MENU_PREV:
#ifdef U8X8_MSG_GPIO_MENU_PREV_PIN
		u8x8_SetGPIOResult(u8x8, io_get_pinvalue(U8X8_MSG_GPIO_MENU_PREV_PIN));
#else
		u8x8_SetGPIOResult(u8x8, /* get menu next pin state */ 0);
#endif
		break;
	case U8X8_MSG_GPIO_MENU_HOME:
#ifdef U8X8_MSG_GPIO_MENU_HOME_PIN
		u8x8_SetGPIOResult(u8x8, io_get_pinvalue(U8X8_MSG_GPIO_MENU_HOME_PIN));
#else
		u8x8_SetGPIOResult(u8x8, /* get menu next pin state */ 0);
#endif
		break;
	default:
		u8x8_SetGPIOResult(u8x8, 1); // default return value
		break;
	}
	return 1;
}

#define LCDWIDTH u8g2_GetDisplayWidth(&u8g2)
#define LCDHEIGHT u8g2_GetDisplayHeight(&u8g2)
#define FONTHEIGHT (u8g2_GetAscent(&u8g2) + u8g2_GetDescent(&u8g2))
#define ALIGN_CENTER(t) ((LCDWIDTH - u8g2_GetUTF8Width(&u8g2, t)) / 2)
#define ALIGN_RIGHT(t) (LCDWIDTH - u8g2_GetUTF8Width(&u8g2, t))
#define ALIGN_LEFT 0
#define JUSTIFY_CENTER ((LCDHEIGHT + FONTHEIGHT) / 2)
#define JUSTIFY_BOTTOM (LCDHEIGHT - u8g2_GetDescent(&u8g2))
#define JUSTIFY_TOP u8g2_GetAscent(&u8g2)

#ifdef ENABLE_MAIN_LOOP_MODULES
/**
 * Handles SD card in the main loop
 * */
uint8_t graphic_lcd_loop(void *args, bool *handled)
{
	return STATUS_OK;
}

CREATE_EVENT_LISTENER(cnc_dotasks, graphic_lcd_loop);
#endif

#define GLCD_UCNC_STR __romstr__("µCNC")
#define GLCD_UCNC_VERSION_STR __romstr__(("v" CNC_VERSION))

void graphic_lcd_start_screen(void)
{
	char buff[32];
	rom_strcpy(buff, GLCD_UCNC_STR);
	u8g2_ClearBuffer(&u8g2);
	u8g2_SetFont(&u8g2, u8g2_font_9x15_t_symbols);
	u8g2_DrawUTF8X2(&u8g2, (LCDWIDTH / 2 - u8g2_GetUTF8Width(&u8g2, buff)), JUSTIFY_CENTER - FONTHEIGHT, buff);
	rom_strcpy(buff, GLCD_UCNC_VERSION_STR);
	u8g2_SetFont(&u8g2, u8g2_font_6x12_t_symbols);
	u8g2_DrawStr(&u8g2, ALIGN_CENTER(buff), JUSTIFY_CENTER + (2 * FONTHEIGHT), buff);
	u8g2_SendBuffer(&u8g2);
}

char lcd_x_axis[] __rom__ = "X: %0.3f";
char lcd_y_axis[] __rom__ = "Y: %0.3f";
char lcd_z_axis[] __rom__ = "Z: %0.3f";
char lcd_a_axis[] __rom__ = "A: %0.3f";
char lcd_b_axis[] __rom__ = "B: %0.3f";
char lcd_c_axis[] __rom__ = "C: %0.3f";
const char *lcd_axis[] __rom__ = {
	lcd_x_axis,
	lcd_y_axis,
	lcd_z_axis,
	lcd_a_axis,
	lcd_b_axis,
	lcd_c_axis};

void graphic_lcd_system_status_screen(void)
{
	char buff[32];
	char buffer[40];
	rom_strcpy(buff, (const char *)rom_ptr(&(lcd_axis[0])));
	LCD_putstr(buffer);
	rom_strcpy(buff, GLCD_UCNC_STR);
	u8g2_ClearBuffer(&u8g2);
	u8g2_SetFont(&u8g2, u8g2_font_9x15_t_symbols);
	u8g2_DrawStr(&u8g2, ALIGN_CENTER(buff), JUSTIFY_CENTER - FONTHEIGHT, buff);
	rom_strcpy(buff, GLCD_UCNC_VERSION_STR);
	u8g2_SetFont(&u8g2, u8g2_font_6x12_t_symbols);
	u8g2_DrawStr(&u8g2, ALIGN_CENTER(buff), JUSTIFY_CENTER + FONTHEIGHT, buff);
	u8g2_SendBuffer(&u8g2);
}

DECL_MODULE(graphic_lcd)
{
	u8g2_Setup_st7920_s_128x64_f(&u8g2, U8G2_R0, u8x8_byte_4wire_sw_spi, u8x8_gpio_and_delay_ucnc);
	u8g2_InitDisplay(&u8g2); // send init sequence to the display, display is in sleep mode after this,
	u8g2_ClearDisplay(&u8g2);
	u8g2_SetPowerSave(&u8g2, 0); // wake up display

	graphic_lcd_start_screen();
#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_dotasks, graphic_lcd_loop);
#else
#warning "Main loop extensions are not enabled. SD card will not work."
#endif
}
