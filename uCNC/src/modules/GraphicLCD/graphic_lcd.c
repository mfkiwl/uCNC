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
#include <math.h>

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
#define U8X8_MSG_GPIO_I2C_CLOCK_PIN DIN30
#endif
#ifndef U8X8_MSG_GPIO_I2C_DATA_PIN
#define U8X8_MSG_GPIO_I2C_DATA_PIN DIN31
#endif

#ifndef GRAPHIC_LCD_REFRESH
#define GRAPHIC_LCD_REFRESH 500
#endif

static u8g2_t u8g2;

#define LCDWIDTH u8g2_GetDisplayWidth(&u8g2)
#define LCDHEIGHT u8g2_GetDisplayHeight(&u8g2)
#define FONTHEIGHT (u8g2_GetAscent(&u8g2) - u8g2_GetDescent(&u8g2))
#define ALIGN_CENTER(t) ((LCDWIDTH - u8g2_GetUTF8Width(&u8g2, t)) / 2)
#define ALIGN_RIGHT(t) (LCDWIDTH - u8g2_GetUTF8Width(&u8g2, t))
#define ALIGN_LEFT 0
#define JUSTIFY_CENTER ((LCDHEIGHT + FONTHEIGHT) / 2)
#define JUSTIFY_BOTTOM (LCDHEIGHT + u8g2_GetDescent(&u8g2))
#define JUSTIFY_TOP u8g2_GetAscent(&u8g2)

typedef struct
{
	uint32_t screen_timeout;
	int8_t current_screen;
	int8_t current_menu;
} screen_options_t;

static screen_options_t display_screen;

typedef struct
{
	char menu_name[32];
	void (*drawmenu)(void *);
} graphic_lcd_menu_t;

typedef struct graphic_lcd_menu_item_
{
	const graphic_lcd_menu_t *menu;
	struct graphic_lcd_menu_item_ *next;
} graphic_lcd_menu_item_t;

static graphic_lcd_menu_item_t *graphic_lcd_menu_entry;

void graphic_lcd_add_menu(graphic_lcd_menu_item_t *menu)
{
	graphic_lcd_menu_item_t *ptr = graphic_lcd_menu_entry;

	while (ptr != NULL)
	{
		ptr = ptr->next;
	}
	ptr = menu;
}

void graphic_lcd_hold_menu(void* ptr) {}
const graphic_lcd_menu_t hold_menu __rom__ = {"hold", graphic_lcd_hold_menu};
static const graphic_lcd_menu_item_t hold_menu_entry = {&hold_menu, NULL};

/**
 *
 * can also be done via hardware SPI and I2C libraries of µCNC
 * but is not needed
 *
 * */

// #ifdef MCU_HAS_SPI
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
// #endif

// #ifdef MCU_HAS_I2C
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
// 		serial_print_int(buf_idx);
// 		softi2c_send(NULL, u8x8_GetI2CAddress(u8x8) >> 1, buffer, (int)buf_idx);
// 		break;
// 	default:
// 		return 0;
// 	}

// 	return 1;
// }
// #endif

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
		while (arg_int--)
			mcu_delay_100ns();
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
		if (arg_int)
		{
			mcu_config_input(U8X8_MSG_GPIO_I2C_CLOCK_PIN);
			mcu_config_pullup(U8X8_MSG_GPIO_I2C_CLOCK_PIN);
			u8x8_SetGPIOResult(u8x8, mcu_get_input(U8X8_MSG_GPIO_I2C_CLOCK_PIN));
		}
		else
		{
			mcu_config_output(U8X8_MSG_GPIO_I2C_CLOCK_PIN);
			mcu_clear_output(U8X8_MSG_GPIO_I2C_CLOCK_PIN);
			u8x8_SetGPIOResult(u8x8, 0);
		}
#endif
		break;					 // arg_int=1: Input dir with pullup high for I2C clock pin
	case U8X8_MSG_GPIO_I2C_DATA: // arg_int=0: Output low at I2C data pin
