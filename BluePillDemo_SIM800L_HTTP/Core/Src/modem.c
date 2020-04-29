#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32f1xx_hal.h"
#include "modem.h"
#include "buffered_serial.h"

static ModemStatus_t SendBasicCommandResponse(char *command, uint16_t timeout);
static ModemStatus_t SendBasicCommandTextResponse(char *command, char *response, uint8_t response_length, uint16_t timeout);
static ModemStatus_t GetEcho(char *command, uint16_t timeout);
static ModemStatus_t GetStandardResponse(uint16_t timeout);

ModemStatus_t ModemInit(void)
{
	uint8_t init_response[20];
	uint8_t tries = 0U;

	while (tries < 10U)
	{
		serial_write_data(6U, (uint8_t *)"ATE1\r\n");
		HAL_Delay(100U);

		serial_read_data(20U, init_response);
		if (memcmp(init_response, "ATE1\r\r\nOK\r\n", 16))
		{
			return MODEM_OK;
		}

		if (memcmp("OK\r\n", init_response, 4) == 0)
		{
			return MODEM_OK;
		}

		tries++;
		HAL_Delay(1000U);
	}

	return MODEM_NO_RESPONSE;
}

static ModemStatus_t GetEcho(char *command, uint16_t timeout)
{
	uint8_t length = strlen(command);
	uint8_t i = 0U;
	uint32_t startTime = HAL_GetTick();
	uint8_t byte;

	while (i < length)
	{
		if (serial_received_bytes_waiting() > 0U)
		{
			serial_read_data(1U, &byte);
			if (byte != command[i])
			{
				return MODEM_UNEXPECTED_RESPONSE;
			}

			i++;
		}
		else
		{
			if (HAL_GetTick() > startTime + timeout)
			{
				return MODEM_TIMEOUT;
			}
		}
	}

	while (true)
	{
		if (serial_received_bytes_waiting() > 0U)
		{
			serial_read_data(1U, &byte);
			if (byte != '\r' && byte != '\n')
			{
				return MODEM_UNEXPECTED_RESPONSE;
			}

			if (byte == '\n')
			{
				break;
			}
		}
		else
		{
			if (HAL_GetTick() > startTime + timeout)
			{
				return MODEM_TIMEOUT;
			}
		}
	}

	return MODEM_OK;
}

static ModemStatus_t GetStandardResponse(uint16_t timeout)
{
	uint8_t i = 0U;
	uint32_t startTime = HAL_GetTick();
	uint8_t response[20];

	while (true)
	{
		if (serial_received_bytes_waiting() > 0U)
		{
			serial_read_data(1U, &response[i]);
			if (response[i] == '\n')
			{
				if (memcmp(response, "\r\n", 2) == 0)
				{
					i = 0;
					continue;
				}
				else if (memcmp(response, "OK\r\n", 4) == 0)
				{
					return MODEM_OK;
				}
				else if (memcmp(response, "ERROR\r\n", 7) == 0)
				{
					return MODEM_ERROR;
				}
				else
				{
					return MODEM_UNEXPECTED_RESPONSE;
				}
			}
			i++;

			if (i == sizeof(response))
			{
				return MODEM_UNEXPECTED_RESPONSE;
			}
		}
		else
		{
			if (HAL_GetTick() > startTime + timeout)
			{
				return MODEM_TIMEOUT;
			}
		}
	}
}

static ModemStatus_t SendBasicCommandTextResponse(char *command, char *response, uint8_t response_length, uint16_t timeout)
{
	uint8_t length = strlen(command);
	uint8_t i = 0U;
	uint32_t startTime = HAL_GetTick();
	ModemStatus_t status;

	serial_write_data(length, (uint8_t *)command);
	serial_write_data(1U, (uint8_t *)"\r");

	status = GetEcho(command, timeout);
	if (status != MODEM_OK)
	{
		return status;
	}

	while (true)
	{
		if (serial_received_bytes_waiting() > 0U)
		{
			serial_read_data(1U, (uint8_t *)&response[i]);
			if (response[i] == '\n')
			{
				break;
			}
			i++;

			if (i == response_length)
			{
				return MODEM_OVERFLOW;
			}
		}
		else
		{
			if (HAL_GetTick() > startTime + timeout)
			{
				return MODEM_TIMEOUT;
			}
		}
	}

	return GetStandardResponse(timeout);
}

