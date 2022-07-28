/*
	Name: esp8266_uart.cpp
	Description: Contains all Arduino ESP8266 C++ to C functions used by UART in µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 27-07-2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../../../cnc_config.h"
#include "../mcu.h"
#ifdef ESP8266
#include <Arduino.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef ENABLE_WIFI
#include "../../../../../webui/esp3d/esp3d.h"
#include "../../../../../webui/esp3d/espcom.h"
Esp3D webuiserver;
#endif

#ifndef ESP8266_BUFFER_SIZE
#define ESP8266_BUFFER_SIZE 255
#endif

static char esp8266_tx_buffer[ESP8266_BUFFER_SIZE];
static uint8_t esp8266_tx_buffer_counter;

extern "C"
{
#ifdef ENABLE_WIFI
	bool esp8266_wifi_clientok(void)
	{
		return ESPCOM::hasClients();
	}

	// device 2 PC
	long esp_serial_readbytes(uint8_t *buffer, size_t len)
	{
		uint8_t length = esp8266_tx_buffer_counter;
		memcpy(buffer, esp8266_tx_buffer, esp8266_tx_buffer_counter);

		return Serial.readBytes(buffer, len);
	}
	// baudrate
	long esp_serial_baudrate(void)
	{
		return BAUDRATE;
	}
	// device 2 PC available
	int esp_serial_available(void)
	{
		return esp8266_tx_buffer_counter;
	}
	// PC 2 device flush
	void esp_com_flush(void)
	{
		// nothing to be done
	}
	// PC 2 device
	size_t esp_serial_write(uint8_t d)
	{
		mcu_com_rx_cb((unsigned char)d);
		return d;
	}
	// PC 2 device
	void esp_serial_print(const char *data)
	{
		while (*data)
		{
			mcu_com_rx_cb((unsigned char)*data);
			data++;
		}
	}
#endif

	void esp8266_uart_init(int baud)
	{
		Serial.begin(baud);
#ifdef ENABLE_WIFI
		webuiserver.begin();
#endif
		esp8266_tx_buffer_counter = 0;
	}

	void esp8266_uart_flush(void)
	{
		Serial.println(esp8266_tx_buffer);
		Serial.flush();
#ifdef ENABLE_WIFI
		if (ESPCOM::hasClients())
		{
			ESPCOM::println(esp8266_tx_buffer, DEFAULT_PRINTER_PIPE);
		}
#endif
		esp8266_tx_buffer_counter = 0;
	}

	unsigned char esp8266_uart_read(void)
	{
		return (unsigned char)Serial.read();
	}

	void esp8266_uart_write(char c)
	{
		switch (c)
		{
		case '\n':
		case '\r':
			if (esp8266_tx_buffer_counter)
			{
				esp8266_tx_buffer[esp8266_tx_buffer_counter] = 0;
				esp8266_uart_flush();
			}
			break;
		default:
			if (esp8266_tx_buffer_counter >= (ESP8266_BUFFER_SIZE - 1))
			{
				esp8266_tx_buffer[esp8266_tx_buffer_counter] = 0;
				esp8266_uart_flush();
			}

			esp8266_tx_buffer[esp8266_tx_buffer_counter++] = c;
			break;
		}
	}

	bool esp8266_uart_rx_ready(void)
	{
		bool wifiready = false;
#ifdef ENABLE_WIFI
		if (ESPCOM::hasClients())
		{
			wifiready = (ESPCOM::writeAvailable() > 0);
		}
#endif
		return ((Serial.available() > 0) || wifiready);
	}

	bool esp8266_uart_tx_ready(void)
	{
		return (esp8266_tx_buffer_counter != ESP8266_BUFFER_SIZE);
	}

	void esp8266_uart_process(void)
	{
		while (Serial.available() > 0)
		{
			system_soft_wdt_feed();
			mcu_com_rx_cb((unsigned char)Serial.read());
		}
#ifdef ENABLE_WIFI
		webuiserver.process();
#endif
	}
}

#endif