#ifdef U8X8_MSG_GPIO_I2C_DATA_PIN
		if (arg_int)
		{
			mcu_config_input(U8X8_MSG_GPIO_I2C_DATA_PIN);
			mcu_config_pullup(U8X8_MSG_GPIO_I2C_DATA_PIN);
			u8x8_SetGPIOResult(u8x8, mcu_get_input(U8X8_MSG_GPIO_I2C_DATA_PIN));
		}
		else
		{
			mcu_config_output(U8X8_MSG_GPIO_I2C_DATA_PIN);
			mcu_clear_output(U8X8_MSG_GPIO_I2C_DATA_PIN);
			u8x8_SetGPIOResult(u8x8, 0);
		}
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

/**
 * Helper functions for numbers
 * */

void ftoa(float __val, char *__s, int __radix)
{
	uint8_t i = 0;
	if (__val < 0)
	{
		__s[i++] = '-';
		__val = -__val;
	}

	uint32_t interger = (uint32_t)floorf(__val);
	__val -= interger;
	uint32_t mult = (interger < 10) ? 1000 : ((interger < 100) ? 100 : 10);
	__val *= mult;
	uint32_t digits = (uint32_t)roundf(__val);
	if (digits == mult)
	{
		interger++;
		digits = 0;
	}

	itoa(interger, &__s[i], 10);
	while (__s[i] != 0)
	{
		i++;
	}
	__s[i++] = '.';
	if ((mult == 1000) && (digits < 100))
	{
		__s[i++] = '0';
	}

	if ((mult >= 100) && (digits < 10))
	{
		__s[i++] = '0';
	}

	itoa(digits, &__s[i], 10);
}

void graphic_lcd_start_screen(void)
{
	char buff[32];
	rom_strcpy(buff, __romstr__("µCNC"));
	u8g2_ClearBuffer(&u8g2);
	u8g2_SetFont(&u8g2, u8g2_font_9x15_t_symbols);
	u8g2_DrawUTF8X2(&u8g2, (LCDWIDTH / 2 - u8g2_GetUTF8Width(&u8g2, buff)), JUSTIFY_CENTER - FONTHEIGHT / 2, buff);
	rom_strcpy(buff, __romstr__(("v" CNC_VERSION)));
	u8g2_SetFont(&u8g2, u8g2_font_6x12_tr);
	u8g2_DrawStr(&u8g2, ALIGN_CENTER(buff), JUSTIFY_CENTER + FONTHEIGHT, buff);
	u8g2_SendBuffer(&u8g2);
}

