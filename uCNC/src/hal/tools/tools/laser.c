/*
	Name: laser.c
	Description: Defines a laser tool using PWM0 for µCNC.
				 Defines a coolant output using DOUT1 (can be used for air assist).

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 17/12/2021

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../../cnc.h"

#include <stdbool.h>
#include <math.h>

/**
 * This configures a simple spindle control with a pwm assigned to PWM0 and dir invert assigned to DOUT0
 * This spindle also has a coolant pin assigned to DOUT1
 *
 * */

// give names to the pins (easier to identify)
#ifndef LASER_PWM
#define LASER_PWM PWM0
#endif

#ifndef LASER_FREQ
#define LASER_FREQ 8000
#endif

// #define ENABLE_COOLANT
#ifdef ENABLE_COOLANT
#ifndef COOLANT_FLOOD
#define COOLANT_FLOOD DOUT2
#endif
#ifndef COOLANT_MIST
#define COOLANT_MIST DOUT3
#endif
#endif

// this sets the minimum power (laser will never fully turn off during engraving and prevents power up delays)
#define PWM_MIN_VALUE 2

static bool previous_laser_mode;

void laser_startup_code(void)
{
// force laser mode
#if !(LASER_PWM < 0)
	mcu_config_pwm(LASER_PWM, LASER_FREQ);
	mcu_set_pwm(LASER_PWM, 0);
#else
	io_set_pwm(LASER_PWM, 0);
#endif
	previous_laser_mode = g_settings.laser_mode;
	g_settings.laser_mode = LASER_PWM_MODE;
}

void laser_shutdown_code(void)
{
	// restore laser mode
	g_settings.laser_mode = previous_laser_mode;
}

void laser_set_speed(int16_t value)
{
// easy macro to execute the same code as below
// SET_LASER(LASER_PWM, value, invert);

// speed optimized version (in AVR it's 24 instruction cycles)
#if !(LASER_PWM < 0)
	mcu_set_pwm(LASER_PWM, (uint8_t)ABS(value));
#else
	io_set_pwm(LASER_PWM, (uint8_t)ABS(value));
#endif
}

int16_t laser_range_speed(int16_t value)
{
	// converts core tool speed to laser power (PWM)
	value = (int16_t)(PWM_MIN_VALUE + ((255.0f - PWM_MIN_VALUE) * (((float)value) / g_settings.spindle_max_rpm)));
	return value;
}

void laser_set_coolant(uint8_t value)
{
// easy macro
#ifdef ENABLE_COOLANT
	SET_COOLANT(COOLANT_FLOOD, COOLANT_MIST, value);
#endif
}

uint16_t laser_get_speed(void)
{
#if !(LASER_PWM < 0)
	float laser = (float)mcu_get_pwm(LASER_PWM) * g_settings.spindle_max_rpm * UINT8_MAX_INV;
	return (uint16_t)roundf(laser);
#else
	return 0;
#endif
}

const tool_t __rom__ laser = {
	.startup_code = &laser_startup_code,
	.shutdown_code = &laser_shutdown_code,
#if PID_CONTROLLERS > 0
	.pid_update = NULL,
	.pid_error = NULL,
#endif
	.range_speed = &laser_range_speed,
	.get_speed = &laser_get_speed,
	.set_speed = &laser_set_speed,
	.set_coolant = &laser_set_coolant};