static ModemStatus_t SendBasicCommandResponse(char *command, uint16_t timeout)
{
	uint8_t length = strlen(command);
	ModemStatus_t status;

	serial_write_data(length, (uint8_t *)command);
	serial_write_data(1U, (uint8_t *)"\r");

	status = GetEcho(command, timeout);
	if (status != MODEM_OK)
	{
		return status;
	}

	return GetStandardResponse(timeout);
}

ModemStatus_t ModemHello(void)
{
	return SendBasicCommandResponse("AT", 1000U);
}

ModemStatus_t ModemGetSignalStrength(uint8_t *strength)
{
	ModemStatus_t status;
	char response[20];

	*strength = 0U;
	status = SendBasicCommandTextResponse("AT+CSQ", response, sizeof(response), 1000U);
	if (status == MODEM_OK)
	{
		if (sscanf(response, "+CSQ: %hhu", strength) != 1)
		{
			status = MODEM_UNEXPECTED_RESPONSE;
		}
	}

	return status;
}

ModemStatus_t ModemGetNetworkRegistered(bool *registered)
{
	ModemStatus_t status;
	char response[20];
	uint8_t registeredNumber;

	*registered = false;
	status = SendBasicCommandTextResponse("AT+CREG?", response, sizeof(response), 1000U);
	if (status == MODEM_OK)
	{
		if (sscanf(response, "+CREG: 0,%hhu", &registeredNumber) != 1)
		{
			status = MODEM_UNEXPECTED_RESPONSE;
		}

		if (registeredNumber == 1U)
		{
			*registered = true;
		}
	}

	return status;
}

