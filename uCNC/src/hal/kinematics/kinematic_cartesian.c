/*
		Name: kinematic_cartesian.c
		Description: Implements all kinematics math equations to translate the motion of a cartesian machine.
				Also implements the homing motion for this type of machine.

		Copyright: Copyright (c) João Martins
		Author: João Martins
		Date: 26/09/2019

		µCNC is free software: you can redistribute it and/or modify
		it under the terms of the GNU General Public License as published by
		the Free Software Foundation, either version 3 of the License, or
		(at your option) any later version. Please see <http://www.gnu.org/licenses/>

		µCNC is distributed WITHOUT ANY WARRANTY;
		Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
		See the	GNU General Public License for more details.
*/

#include "../../cnc.h"

#if (KINEMATIC == KINEMATIC_CARTESIAN)
#include <stdio.h>
#include <math.h>

void kinematics_init(void)
{
}

void kinematics_apply_inverse(float *axis, int32_t *steps)
{
	for (uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		steps[i] = (int32_t)lroundf(g_settings.step_per_mm[i] * axis[i]);
	}
}

void kinematics_apply_forward(int32_t *steps, float *axis)
{
	for (uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		axis[i] = (((float)steps[i]) / g_settings.step_per_mm[i]);
	}
}

uint8_t kinematics_home(void)
{
	float target[AXIS_COUNT];
	
#ifndef DISABLE_ALL_LIMITS
#ifndef DISABLE_Z_HOMING
#if (defined(AXIS_Z) && (ASSERT_PIN(LIMIT_Z) || ASSERT_PIN(LIMIT_Z2)))
	if (mc_home_axis(AXIS_Z, LIMIT_Z_MASK))
	{
		return KINEMATIC_HOMING_ERROR_Z;
	}
#endif
#endif

#ifndef DISABLE_X_HOMING
#if (defined(AXIS_X) && (ASSERT_PIN(LIMIT_X) || ASSERT_PIN(LIMIT_X2)))
	if (mc_home_axis(AXIS_X, LIMIT_X_MASK))
	{
		return KINEMATIC_HOMING_ERROR_X;
	}
#endif
#endif

#ifndef DISABLE_Y_HOMING
#if (defined(AXIS_Y) && (ASSERT_PIN(LIMIT_Y) || ASSERT_PIN(LIMIT_Y2)))
	if (mc_home_axis(AXIS_Y, LIMIT_Y_MASK))
	{
		return KINEMATIC_HOMING_ERROR_Y;
	}
#endif
#endif

#ifndef DISABLE_A_HOMING
#if (defined(AXIS_A) && ASSERT_PIN(LIMIT_A))
	if (mc_home_axis(AXIS_A, LIMIT_A_MASK))
	{
		return KINEMATIC_HOMING_ERROR_A;
	}
#endif
#endif

#ifndef DISABLE_B_HOMING
#if (defined(AXIS_B) && ASSERT_PIN(LIMIT_B))
	if (mc_home_axis(AXIS_B, LIMIT_B_MASK))
	{
		return KINEMATIC_HOMING_ERROR_B;
	}
#endif
#endif

#ifndef DISABLE_C_HOMING
#if (defined(AXIS_C) && ASSERT_PIN(LIMIT_C))
	if (mc_home_axis(AXIS_C, LIMIT_C_MASK))
	{
		return KINEMATIC_HOMING_ERROR_C;
	}
#endif
#endif

	cnc_unlock(true);
	// flags homing clear by the unlock
	cnc_set_exec_state(EXEC_HOMING);
	motion_data_t block_data = {0};
	mc_get_position(target);

	for (uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		target[i] += ((g_settings.homing_dir_invert_mask & (1 << i)) ? -g_settings.homing_offset : g_settings.homing_offset);
	}

	block_data.feed = g_settings.homing_fast_feed_rate;
	block_data.spindle = 0;
	block_data.dwell = 0;
	// starts offset and waits to finnish
	mc_line(target, &block_data);
	itp_sync();
#endif
	// unlocks the machine to go to offset
	cnc_clear_exec_state(EXEC_HOMING);

#ifdef SET_ORIGIN_AT_HOME_POS
	memset(target, 0, sizeof(target));
#else
	for (uint8_t i = AXIS_COUNT; i != 0;)
	{
		i--;
		target[i] = (!(g_settings.homing_dir_invert_mask & (1 << i)) ? 0 : g_settings.max_distance[i]);
	}
#endif

	// reset position
	itp_reset_rt_position(target);

	return STATUS_OK;
}

void kinematics_apply_transform(float *axis)
{
	/*
	Define your custom transform
*/
#ifdef ENABLE_SKEW_COMPENSATION
	// apply correction skew factors that compensate for machine axis alignemnt
	axis[AXIS_X] -= axis[AXIS_Y] * g_settings.skew_xy_factor;
#ifndef SKEW_COMPENSATION_XY_ONLY
	axis[AXIS_X] -= axis[AXIS_Z] * (g_settings.skew_xy_factor - g_settings.skew_xz_factor * g_settings.skew_yz_factor);
	axis[AXIS_Y] -= axis[AXIS_Z] * g_settings.skew_yz_factor;
#endif
#endif
}

void kinematics_apply_reverse_transform(float *axis)
{
	/*
	Define your custom transform inverse operation
*/

	// perform unskew of the coordinates
#ifdef ENABLE_SKEW_COMPENSATION
	axis[AXIS_X] += axis[AXIS_Y] * g_settings.skew_xy_factor;
#ifndef SKEW_COMPENSATION_XY_ONLY
	axis[AXIS_X] += axis[AXIS_Z] * g_settings.skew_xz_factor;
	axis[AXIS_Y] += axis[AXIS_Z] * g_settings.skew_yz_factor;
#endif
#endif
}

bool kinematics_check_boundaries(float *axis)
{
	if (!g_settings.soft_limits_enabled || cnc_get_exec_state(EXEC_HOMING))
	{
		return true;
	}

	for (uint8_t i = AXIS_COUNT; i != 0;)
	{
		i--;
#ifdef SET_ORIGIN_AT_HOME_POS
		float value = !(g_settings.homing_dir_invert_mask & (1 << i)) ? -axis[i] : axis[i];
#else
		float value = axis[i];
#endif
		if (value > g_settings.max_distance[i] || value < 0)
		{
			return false;
		}
	}

	return true;
}

#endif
