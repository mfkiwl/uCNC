/*
	Name: digimstep.c
	Description: Digital pin stepper settings module for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 31-03-2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../cnc.h"

#ifdef ENABLE_DIGITAL_MSTEP

/*custom gcode commands*/
#if defined(ENABLE_PARSER_MODULES)
// this ID must be unique for each code
#define M351 EXTENDED_MCODE(351)

uint8_t m351_parse(void *args, bool* handled);
uint8_t m351_exec(void *args, bool* handled);

CREATE_EVENT_LISTENER(gcode_parse, m351_parse);
CREATE_EVENT_LISTENER(gcode_exec, m351_exec);

// this just parses and acceps the code
uint8_t m351_parse(void *args, bool* handled)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;

	if (ptr->word == 'M' && ptr->value == 351.0f)
	{
		*handled = true;

		if (ptr->cmd->group_extended != 0)
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			return STATUS_GCODE_MODAL_GROUP_VIOLATION;
		}
		// tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
		ptr->cmd->group_extended = M351;
		return STATUS_OK;
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return ptr->error;
}

// this actually performs 2 steps in 1 (validation and execution)
uint8_t m351_exec(void *args, bool* handled)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;

	if (ptr->cmd->group_extended == M351)
	{
		*handled = true;

		itp_sync();
		if (!ptr->cmd->words)
		{
			int32_t val = -1;
			// if no additional args then print the
			protocol_send_string(__romstr__("[MSTEPS:"));
			val = -1;
			serial_putc('X');
#ifdef STEPPER0_MSTEP0
			val = mcu_get_output(STEPPER0_MSTEP0) ? 1 : 0;
#endif
#ifdef STEPPER0_MSTEP1
			val = MAX(0, val);
			val |= mcu_get_output(STEPPER0_MSTEP1) ? 2 : 0;
#endif
			serial_print_flt(val);
			serial_putc(',');
			val = -1;
			serial_putc('Y');
#ifdef STEPPER1_MSTEP0
			val = mcu_get_output(STEPPER1_MSTEP0) ? 1 : 0;
#endif
#ifdef STEPPER1_MSTEP1
			val = MAX(0, val);
			val |= mcu_get_output(STEPPER1_MSTEP1) ? 2 : 0;
#endif
			serial_print_flt(val);
			serial_putc(',');
			val = -1;
			serial_putc('Z');
#ifdef STEPPER2_MSTEP0
			val = mcu_get_output(STEPPER2_MSTEP0) ? 1 : 0;
#endif
#ifdef STEPPER2_MSTEP1
			val = MAX(0, val);
			val |= mcu_get_output(STEPPER2_MSTEP1) ? 2 : 0;
#endif
			serial_print_flt(val);
			serial_putc(',');
			val = -1;
			serial_putc('A');
#ifdef STEPPER3_MSTEP0
			val = mcu_get_output(STEPPER3_MSTEP0) ? 1 : 0;
#endif
#ifdef STEPPER3_MSTEP1
			val = MAX(0, val);
			val |= mcu_get_output(STEPPER3_MSTEP1) ? 2 : 0;
#endif
			serial_print_flt(val);
			serial_putc(',');
			val = -1;
			serial_putc('B');
#ifdef STEPPER4_MSTEP0
			val = mcu_get_output(STEPPER4_MSTEP0) ? 1 : 0;
#endif
#ifdef STEPPER4_MSTEP1
			val = MAX(0, val);
			val |= mcu_get_output(STEPPER4_MSTEP1) ? 2 : 0;
#endif
			serial_print_flt(val);
			serial_putc(',');
			val = -1;
			serial_putc('C');
#ifdef STEPPER5_MSTEP0
			val = mcu_get_output(STEPPER5_MSTEP0) ? 1 : 0;
#endif
#ifdef STEPPER5_MSTEP1
			val = MAX(0, val);
			val |= mcu_get_output(STEPPER5_MSTEP1) ? 2 : 0;
#endif
			serial_print_flt(val);
			serial_putc(',');
			val = -1;
			serial_putc('I');
#ifdef STEPPER6_MSTEP0
			val = mcu_get_output(STEPPER6_MSTEP0) ? 1 : 0;
#endif
#ifdef STEPPER6_MSTEP1
			val = MAX(0, val);
			val |= mcu_get_output(STEPPER6_MSTEP1) ? 2 : 0;
#endif
			serial_print_flt(val);
			serial_putc(',');
			val = -1;
			serial_putc('J');
#ifdef STEPPER7_MSTEP0
			val = mcu_get_output(STEPPER7_MSTEP0) ? 1 : 0;
#endif
#ifdef STEPPER7_MSTEP1
			val = MAX(0, val);
			val |= mcu_get_output(STEPPER7_MSTEP1) ? 2 : 0;
#endif
			serial_print_flt(val);
			serial_putc(']');
			protocol_send_string(MSG_EOL);
		}

		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_X))
		{
#ifdef STEPPER0_MSTEP0
			io_set_output(STEPPER0_MSTEP0, ((uint8_t)ptr->words->xyzabc[0] & 0x01));
#endif
#ifdef STEPPER0_MSTEP1
			io_set_output(STEPPER0_MSTEP1, ((uint8_t)ptr->words->xyzabc[0] & 0x02));
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_Y))
		{
#ifdef STEPPER1_MSTEP0
			io_set_output(STEPPER1_MSTEP0, ((uint8_t)ptr->words->xyzabc[1] & 0x01));
#endif
#ifdef STEPPER1_MSTEP1
			io_set_output(STEPPER1_MSTEP1, ((uint8_t)ptr->words->xyzabc[1] & 0x02));
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_Z))
		{
#ifdef STEPPER2_MSTEP0
			io_set_output(STEPPER2_MSTEP0, (((uint8_t)ptr->words->xyzabc[2]) & 0x01));
#endif
#ifdef STEPPER2_MSTEP1
			io_set_output(STEPPER2_MSTEP1, (((uint8_t)ptr->words->xyzabc[2]) & 0x02));
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_A))
		{
#ifdef STEPPER3_MSTEP0
			io_set_output(STEPPER3_MSTEP0, ((uint8_t)ptr->words->xyzabc[3] & 0x01));
#endif
#ifdef STEPPER3_MSTEP1
			io_set_output(STEPPER3_MSTEP1, ((uint8_t)ptr->words->xyzabc[3] & 0x02));
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_B))
		{
#ifdef STEPPER4_MSTEP0
			io_set_output(STEPPER4_MSTEP0, ((uint8_t)ptr->words->xyzabc[4] & 0x01));
#endif
#ifdef STEPPER4_MSTEP1
			io_set_output(STEPPER4_MSTEP1, ((uint8_t)ptr->words->xyzabc[4] & 0x02));
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_C))
		{
#ifdef STEPPER5_MSTEP0
			io_set_output(STEPPER5_MSTEP0, ((uint8_t)ptr->words->xyzabc[5] & 0x01));
#endif
#ifdef STEPPER5_MSTEP1
			io_set_output(STEPPER5_MSTEP1, ((uint8_t)ptr->words->xyzabc[5] & 0x02));
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_I))
		{
#ifdef STEPPER6_MSTEP0
			io_set_output(STEPPER6_MSTEP0, ((uint8_t)ptr->words->ijk[0] & 0x01));
#endif
#ifdef STEPPER6_MSTEP1
			io_set_output(STEPPER6_MSTEP1, ((uint8_t)ptr->words->ijk[0] & 0x02));
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_J))
		{
#ifdef STEPPER7_MSTEP0
			io_set_output(STEPPER7_MSTEP0, ((uint8_t)ptr->words->ijk[1] & 0x01));
#endif
#ifdef STEPPER7_MSTEP1
			io_set_output(STEPPER7_MSTEP1, ((uint8_t)ptr->words->ijk[1] & 0x02));
#endif
		}

		return STATUS_OK;
	}

	return STATUS_GCODE_EXTENDED_UNSUPPORTED;
}

