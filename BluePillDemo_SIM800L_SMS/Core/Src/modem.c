#include <string.h>
#include <stdio.h>
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
		if (memcmp(init_response, "ATE1\r\r\nOK\r\n", 11) == 0)
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

ModemStatus_t ModemSetSMSTextMode(void)
{
	return SendBasicCommandResponse("AT+CMGF=1", 1000U);
}

ModemStatus_t ModemDisableNewSMSNotification(void)
{
	return SendBasicCommandResponse("AT+CNMI=0", 1000U);
}

ModemStatus_t ModemDeleteAllReadAndSentSMS(void)
{
	return SendBasicCommandResponse("AT+CMGD=1,2", 1000U);
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

ModemStatus_t ModemGetNetworkRegistrationStatus(bool *registered)
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

ModemStatus_t ModemSendSMS(char *address, char *message)
{
	uint8_t i = 0U;
	uint32_t startTime = HAL_GetTick();
	ModemStatus_t status;
	char prompt;
	char newline;
	char response[20];
	char command[25];

	if (address == NULL || message == NULL || strlen(message) > 160)
	{
		return MODEM_BAD_PARAMETER;
	}

	strcpy(command, "AT+CMGS=\"");
	strcat(command, address);
	strcat(command, "\"\r");
	serial_write_data(strlen(command), (uint8_t *)command);

	status = GetEcho(command, 1000U);
	if (status != MODEM_OK)
	{
		return status;
	}

	while (true)
	{
		if (serial_received_bytes_waiting() > 0U)
		{
			serial_read_data(1U, (uint8_t *)&prompt);
			if (prompt == '>')
			{
				break;
			}
			else
			{
				return MODEM_UNEXPECTED_RESPONSE;
			}
		}
		else
		{
			if (HAL_GetTick() > startTime + 1000U)
			{
				return MODEM_TIMEOUT;
			}
		}
	}

	serial_write_data(strlen(message), (uint8_t *)message);
	serial_write_data(1U, (uint8_t *)"\x1a");

	while (true)
	{
		if (serial_received_bytes_waiting() > 0U)
		{
			serial_read_data(1U, (uint8_t *)&newline);
			if (newline == '\n')
			{
				break;
			}
		}
		else
		{
			if (HAL_GetTick() > startTime + 5000U)
			{
				return MODEM_TIMEOUT;
			}
		}
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

			if (i == 20U)
			{
				return MODEM_OVERFLOW;
			}
		}
		else
		{
			if (HAL_GetTick() > startTime + 5000U)
			{
				return MODEM_TIMEOUT;
			}
		}
	}

	return GetStandardResponse(1000U);
}

ModemStatus_t ModemListAllUnreadSMS(new_sms_callback_t new_sms_callback)
{
	char response[100];
	uint8_t i;
	ModemStatus_t status;
	SmsMessage_t smsMessage;
	uint32_t startTime = HAL_GetTick();

	serial_write_data(22U, (uint8_t *)"AT+CMGL=\"REC UNREAD\"\r\n");

	status = GetEcho("AT+CMGL=\"REC UNREAD\"", 1000U);
	if (status != MODEM_OK)
	{
		return status;
	}

	while (true)
	{
		i = 0U;
		while (true)
		{
			if (serial_received_bytes_waiting() > 0U)
			{
				serial_read_data(1U, (uint8_t *)&response[i]);
				if (response[i] == '\n')
				{
					if (memcmp(response, "OK\r\n", 4) == 0)
					{
						return MODEM_OK;
					}
					else if (memcmp(response, "ERROR\r\n", 7) == 0)
					{
						return MODEM_ERROR;
					}
					else if (memcmp(response, "\r\n", 2) == 0)
					{
						i = 0U;
						continue;
					}
					else
					{
						strtok(response, "\"");
						strtok(NULL, "\"");
						strtok(NULL, "\"");
						strcpy(smsMessage.from, strtok(NULL, "\""));
						strtok(NULL, "\"");
						strtok(NULL, "\"");
						strcpy(smsMessage.date, strtok(NULL, ","));
						strcpy(smsMessage.time, strtok(NULL, "\""));
						break;
					}
				}
				i++;
				if (i == 100)
				{
					return MODEM_OVERFLOW;
				}
			}
			else
			{
				if (HAL_GetTick() > startTime + 1000U)
				{
					return MODEM_TIMEOUT;
				}
			}
		}

		i = 0U;
		while (true)
		{
			if (serial_received_bytes_waiting() > 0U)
			{
				serial_read_data(1U, (uint8_t *)&smsMessage.text[i]);
				if (smsMessage.text[i] == '\n')
				{
					smsMessage.text[i] = '\0';
					new_sms_callback(&smsMessage);
					break;
				}
				i++;
				if (i == 160)
				{
					return MODEM_OVERFLOW;
				}
			}
			else
			{
				if (HAL_GetTick() > startTime + 1000U)
				{
					return MODEM_TIMEOUT;
				}
			}
		}
	}

	return MODEM_OK;
}