void graphic_lcd_system_status_screen(void)
{
	// starts from the bottom up

	// coordinates
	char buff[32];
	uint8_t y = JUSTIFY_BOTTOM;

	float axis[MAX(AXIS_COUNT, 3)];
	int32_t steppos[STEPPER_COUNT];
	itp_get_rt_position(steppos);
	kinematics_apply_forward(steppos, axis);
	kinematics_apply_reverse_transform(axis);

#if (AXIS_COUNT >= 4)
	memset(buff, 0, 32);
	buff[0] = 'A';
	ftoa(axis[3], &buff[1], 10);
	u8g2_DrawStr(&u8g2, ALIGN_LEFT, y, buff);

#if (AXIS_COUNT >= 5)
	buff[0] = 'B';
	ftoa(axis[4], &buff[1], 10);
	u8g2_DrawStr(&u8g2, ALIGN_CENTER(buff), y, buff);
#endif
#if (AXIS_COUNT >= 6)
	buff[0] = 'C';
	ftoa(axis[5], &buff[1], 10);
	u8g2_DrawStr(&u8g2, ALIGN_RIGHT(buff), y, buff);
#endif
	y -= FONTHEIGHT;
#endif

	memset(buff, 0, 32);
	u8g2_DrawLine(&u8g2, 0, y - FONTHEIGHT - 1, LCDWIDTH, y - FONTHEIGHT - 1);

#if (AXIS_COUNT >= 1)
	buff[0] = 'X';
	ftoa(axis[0], &buff[1], 10);
	u8g2_DrawStr(&u8g2, ALIGN_LEFT, y, buff);
#endif
#if (AXIS_COUNT >= 2)
	buff[0] = 'Y';
	ftoa(axis[1], &buff[1], 10);
	u8g2_DrawStr(&u8g2, ALIGN_CENTER(buff), y, buff);
#endif
#if (AXIS_COUNT >= 3)
	buff[0] = 'Z';
	ftoa(axis[2], &buff[1], 10);
	u8g2_DrawStr(&u8g2, ALIGN_RIGHT(buff), y, buff);
#endif

	memset(buff, 0, 32);
	y -= (FONTHEIGHT + 3);

	// units, feed and tool
	if (g_settings.report_inches)
	{
		rom_strcpy(buff, __romstr__("IN F "));
	}
	else
	{
		rom_strcpy(buff, __romstr__("MM F "));
	}

	// Realtime feed
	ftoa(itp_get_rt_feed(), &buff[5], 10);
	u8g2_DrawStr(&u8g2, ALIGN_LEFT, y, buff);
	memset(buff, 0, 32);

	// Tool
	char tool[5];
	uint8_t modalgroups[12];
	uint16_t feed;
	uint16_t spindle;
	uint8_t coolant;
	parser_get_modes(modalgroups, &feed, &spindle, &coolant);
	rom_strcpy(tool, __romstr__(" T "));
	itoa(modalgroups[11], &tool[3], 10);
	// Realtime tool speed
	rom_strcpy(buff, __romstr__("S "));
	itoa(tool_get_speed(), &buff[2], 10);
	strcat(buff, tool);
	u8g2_DrawStr(&u8g2, ALIGN_RIGHT(buff), y, buff);
	memset(buff, 0, 32);
	u8g2_DrawLine(&u8g2, 0, y - FONTHEIGHT - 1, LCDWIDTH, y - FONTHEIGHT - 1);

	y -= (FONTHEIGHT + 3);

	// system status
	uint8_t i;

	rom_strcpy(buff, __romstr__("St:"));
	uint8_t state = cnc_get_exec_state(0xFF);
	uint8_t filter = 0x80;
	while (!(state & filter) && filter)
	{
		filter >>= 1;
	}

	state &= filter;
	if (cnc_has_alarm())
	{
		rom_strcpy(&buff[3], MSG_STATUS_ALARM);
	}
	else if (mc_get_checkmode())
	{
		rom_strcpy(&buff[3], MSG_STATUS_CHECK);
	}
	else
	{
		switch (state)
		{
		case EXEC_DOOR:
			rom_strcpy(&buff[3], MSG_STATUS_DOOR);
			break;
		case EXEC_HALT:
			rom_strcpy(&buff[3], MSG_STATUS_ALARM);
			break;
		case EXEC_HOLD:
			rom_strcpy(&buff[3], MSG_STATUS_HOLD);
			break;
		case EXEC_HOMING:
			rom_strcpy(&buff[3], MSG_STATUS_HOME);
			break;
		case EXEC_JOG:
			rom_strcpy(&buff[3], MSG_STATUS_JOG);
			break;
		case EXEC_RESUMING:
		case EXEC_RUN:
			rom_strcpy(&buff[3], MSG_STATUS_RUN);
			break;
		default:
			rom_strcpy(&buff[3], MSG_STATUS_IDLE);
			break;
		}
	}
	u8g2_DrawStr(&u8g2, ALIGN_LEFT, y, buff);
	memset(buff, 0, 32);

	uint8_t controls = io_get_controls();
	uint8_t limits = io_get_limits();
	uint8_t probe = io_get_probe();
	rom_strcpy(buff, __romstr__("Sw:"));
	i = 3;
	if (CHECKFLAG(controls, (ESTOP_MASK | SAFETY_DOOR_MASK | FHOLD_MASK)) || CHECKFLAG(limits, LIMITS_MASK) || probe)
	{
		if (CHECKFLAG(controls, ESTOP_MASK))
		{
			buff[i++] = 'R';
		}

		if (CHECKFLAG(controls, SAFETY_DOOR_MASK))
		{
			buff[i++] = 'D';
		}

		if (CHECKFLAG(controls, FHOLD_MASK))
		{
			buff[i++] = 'H';
		}

		if (probe)
		{
			buff[i++] = 'P';
		}

		if (CHECKFLAG(limits, LIMIT_X_MASK))
		{
			buff[i++] = 'X';
		}

		if (CHECKFLAG(limits, LIMIT_Y_MASK))
		{
			buff[i++] = 'Y';
		}

		if (CHECKFLAG(limits, LIMIT_Z_MASK))
		{
			buff[i++] = 'Z';
		}

		if (CHECKFLAG(limits, LIMIT_A_MASK))
		{
			buff[i++] = 'A';
		}

		if (CHECKFLAG(limits, LIMIT_B_MASK))
		{
			buff[i++] = 'B';
		}

		if (CHECKFLAG(limits, LIMIT_C_MASK))
		{
			buff[i++] = 'C';
		}
	}
	u8g2_DrawStr(&u8g2, LCDWIDTH / 2, y, buff);
}