ModemStatus_t ModemHttpInit(char *apn, char *username, char *password)
{
	ModemStatus_t status;
	char atCommand[100];

	// set connection type to gprs
	status = SendBasicCommandResponse("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"", 1000U);
	if (status != MODEM_OK)
	{
		return status;
	}

	// send apn
	strcpy(atCommand, "AT+SAPBR=3,1,\"APN\",\"");
	strcat(atCommand, apn);
	strcat(atCommand,"\"");
	status = SendBasicCommandResponse(atCommand, 1000U);
	if (status != MODEM_OK)
	{
		return status;
	}

	// send user name
	strcpy(atCommand, "AT+SAPBR=3,1,\"USER\",\"");
	strcat(atCommand, username);
	strcat(atCommand,"\"");
	status = SendBasicCommandResponse(atCommand, 1000U);
	if (status != MODEM_OK)
	{
		return status;
	}

	// send password
	strcpy(atCommand, "AT+SAPBR=3,1,\"PWD\",\"");
	strcat(atCommand, password);
	strcat(atCommand,"\"");
	status = SendBasicCommandResponse(atCommand, 1000U);
	if (status != MODEM_OK)
	{
		return status;
	}

	// open bearer
	status = SendBasicCommandResponse("AT+SAPBR=1,1", 5000U);
	if (status != MODEM_OK)
	{
		return status;
	}

	// init http service
	return SendBasicCommandResponse("AT+HTTPINIT", 1000U);
}

ModemStatus_t ModemHttpClose(void)
{
	return SendBasicCommandResponse("AT+SAPBR=0,1", 5000U);
}

ModemStatus_t ModemHttpSendURL(char *url)
{
	char atCommand[256];

	if (strlen(url) > 230)
	{
		return MODEM_OVERFLOW;
	}

	strcpy(atCommand, "AT+HTTPPARA=\"URL\",\"");
	strcat(atCommand, url);
	strcat(atCommand,"\"");
	return SendBasicCommandResponse(atCommand, 1000U);
}

ModemStatus_t ModemHttpSendAction(HttpAction_t httpAction, uint16_t *httpResponseCode, uint16_t *responseLength)
{
	char atCommand[30];
	char atResponse[30];
	uint8_t i;
	uint32_t startTime = HAL_GetTick();
	ModemStatus_t status;

	strcpy(atCommand, "AT+HTTPACTION=");
	itoa(httpAction, atCommand + 14, 10);
	status = SendBasicCommandResponse(atCommand, 1000U);
	if (status != MODEM_OK)
	{
		return status;
	}

	// get blank line
	i = 0U;
	while (true)
	{
		if (serial_received_bytes_waiting() > 0U)
		{
			serial_read_data(1U, (uint8_t *)&atResponse[i]);
			if (atResponse[i] == '\n')
			{
				break;
			}
			i++;
			if (i > 30U)
			{
				return MODEM_UNEXPECTED_RESPONSE;
			}
		}
		else
		{
			if (HAL_GetTick() > startTime + 10000U)
			{
				return MODEM_TIMEOUT;
			}
		}
	}

	// get response
	i = 0U;
	while (true)
	{
		if (serial_received_bytes_waiting() > 0U)
		{
			serial_read_data(1U, (uint8_t *)&atResponse[i]);
			if (atResponse[i] == '\n')
			{
				break;
			}
			i++;
			if (i > 30U)
			{
				return MODEM_UNEXPECTED_RESPONSE;
			}
		}
		else
		{
			if (HAL_GetTick() > startTime + 10000U)
			{
				return MODEM_TIMEOUT;
			}
		}
	}

	if (sscanf(atResponse, "+HTTPACTION: 0,%hu,%hu", httpResponseCode, responseLength) != 2)
	{
		return MODEM_UNEXPECTED_RESPONSE;
	}

	return MODEM_OK;
}

ModemStatus_t ModemHttpReadResponse(uint16_t startPosition, uint16_t length, uint8_t *responseBuffer)
{
	char atCommand[30];
	char atResponse[30];
	ModemStatus_t status;
	uint32_t startTime = HAL_GetTick();
	uint8_t i;
	uint16_t bytesRead = 0U;

	strcpy(atCommand, "AT+HTTPREAD=");
	itoa(startPosition, atCommand + strlen(atCommand), 10);
	strcat(atCommand, ",");
	itoa(length, atCommand + strlen(atCommand), 10);

	serial_write_data(strlen(atCommand), (uint8_t *)atCommand);
	serial_write_data(1U, (uint8_t *)"\r");

	status = GetEcho(atCommand, 100U);
	if (status != MODEM_OK)
	{
		return status;
	}

	// get bytes read response
	i = 0U;
	while (true)
	{
		if (serial_received_bytes_waiting() > 0U)
		{
			serial_read_data(1U, (uint8_t *)&atResponse[i]);
			if (atResponse[i] == '\n')
			{
				break;
			}
			i++;
			if (i > 30U)
			{
				return MODEM_UNEXPECTED_RESPONSE;
			}
		}
		else
		{
			if (HAL_GetTick() > startTime + 10000U)
			{
				return MODEM_TIMEOUT;
			}
		}
	}

	// check bytes read response
	if (sscanf(atResponse, "+HTTPREAD: %hu", &bytesRead) != 1)
	{
		return MODEM_UNEXPECTED_RESPONSE;
	}
	if (bytesRead != length)
	{
		return MODEM_UNEXPECTED_RESPONSE;
	}

	// get http response
	bytesRead = 0U;
	while (true)
	{
		bytesRead += serial_read_data(length - bytesRead, responseBuffer + bytesRead);
		if (bytesRead == length)
		{
			break;
		}

		if (HAL_GetTick() > startTime + 10000U)
		{
			return MODEM_TIMEOUT;
		}
	}

	return GetStandardResponse(100U);
}

ModemStatus_t ModemDisableNewSMSNotification(void)
{
	return SendBasicCommandResponse("AT+CNMI=0", 1000U);
}
