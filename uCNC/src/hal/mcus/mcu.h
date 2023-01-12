/*
	Name: mcu.h
	Description: Contains all the function declarations necessary to interact with the MCU.
		This provides an intenterface between the µCNC and the MCU unit used to power the µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 01/11/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef MCU_H
#define MCU_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef MCU_CALLBACK
#define MCU_CALLBACK
#endif

#ifndef MCU_TX_CALLBACK
#define MCU_TX_CALLBACK MCU_CALLBACK
#endif

#ifndef MCU_RX_CALLBACK
#define MCU_RX_CALLBACK MCU_CALLBACK
#endif

#ifndef MCU_IO_CALLBACK
#define MCU_IO_CALLBACK MCU_CALLBACK
#endif

// defines special mcu to access flash strings and arrays
#ifndef __rom__
#define __rom__
#endif
#ifndef __romstr__
#define __romstr__
#endif
#ifndef rom_ptr
#define rom_ptr *
#endif
#ifndef rom_strcpy
#define rom_strcpy strcpy
#endif
#ifndef rom_strncpy
#define rom_strncpy strncpy
#endif
#ifndef rom_memcpy
#define rom_memcpy memcpy
#endif

	// the extern is not necessary
	// this explicit declaration just serves to reeinforce the idea that these callbacks are implemented on other µCNC core code translation units
	// these callbacks provide a transparent way for the mcu to call them when the ISR/IRQ is triggered

	MCU_CALLBACK void mcu_step_cb(void);
	MCU_CALLBACK void mcu_step_reset_cb(void);
	MCU_RX_CALLBACK void mcu_com_rx_cb(unsigned char c);
	MCU_TX_CALLBACK void mcu_com_tx_cb();
	MCU_CALLBACK void mcu_rtc_cb(uint32_t millis);
	MCU_IO_CALLBACK void mcu_controls_changed_cb(void);
	MCU_IO_CALLBACK void mcu_limits_changed_cb(void);
	MCU_IO_CALLBACK void mcu_probe_changed_cb(void);
	MCU_IO_CALLBACK void mcu_inputs_changed_cb(void);

/*IO functions*/

/**
 * config a pin in input mode
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_config_input
	void mcu_config_input(uint8_t pin);
#endif

/**
 * config a pin in output mode
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_config_output
	void mcu_config_output(uint8_t pin);
#endif

/**
 * get the value of a digital input pin
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_get_input
	uint8_t mcu_get_input(uint8_t pin);
#endif

/**
 * gets the value of a digital output pin
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_get_output
	uint8_t mcu_get_output(uint8_t pin);
#endif

/**
 * sets the value of a digital output pin to logical 1
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_set_output
	void mcu_set_output(uint8_t pin);
#endif

/**
 * sets the value of a digital output pin to logical 0
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_clear_output
	void mcu_clear_output(uint8_t pin);
#endif

/**
 * toggles the value of a digital output pin
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_toggle_output
	void mcu_toggle_output(uint8_t pin);
#endif

	/**
	 *
	 * This is used has by the generic mcu functions has generic (overridable) IO initializer
	 *
	 * */
	void mcu_io_init(void);

	/**
	 * initializes the mcu
	 * this function needs to:
	 *   - configure all IO pins (digital IO, PWM, Analog, etc...)
	 *   - configure all interrupts
	 *   - configure uart or usb
	 *   - start the internal RTC
	 * */
	void mcu_init(void);

/**
 * enables the pin probe mcu isr on change
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_enable_probe_isr
	void mcu_enable_probe_isr(void);
#endif

/**
 * disables the pin probe mcu isr on change
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_disable_probe_isr
	void mcu_disable_probe_isr(void);
#endif

/**
 * gets the voltage value of a built-in ADC pin
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_get_analog
	uint8_t mcu_get_analog(uint8_t channel);
#endif

/**
 * configs the pwm pin and output frequency
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_config_pwm
	void mcu_config_pwm(uint8_t pin, uint16_t freq);
#endif

/**
 * sets the pwm value of a built-in pwm pin
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_set_pwm
	void mcu_set_pwm(uint8_t pwm, uint8_t value);
#endif

/**
 * gets the configured pwm value of a built-in pwm pin
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_get_pwm
	uint8_t mcu_get_pwm(uint8_t pwm);
#endif

/**
 * sets the pwm for a servo (50Hz with tON between 1~2ms)
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_set_servo
	void mcu_set_servo(uint8_t servo, uint8_t value);
#endif

/**
 * gets the pwm for a servo (50Hz with tON between 1~2ms)
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_get_servo
	uint8_t mcu_get_servo(uint8_t servo);
#endif

/**
 * checks if the serial hardware of the MCU is ready do send the next char
 * */
#ifndef mcu_tx_ready
	bool mcu_tx_ready(void); // Start async send
#endif