void graphic_lcd_system_draw_menu(void)
{
	char buff[32];
	graphic_lcd_menu_item_t *ptr = graphic_lcd_menu_entry;
	graphic_lcd_menu_t menu;

	while (ptr != NULL)
	{
		rom_memcpy(&menu, ptr->menu, sizeof(graphic_lcd_menu_t));
		u8g2_DrawStr(&u8g2, 1, FONTHEIGHT, menu.menu_name);
		ptr = ptr->next;
	}
}

#ifdef ENABLE_MAIN_LOOP_MODULES
/**
 * Handles SD card in the main loop
 * */
uint8_t graphic_lcd_loop(void *args, bool *handled)
{
	static uint32_t refresh = 0;
	if (display_screen.screen_timeout < mcu_millis())
	{
		u8g2_SetFont(&u8g2, u8g2_font_tinytim_tf);
		display_screen.current_screen = 0;
		display_screen.current_menu = 0;
	}

	if (refresh < mcu_millis())
	{
		u8g2_ClearBuffer(&u8g2);
		switch (display_screen.current_screen)
		{
		case -1:
			graphic_lcd_start_screen();
			break;
		case 0:
			graphic_lcd_system_status_screen();
			graphic_lcd_system_draw_menu();
			break;
		}

		u8g2_NextPage(&u8g2);
		refresh = mcu_millis() + GRAPHIC_LCD_REFRESH;
	}

	return STATUS_OK;
}

CREATE_EVENT_LISTENER(cnc_dotasks, graphic_lcd_loop);

uint8_t graphic_lcd_start(void *args, bool *handled)
{
	u8g2_Setup_st7920_s_128x64_f(&u8g2, U8G2_R0, u8x8_byte_4wire_sw_spi, u8x8_gpio_and_delay_ucnc);
	// u8g2_Setup_ssd1306_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_sw_i2c, u8x8_gpio_and_delay_ucnc);
	u8g2_InitDisplay(&u8g2); // send init sequence to the display, display is in sleep mode after this,
	u8g2_ClearDisplay(&u8g2);
	u8g2_SetPowerSave(&u8g2, 0); // wake up display
	display_screen.screen_timeout = mcu_millis() + 5000;
	display_screen.current_screen = -1;
	u8g2_FirstPage(&u8g2);
}

CREATE_EVENT_LISTENER(cnc_reset, graphic_lcd_start);
#endif

DECL_MODULE(graphic_lcd)
{
	graphic_lcd_add_menu(&hold_menu_entry);

#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_reset, graphic_lcd_start);
	ADD_EVENT_LISTENER(cnc_dotasks, graphic_lcd_loop);
#else
#warning "Main loop extensions are not enabled. SD card will not work."
#endif
}
