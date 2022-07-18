/*
Name: esp8266_wifi.cpp
Description: Contains all Arduino ESP8266 C++ to C functions used by WiFi in µCNC.

Copyright: Copyright (c) João Martins
Author: João Martins
Date: 24-06-2022

µCNC is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version. Please see <http://www.gnu.org/licenses/>

µCNC is distributed WITHOUT ANY WARRANTY;
Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the	GNU General Public License for more details.
*/

#include "../../../../cnc_config.h"
#ifdef ESP8266
#ifdef ENABLE_WIFI
#include <Arduino.h>
#include "../../../../../webui/esp3d/esp3d.h"
#include "../../../../../webui/esp3d/espcom.h"
Esp3D webuiserver;

extern "C"
{
	bool esp8266_wifi_clientok(void)
	{
		return ESPCOM::hasClients();
	}

	void esp8266_wifi_init(int baud)
	{
		webuiserver.begin();

		// memset(wifi_rx_buffer.buffer, 0, WIFI_BUFFER_SIZE);
		// wifi_rx_buffer.len = 0;
		// wifi_rx_buffer.current = 0;

		// memset(wifi_tx_buffer.buffer, 0, WIFI_BUFFER_SIZE);
		// wifi_tx_buffer.len = 0;
		// wifi_tx_buffer.current = 0;
	}

	// char esp8266_wifi_read(void)
	// {
	// 	if (wifi_rx_buffer.len != 0 && wifi_rx_buffer.len > wifi_rx_buffer.current)
	// 	{
	// 		return wifi_rx_buffer.buffer[wifi_rx_buffer.current++];
	// 	}

	// 	if (esp8266_wifi_clientok())
	// 	{
	// 		size_t rxlen = serverClient.available();
	// 		if (rxlen > 0)
	// 		{
	// 			serverClient.readBytes(wifi_rx_buffer.buffer, rxlen);
	// 			wifi_rx_buffer.len = rxlen;
	// 			wifi_rx_buffer.current = 1;
	// 			return wifi_rx_buffer.buffer[0];
	// 		}
	// 	}

	// 	return 0;
	// }

	// void esp8266_wifi_write(char c)
	// {
	// 	if (esp8266_wifi_clientok())
	// 	{
	// 		wifi_tx_buffer.buffer[wifi_tx_buffer.len] = c;
	// 		wifi_tx_buffer.len++;
	// 		if (c == '\n')
	// 		{
	// 			serverClient.write(wifi_tx_buffer.buffer, (size_t)wifi_tx_buffer.len);
	// 			memset(wifi_tx_buffer.buffer, 0, WIFI_BUFFER_SIZE);
	// 			wifi_tx_buffer.len = 0;
	// 		}
	// 	}
	// }

	// bool esp8266_wifi_rx_ready(void)
	// {
	// 	if (wifi_rx_buffer.len != 0 && wifi_rx_buffer.len > wifi_rx_buffer.current)
	// 	{
	// 		return true;
	// 	}

	// 	if (esp8266_wifi_clientok())
	// 	{
	// 		return (serverClient.available() != 0);
	// 	}

	// 	return false;
	// }

	// bool esp8266_wifi_tx_ready(void)
	// {
	// 	if (esp8266_wifi_clientok())
	// 	{
	// 		if (wifi_tx_buffer.len < WIFI_BUFFER_SIZE)
	// 		{
	// 			return true;
	// 		}
	// 	}

	// 	return false;
	// }

	void esp8266_wifi_update(void)
	{
		webuiserver.process();
	}
}

#endif
#endif