/**
 * sends a char either via uart (hardware, software or USB virtual COM port)
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_putc
	void mcu_putc(char c);
#endif

// ISR
/**
 * enables global interrupts on the MCU
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_enable_global_isr
	void mcu_enable_global_isr(void);
#endif

/**
 * disables global interrupts on the MCU
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_disable_global_isr
	void mcu_disable_global_isr(void);
#endif

/**
 * gets global interrupts state on the MCU
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_get_global_isr
	bool mcu_get_global_isr(void);
#endif

	// Step interpolator
	/**
	 * convert step rate/frequency to timer ticks and prescaller
	 * */
	void mcu_freq_to_clocks(float frequency, uint16_t *ticks, uint16_t *prescaller);

	/**
	 * convert timer ticks and prescaller to step rate/frequency
	 * */
	float mcu_clocks_to_freq(uint16_t ticks, uint16_t prescaller);

	/**
	 * starts the timer interrupt that generates the step pulses for the interpolator
	 * */
	void mcu_start_itp_isr(uint16_t ticks, uint16_t prescaller);

	/**
	 * changes the step rate of the timer interrupt that generates the step pulses for the interpolator
	 * */
	void mcu_change_itp_isr(uint16_t ticks, uint16_t prescaller);

	/**
	 * stops the timer interrupt that generates the step pulses for the interpolator
	 * */
	void mcu_stop_itp_isr(void);

	/**
	 * gets the MCU running time in milliseconds.
	 * the time counting is controled by the internal RTC
	 * */
	uint32_t mcu_millis(void);

	/**
	 * gets the MCU running time in microseconds.
	 * the time counting is controled by the internal RTC
	 * */
	uint32_t mcu_micros(void);

#ifndef mcu_nop
#define mcu_nop() asm volatile("nop\n\t")
#endif

	void mcu_delay_loop(uint16_t loops);

#ifndef mcu_delay_cycles
// set per MCU
#ifndef MCU_CLOCKS_PER_CYCLE
#error "MCU_CLOCKS_PER_CYCLE not defined for this MCU"
#endif
#ifndef MCU_CYCLES_PER_LOOP_OVERHEAD
#error "MCU_CYCLES_PER_LOOP_OVERHEAD not defined for this MCU"
#endif
#ifndef MCU_CYCLES_PER_LOOP
#error "MCU_CYCLES_PER_LOOP not defined for this MCU"
#endif
#ifndef MCU_CYCLES_PER_LOOP_OVERHEAD
#error "MCU_CYCLES_PER_LOOP_OVERHEAD not defined for this MCU"
#endif

#define mcu_delay_cycles(X)                                                                                                                     \
	{                                                                                                                                           \
		if (X > (MCU_CYCLES_PER_LOOP + MCU_CYCLES_PER_LOOP_OVERHEAD))                                                                           \
		{                                                                                                                                       \
			mcu_delay_loop((uint16_t)((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP));                                               \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 0)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                      \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 1)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                      \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 2)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                      \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 3)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                      \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 4)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                      \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 5)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                      \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 6)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                      \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 7)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                      \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 8)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                      \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 9)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                      \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 10) \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                      \
			}                                                                                                                                   \
		}                                                                                                                                       \
		else                                                                                                                                    \
		{                                                                                                                                       \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 0)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                      \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 1)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                      \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 2)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                      \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 3)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                      \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 4)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                      \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 5)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                      \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 6)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                      \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 7)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                      \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 8)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                      \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 9)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                      \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 10)                                                                   \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                      \
			}                                                                                                                                   \
		}                                                                                                                                       \
	}
#endif

#ifndef mcu_delay_100ns
#define mcu_delay_100ns() mcu_delay_cycles((F_CPU / MCU_CLOCKS_PER_CYCLE / 10000000UL))
#endif

/**
 * provides a delay in us (micro seconds)
 * the maximum allowed delay is 255 us
 * */
#ifndef mcu_delay_us
#define mcu_delay_us(X) mcu_delay_cycles(F_CPU / MCU_CLOCKS_PER_CYCLE / 1000000UL * X)
#endif

#ifdef MCU_HAS_ONESHOT_TIMER
	typedef void (*mcu_timeout_delgate)(void);
	extern MCU_CALLBACK mcu_timeout_delgate mcu_timeout_cb;
/**
 * configures a single shot timeout in us
 * */
#ifndef mcu_config_timeout
	void mcu_config_timeout(mcu_timeout_delgate fp, uint32_t timeout);
#endif

/**
 * starts the timeout. Once hit the the respective callback is called
 * */
#ifndef mcu_start_timeout
	void mcu_start_timeout();
#endif
#endif

	/**
	 * runs all internal tasks of the MCU.
	 * for the moment these are:
	 *   - if USB is enabled and MCU uses tinyUSB framework run tinyUSB tud_task
	 * */
	void mcu_dotasks(void);

	// Non volatile memory
	/**
	 * gets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
	 * */
	uint8_t mcu_eeprom_getc(uint16_t address);

	/**
	 * sets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
	 * */
	void mcu_eeprom_putc(uint16_t address, uint8_t value);

	/**
	 * flushes all recorded registers into the eeprom.
	 * */
	void mcu_eeprom_flush(void);

#ifdef MCU_HAS_SPI
#ifndef mcu_spi_xmit
	uint8_t mcu_spi_xmit(uint8_t data);
#endif

#ifndef mcu_spi_config
	void mcu_spi_config(uint8_t mode, uint32_t frequency);
#endif
#endif

#ifdef MCU_HAS_I2C
#ifndef mcu_i2c_write
	uint8_t mcu_i2c_write(uint8_t data, bool send_start, bool send_stop);
#endif

#ifndef mcu_i2c_read
	uint8_t mcu_i2c_read(bool with_ack, bool send_stop);
#endif
#endif

#ifdef BOARD_HAS_CUSTOM_SYSTEM_COMMANDS
	uint8_t mcu_custom_grbl_cmd(char* grbl_cmd_str, uint8_t grbl_cmd_len, char next_char);
#endif

#ifdef __cplusplus
}
#endif

#endif
