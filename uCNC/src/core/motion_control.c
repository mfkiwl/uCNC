/*
	Name: motion_control.c
	Description: Contains the building blocks for performing motions/actions in µCNC
	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 19/11/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../cnc.h"
#include <math.h>
#include <string.h>
#include <float.h>

static bool mc_checkmode;
static int32_t mc_last_step_pos[STEPPER_COUNT];
static float mc_last_target[AXIS_COUNT];
// static float mc_prev_target_dir[AXIS_COUNT];
#ifdef ENABLE_BACKLASH_COMPENSATION
static uint8_t mc_last_dirbits;
#endif

#ifdef ENABLE_MOTION_CONTROL_MODULES
// event_mc_line_segment_handler
WEAK_EVENT_HANDLER(mc_line_segment)
{
	DEFAULT_EVENT_HANDLER(mc_line_segment);
}
#endif

void mc_init(void)
{
#ifdef FORCE_GLOBALS_TO_0
	memset(mc_last_step_pos, 0, sizeof(mc_last_step_pos));
	memset(mc_last_target, 0, sizeof(mc_last_target));
#ifdef ENABLE_BACKLASH_COMPENSATION
	mc_last_dirbits = 0;
#endif
#endif
	mc_checkmode = false;
	mc_sync_position();
}

bool mc_get_checkmode(void)
{
	return mc_checkmode;
}

bool mc_toogle_checkmode(void)
{
	mc_checkmode = !mc_checkmode;
	return mc_checkmode;
}

static uint8_t mc_line_segment(int32_t *step_new_pos, motion_data_t *block_data)
{
// resets accumulator vars of the block
#ifdef ENABLE_LINACT_PLANNER
	block_data->full_steps = 0;
#endif
	block_data->total_steps = 0;
#if KINEMATIC == KINEMATIC_DELTA
	block_data->dirbits = 0;
#endif
	for (uint8_t i = STEPPER_COUNT; i != 0;)
	{
		i--;
		int32_t s = step_new_pos[i] - mc_last_step_pos[i];
		uint32_t steps = (uint32_t)ABS(s);
		block_data->steps[i] = (step_t)steps;

#if KINEMATIC == KINEMATIC_DELTA
		// with the delta dir bits need to be rechecked
		if (s < 0)
		{
			block_data->dirbits |= (1 << i);
		}
#endif

#ifdef ENABLE_LINACT_PLANNER
		block_data->full_steps += steps;
#endif
		if (block_data->total_steps < steps)
		{
			block_data->total_steps = steps;
#if KINEMATIC == KINEMATIC_DELTA
			// with the delta main stepper need to be rechecked
			block_data->main_stepper = i;
#endif
		}
	}

	// no significant motion will take place. don't send any thing to the planner
	if (!(block_data->total_steps))
	{
		return STATUS_OK;
	}

	// stores current step target position
	memcpy(mc_last_step_pos, step_new_pos, sizeof(mc_last_step_pos));

	if (!mc_checkmode) // check mode (gcode simulation) doesn't send code to planner
	{
#ifdef ENABLE_BACKLASH_COMPENSATION
		// checks if any of the linear actuators there is a shift in direction
		uint8_t inverted_steps = mc_last_dirbits ^ block_data->dirbits;
		if (inverted_steps)
		{
			motion_data_t backlash_block_data = {0};
			memcpy(&backlash_block_data, block_data, sizeof(motion_data_t));
			memset(backlash_block_data.steps, 0, sizeof(backlash_block_data.steps));
			// resets accumulator vars
			backlash_block_data.total_steps = 0;
#ifdef ENABLE_LINACT_PLANNER
			backlash_block_data.full_steps = 0;
#endif
			backlash_block_data.feed = FLT_MAX; // max feedrate possible (same as rapid move)

			SETFLAG(backlash_block_data.motion_mode, MOTIONCONTROL_MODE_BACKLASH_COMPENSATION);

			for (uint8_t i = STEPPER_COUNT; i != 0;)
			{
				i--;
				if (inverted_steps & (1 << i))
				{
					backlash_block_data.steps[i] = g_settings.backlash_steps[i];
#ifdef ENABLE_LINACT_PLANNER
					backlash_block_data.full_steps += backlash_block_data.steps[i];
#endif
					if (backlash_block_data.total_steps < backlash_block_data.steps[i])
					{
						backlash_block_data.total_steps = backlash_block_data.steps[i];
						backlash_block_data.main_stepper = i;
					}
				}
			}

			while (planner_buffer_is_full())
			{
				if (!cnc_dotasks())
				{
					return STATUS_CRITICAL_FAIL;
				}
			}

			planner_add_line(&backlash_block_data);
			// dwell should only execute on the first request
			block_data->dwell = 0;
			mc_last_dirbits = block_data->dirbits;
		}
#endif

		while (planner_buffer_is_full())
		{
			if (!cnc_dotasks())
			{
				return STATUS_CRITICAL_FAIL;
			}
		}

#ifdef ENABLE_MOTION_CONTROL_MODULES
		// event_mc_line_segment_handler
		EVENT_INVOKE(mc_line_segment, block_data);
#endif

		planner_add_line(block_data);
		// dwell should only execute on the first request
		block_data->dwell = 0;
	}

	return STATUS_OK;
}

// all motions should go through mc_line before entering the final planner pipeline
// after this stage the motion follows a pipeline that performs the following steps
// 1. decouples the target point from the remaining pipeline
// 2. applies all kinematic transformations to the target
// 3. converts the target in actuator position
// 4. calculates motion change from the previous line
// 5. if line is to big to be executed correctly by the interpolator, breaks it in to smaller line segments
uint8_t mc_line(float *target, motion_data_t *block_data)
{
	float prev_target[AXIS_COUNT];
	block_data->dirbits = 0; // reset dirbits (this prevents odd behaviour generated by long arcs)

	// In homing mode no kinematics modifications is applied to prevent unwanted axis movements
	if (!cnc_get_exec_state(EXEC_HOMING))
	{
		kinematics_apply_transform(target);
	}

	// check travel limits (soft limits)
	if (!kinematics_check_boundaries(target))
	{
		if (cnc_get_exec_state(EXEC_JOG))
		{
			return STATUS_TRAVEL_EXCEEDED;
		}
		cnc_alarm(EXEC_ALARM_SOFT_LIMIT);
		return STATUS_OK;
	}

	uint8_t error = STATUS_OK;
	int32_t step_new_pos[STEPPER_COUNT];
	// converts transformed target to stepper position
	kinematics_apply_inverse(target, step_new_pos);
	// calculates the amount of stepper motion for this motion

	uint32_t max_steps = 0;
	block_data->main_stepper = 255;
	for (uint8_t i = AXIS_TO_STEPPERS; i != 0;)
	{
		i--;
		int32_t steps = step_new_pos[i] - mc_last_step_pos[i];
		if (steps < 0)
		{
			block_data->dirbits |= (1 << i);
		}

		steps = ABS(steps);
		if (max_steps < (uint32_t)steps)
		{
			max_steps = steps;
			block_data->main_stepper = i;
		}
	}

	// no significant motion will take place. don't send any thing to the planner
	if (!max_steps)
	{
		return STATUS_OK;
	}

	// checks the amount of steps that this motion translates to
	// if the amount of steps is higher than the limit for the 16bit bresenham algorithm
	// splits the line into smaller segments

	// gets the previous machine position (transformed to calculate the direction vector and travelled distance)
	memcpy(prev_target, mc_last_target, sizeof(mc_last_target));

	// calculates the aproximation of the inverted travelled distance
	float inv_dist = 0;
	float motion_segment[AXIS_COUNT];
	for (uint8_t i = AXIS_COUNT; i != 0;)
	{
		i--;
		motion_segment[i] = target[i] - prev_target[i];
		block_data->dir_vect[i] = motion_segment[i];
		inv_dist += fast_flt_pow2(block_data->dir_vect[i]);
	}

#if ((KINEMATIC == KINEMATIC_DELTA) || defined(ENABLE_LASER_PPI))
	float line_dist = fast_flt_sqrt(inv_dist);
	inv_dist = 1.0f / line_dist;
#else
	inv_dist = fast_flt_invsqrt(inv_dist);
#endif

	// calculates max junction speed factor in (axis driven). Else the cos_theta is calculated in the planner (linear actuator driven)
#ifndef ENABLE_LINACT_PLANNER
	for (uint8_t i = AXIS_COUNT; i != 0;)
	{
		i--;
		block_data->dir_vect[i] *= inv_dist;
	}
#endif

#ifdef ENABLE_LASER_PPI
	mc_last_step_pos[STEPPER_COUNT - 1] = 0;
	float laser_pulses_per_mm = 0;
	if (block_data->motion_flags.bit.spindle_running && block_data->spindle)
	{
		laser_pulses_per_mm = g_settings.step_per_mm[STEPPER_COUNT - 1];
		// modify PPI settings according o the S value
		if (g_settings.laser_mode & LASER_PPI_MODE)
		{
			float laser_ppi_scale = (float)block_data->spindle / (float)g_settings.spindle_max_rpm;
			if (g_settings.laser_mode & LASER_PPI_VARPOWER_MODE)
			{
				float blend = g_settings.laser_ppi_mixmode_ppi;
				laser_ppi_scale = (laser_ppi_scale * blend) + (1.0f - blend);
			}

			laser_pulses_per_mm *= laser_ppi_scale;
		}

		laser_pulses_per_mm *= line_dist;
	}
	step_new_pos[STEPPER_COUNT - 1] = laser_pulses_per_mm;
	max_steps = MAX(max_steps, step_new_pos[STEPPER_COUNT - 1]);
#endif

	// calculated the total motion execution time @ the given rate
	float feed = block_data->feed;
	float inv_delta = (!CHECKFLAG(block_data->motion_mode, MOTIONCONTROL_MODE_INVERSEFEED) ? (block_data->feed * inv_dist) : block_data->feed);
	block_data->feed = (float)max_steps * inv_delta;

#if ((KINEMATIC == KINEMATIC_DELTA) || defined(BRESENHAM_16BIT))
	// this contains a motion. Any tool update will be done here
	uint32_t line_segments = 1;
#if (KINEMATIC == KINEMATIC_DELTA)
	line_segments = (uint32_t)ceilf(line_dist * DELTA_MOTION_SEGMENT_FACTOR);
	float m_inv = 1.0f / (float)line_segments;
	for (uint8_t i = AXIS_COUNT; i != 0;)
	{
		i--;
		motion_segment[i] *= m_inv;
	}
#else
	if (max_steps > MAX_STEPS_PER_LINE)
	{
		line_segments += (max_steps >> MAX_STEPS_PER_LINE_BITS);
		float m_inv = 1.0f / (float)line_segments;
		for (uint8_t i = AXIS_COUNT; i != 0;)
		{
			i--;
			motion_segment[i] *= m_inv;
		}
	}
#endif

	while (--line_segments)
	{
		block_data->motion_flags.bit.is_subsegment = 1;
		for (uint8_t i = AXIS_COUNT; i != 0;)
		{
			i--;
			prev_target[i] += motion_segment[i];
		}

		kinematics_apply_inverse(prev_target, step_new_pos);
		error = mc_line_segment(step_new_pos, block_data);
		if (error)
		{
			memcpy(target, prev_target, sizeof(prev_target));
			block_data->feed = feed;
			return error;
		}
	}

	if (block_data->motion_flags.bit.is_subsegment)
	{
		kinematics_apply_inverse(target, step_new_pos);
	}
#endif
	error = mc_line_segment(step_new_pos, block_data);
	// stores the new position for the next motion
	memcpy(mc_last_target, target, sizeof(mc_last_target));
	block_data->feed = feed;
	block_data->motion_flags.bit.is_subsegment = 0;
	return error;
}

// applies an algorithm similar to grbl with slight changes
uint8_t mc_arc(float *target, float center_offset_a, float center_offset_b, float radius, uint8_t axis_0, uint8_t axis_1, bool isclockwise, motion_data_t *block_data)
{
	float mc_position[AXIS_COUNT];

	// copy motion control last position
	mc_get_position(mc_position);

	float ptcenter_a = mc_position[axis_0] + center_offset_a;
	float ptcenter_b = mc_position[axis_1] + center_offset_b;

	float pt0_a = -center_offset_a; // Radius vector from center to current location
	float pt0_b = -center_offset_b;
	float pt1_a = target[axis_0] - ptcenter_a; // Radius vector from center to current location
	float pt1_b = target[axis_1] - ptcenter_b;

	// dot product between vect_a and vect_b
	float dotprod = pt0_a * pt1_a + pt0_b * pt1_b;
	// determinant
	float det = pt0_a * pt1_b - pt0_b * pt1_a;
	float arc_angle = atan2(det, dotprod);

	if (isclockwise)
	{
		if (arc_angle >= 0)
		{
			arc_angle -= 2 * M_PI;
		}
	}
	else
	{
		if (arc_angle <= 0)
		{
			arc_angle += 2 * M_PI;
		}
	}

	// uses as temporary vars
	float radiusangle = radius * arc_angle;
	radiusangle = fast_flt_div2(radiusangle);
	float diameter = fast_flt_mul2(radius);
	uint16_t segment_count = floor(fabs(radiusangle) / sqrt(g_settings.arc_tolerance * (diameter - g_settings.arc_tolerance)));
	float arc_per_sgm = (segment_count != 0) ? arc_angle / segment_count : arc_angle;

	// for all other axis finds the linear motion distance
	float increment[AXIS_COUNT];

	for (uint8_t i = AXIS_COUNT; i != 0;)
	{
		i--;
		increment[i] = (target[i] - mc_position[i]) / segment_count;
	}

	increment[axis_0] = 0;
	increment[axis_1] = 0;

	if (CHECKFLAG(block_data->motion_mode, MOTIONCONTROL_MODE_INVERSEFEED))
	{
		// split the required time to complete the motion with the number of segments
		block_data->feed *= segment_count;
	}

	// calculates an aproximation to sine and cosine of the angle segment
	// improves the error for the cosine by calculating an extra term of the taylor series at the expence of an extra multiplication and addition
	// applies arc correction has grbl does
	float arc_per_sgm_sqr = arc_per_sgm * arc_per_sgm;
	float cos_per_sgm = 1 - M_COS_TAYLOR_1 * arc_per_sgm_sqr;
	float sin_per_sgm = arc_per_sgm * cos_per_sgm;
	cos_per_sgm = arc_per_sgm_sqr * (cos_per_sgm + 1);
	cos_per_sgm = 1 - fast_flt_div4(cos_per_sgm);

	uint8_t count = 0;

	for (uint16_t i = 1; i < segment_count; i++)
	{
		if (count < N_ARC_CORRECTION)
		{
			// Apply incremental vector rotation matrix.
			float new_pt = pt0_a * sin_per_sgm + pt0_b * cos_per_sgm;
			pt0_a = pt0_a * cos_per_sgm - pt0_b * sin_per_sgm;
			pt0_b = new_pt;
			count++;
		}
		else
		{
			// Arc correction to radius vector. Computed only every N_ARC_CORRECTION increments.
			// Compute exact location by applying transformation matrix from initial radius vector(=-offset).
			float angle = i * arc_per_sgm;
			float precise_cos = cos(angle);
			// calculates sine using sine and cosine relation equation
			//	sin(x)^2 + cos(x)^2 = 1
			//
			// this is executes in about 50% the time of a sin function
			// https://www.nongnu.org/avr-libc/user-manual/benchmarks.html
			float precise_sin = sqrt(1 - precise_cos * precise_cos);
			if (angle >= 0)
			{
				precise_sin = (ABS(angle) <= M_PI) ? precise_sin : -precise_sin;
			}
			else
			{
				precise_sin = (ABS(angle) <= M_PI) ? -precise_sin : precise_sin;
			}

			pt0_a = -center_offset_a * precise_cos + center_offset_b * precise_sin;
			pt0_b = -center_offset_a * precise_sin - center_offset_b * precise_cos;
			count = 0;
		}

		// Update arc_target location
		mc_position[axis_0] = ptcenter_a + pt0_a;
		mc_position[axis_1] = ptcenter_b + pt0_b;
		for (uint8_t i = AXIS_COUNT; i != 0;)
		{
			i--;
			if (i != axis_0 && i != axis_1)
			{
				mc_position[i] += increment[i];
			}
		}

		uint8_t error = mc_line(mc_position, block_data);
		if (error)
		{
			return error;
		}
	}
	// Ensure last segment arrives at target location.
	return mc_line(target, block_data);
}

uint8_t mc_dwell(motion_data_t *block_data)
{
	if (!mc_checkmode) // check mode (gcode simulation) doesn't send code to planner
	{
		mc_update_tools(block_data);
		cnc_delay_ms(block_data->dwell);
	}

	return STATUS_OK;
}

uint8_t mc_pause(void)
{
	if (!mc_checkmode) // check mode (gcode simulation) doesn't send code to planner
	{
		if (itp_sync() != STATUS_OK)
		{
			return STATUS_CRITICAL_FAIL;
		}
		cnc_set_exec_state(EXEC_HOLD);
	}
	return STATUS_OK;
}

uint8_t mc_update_tools(motion_data_t *block_data)
{
	if (!mc_checkmode) // check mode (gcode simulation) doesn't send code to planner
	{
		if (itp_sync() != STATUS_OK)
		{
			return STATUS_CRITICAL_FAIL;
		}
		// synchronizes the tools
		planner_sync_tools(block_data);
		itp_sync_spindle();
	}

	return STATUS_OK;
}

uint8_t mc_home_axis(uint8_t axis, uint8_t axis_limit)
{
	float target[AXIS_COUNT];
	uint8_t axis_mask = (1 << axis);
	motion_data_t block_data;
	uint8_t limits_flags;

	cnc_unlock(true);

	// locks limits to accept axis limit mask only or else throw error
	io_lock_limits(axis_limit);
	io_invert_limits(0);
	// if HOLD or ALARM are still active or any limit switch is not cleared fails to home
	mcu_limits_changed_cb();
	if (cnc_get_exec_state(EXEC_HOLD | EXEC_ALARM) || CHECKFLAG(io_get_limits(), LIMITS_MASK))
	{
		cnc_alarm(EXEC_ALARM_HOMING_FAIL_LIMIT_ACTIVE);
		return STATUS_CRITICAL_FAIL;
	}

	float max_home_dist;
	max_home_dist = -g_settings.max_distance[axis] * 1.5f;

	// checks homing dir
	if (g_settings.homing_dir_invert_mask & axis_mask)
	{
		max_home_dist = -max_home_dist;
	}

	// sync's the motion control with the real time position
	mc_sync_position();
	mc_get_position(target);
	target[axis] += max_home_dist;
	// initializes planner block data
	block_data.total_steps = ABS(max_home_dist);
	memset(block_data.steps, 0, sizeof(block_data.steps));
	block_data.steps[axis] = max_home_dist;
	block_data.feed = g_settings.homing_fast_feed_rate;
	block_data.spindle = 0;
	block_data.dwell = 0;
	block_data.motion_mode = MOTIONCONTROL_MODE_FEED;

	cnc_unlock(true);
	// re-flags homing clear by the unlock
	cnc_set_exec_state(EXEC_HOMING);
	mc_line(target, &block_data);

	if (itp_sync() != STATUS_OK)
	{
		return STATUS_CRITICAL_FAIL;
	}

	// flushes buffers
	itp_stop();
	itp_clear();
	planner_clear();

	cnc_delay_ms(g_settings.debounce_ms); // adds a delay before reading io pin (debounce)
	limits_flags = io_get_limits();

	// the wrong switch was activated bails
	if (!CHECKFLAG(limits_flags, axis_limit))
	{
		cnc_set_exec_state(EXEC_HALT);
		cnc_alarm(EXEC_ALARM_HOMING_FAIL_APPROACH);
		return STATUS_CRITICAL_FAIL;
	}

	// back off from switch at lower speed
	max_home_dist = g_settings.homing_offset * 5.0f;

	// sync's the motion control with the real time position
	mc_sync_position();
	mc_get_position(target);
	if (g_settings.homing_dir_invert_mask & axis_mask)
	{
		max_home_dist = -max_home_dist;
	}

	target[axis] += max_home_dist;
	block_data.feed = g_settings.homing_slow_feed_rate;
	block_data.total_steps = ABS(max_home_dist);
	block_data.steps[axis] = max_home_dist;
	// unlocks the machine for next motion (this will clear the EXEC_HALT flag
	// temporary inverts the limit mask to trigger ISR on switch release
	io_invert_limits(axis_limit);

	cnc_unlock(true);
	// flags homing clear by the unlock
	cnc_set_exec_state(EXEC_HOMING);
	mc_line(target, &block_data);

	if (itp_sync() != STATUS_OK)
	{
		// restores limits mask
		return STATUS_CRITICAL_FAIL;
	}

	cnc_delay_ms(g_settings.debounce_ms); // adds a delay before reading io pin (debounce)
	// resets limit mask
	io_invert_limits(0);
	// stops, flushes buffers and clears the hold if active
	cnc_stop();
	// clearing the interpolator unlockes any locked stepper
	itp_clear();
	planner_clear();

	cnc_delay_ms(g_settings.debounce_ms); // adds a delay before reading io pin (debounce)
	limits_flags = io_get_limits();

	if (CHECKFLAG(limits_flags, axis_limit))
	{
		cnc_set_exec_state(EXEC_HALT);
		cnc_alarm(EXEC_ALARM_HOMING_FAIL_APPROACH);
		return STATUS_CRITICAL_FAIL;
	}

	return STATUS_OK;
}

uint8_t mc_probe(float *target, uint8_t flags, motion_data_t *block_data)
{
#if !(PROBE < 0)
	uint8_t prev_state = cnc_get_exec_state(EXEC_HOLD);
	io_enable_probe();
	mc_line(target, block_data);

	do
	{
		if (!cnc_dotasks())
		{
			return STATUS_CRITICAL_FAIL;
		}

#if (defined(FORCE_SOFT_POLLING) || (PROBEEN_MASK != PROBEISR_MASK))
		if (io_get_probe() ^ (flags & 0x01))
		{
			mcu_probe_changed_cb();
			break;
		}
#endif
	} while (cnc_get_exec_state(EXEC_RUN));

	io_disable_probe();
	cnc_stop();
	itp_clear();
	planner_clear();
	parser_update_probe_pos();
	// sync the position of the motion control
	mc_sync_position();
	cnc_clear_exec_state(~prev_state | ~EXEC_HOLD); // restores HOLD previous state
	cnc_delay_ms(g_settings.debounce_ms);			// adds a delay before reading io pin (debounce)
	bool probe_ok = io_get_probe();
	probe_ok = (flags & MOTIONCONTROL_PROBE_INVERT) ? !probe_ok : probe_ok;
	if (!probe_ok)
	{
		if (!(flags & MOTIONCONTROL_PROBE_NOALARM_ONFAIL))
		{
			cnc_alarm(EXEC_ALARM_PROBE_FAIL_CONTACT);
		}
		return STATUS_OK;
	}

#endif

	return STATUS_PROBE_SUCCESS;
}

void mc_get_position(float *target)
{
	memcpy(target, mc_last_target, sizeof(mc_last_target));
	kinematics_apply_reverse_transform(target);
}

void mc_sync_position(void)
{
	itp_get_rt_position(mc_last_step_pos);
	kinematics_apply_forward(mc_last_step_pos, mc_last_target);
	parser_sync_position();
}