#endif

DECL_MODULE(digimstep)
{
#ifdef STEPPER0_MSTEP0
	io_set_output(STEPPER0_MSTEP0, (STEPPER0_MSTEP & 0x01));
#endif
#ifdef STEPPER0_MSTEP1
	io_set_output(STEPPER0_MSTEP1, (STEPPER0_MSTEP & 0x02));
#endif
#ifdef STEPPER1_MSTEP0
	io_set_output(STEPPER1_MSTEP0, (STEPPER1_MSTEP & 0x01));
#endif
#ifdef STEPPER1_MSTEP1
	io_set_output(STEPPER1_MSTEP1, (STEPPER1_MSTEP & 0x02));
#endif
#ifdef STEPPER2_MSTEP0
	io_set_output(STEPPER2_MSTEP0, (STEPPER2_MSTEP & 0x01));
#endif
#ifdef STEPPER2_MSTEP1
	io_set_output(STEPPER2_MSTEP1, (STEPPER2_MSTEP & 0x02));
#endif
#ifdef STEPPER3_MSTEP0
	io_set_output(STEPPER3_MSTEP0, (STEPPER3_MSTEP & 0x01));
#endif
#ifdef STEPPER3_MSTEP1
	io_set_output(STEPPER3_MSTEP1, (STEPPER3_MSTEP & 0x02));
#endif
#ifdef STEPPER4_MSTEP0
	io_set_output(STEPPER4_MSTEP0, (STEPPER4_MSTEP & 0x01));
#endif
#ifdef STEPPER4_MSTEP1
	io_set_output(STEPPER4_MSTEP1, (STEPPER4_MSTEP & 0x02));
#endif
#ifdef STEPPER5_MSTEP0
	io_set_output(STEPPER5_MSTEP0, (STEPPER5_MSTEP & 0x01));
#endif
#ifdef STEPPER5_MSTEP1
	io_set_output(STEPPER5_MSTEP1, (STEPPER5_MSTEP & 0x02));
#endif
#ifdef STEPPER6_MSTEP0
	io_set_output(STEPPER6_MSTEP0, (STEPPER6_MSTEP & 0x01));
#endif
#ifdef STEPPER6_MSTEP1
	io_set_output(STEPPER6_MSTEP1, (STEPPER6_MSTEP & 0x02));
#endif
#ifdef STEPPER7_MSTEP0
	io_set_output(STEPPER7_MSTEP0, (STEPPER7_MSTEP & 0x01));
#endif
#ifdef STEPPER7_MSTEP1
	io_set_output(STEPPER7_MSTEP1, (STEPPER7_MSTEP & 0x02));
#endif

#ifdef ENABLE_PARSER_MODULES
	ADD_EVENT_LISTENER(gcode_parse, m351_parse);
	ADD_EVENT_LISTENER(gcode_exec, m351_exec);
#endif
}
#endif
