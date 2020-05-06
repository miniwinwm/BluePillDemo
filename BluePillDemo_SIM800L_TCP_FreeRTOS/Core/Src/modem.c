#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "cmsis_os.h"
#include "stm32f1xx_hal.h"
#include "modem.h"
#include "buffered_serial.h"

// server side of client API for each AT command type
static void ServerModemHello(uint32_t timeoutMs);
static void ServerGetSignalStrength(uint32_t timeoutMs);
static void ServerNetworkRegistrationStatus(uint32_t timeoutMs);
static void ServerSetManualDataReceive(uint32_t timeoutMs);
static void ServerActivateDataConnection(uint32_t timeoutMs);
static void ServerConfigureDataConnection(uint32_t timeoutMs);
static void ServerDeactivateDataConnection(uint32_t timeoutMs);
static void ServerOpenTcpConnection(uint32_t timeoutMs);
static void ServerTcpWrite(uint32_t timeoutMs);
static void ServerCloseTcpConnection(uint32_t timeoutMs);
static void ServerGetOwnIpAddress(uint32_t timeoutMs);
static void ServerGetTcpReadDataWaitingLength(uint32_t timeoutMs);
static void ServerTcpRead(uint32_t timeoutMs);
static void ServerPowerDown(uint32_t timeoutMs);

// urc
static void ServerHandleURC(void);
static bool tcpConnectedState = false;

// utility functions
static ModemStatus_t ServerSendBasicCommandResponse(char *command, uint32_t timeoutMs);
static ModemStatus_t ServerSendBasicCommandTextResponse(char *command, char *response, uint8_t response_length, uint32_t timeoutMs);
static ModemStatus_t ServerGetEcho(char *command, uint32_t timeoutMs);
static ModemStatus_t ServerGetStandardResponse(uint32_t timeoutMs);
static ModemStatus_t ClientSendBasicCommandResponse(AtCommand_t atCommand, uint32_t timeoutMs);
static ModemStatus_t ClientTcpWriteSection(const uint8_t *data, uint8_t length, uint32_t timeoutMs);
static ModemStatus_t ClientTcpReadSection(uint8_t lengthToRead, uint8_t *lengthRead, uint8_t *buffer, uint32_t timeoutMs);
static void ServerFlushReadBufferOnError(ModemStatus_t modemStatus);

// server objects
static uint8_t echoOrUrc[MODEM_MAX_AT_COMMAND_SIZE];
static AtCommandPacket_t atCommandPacket;
static AtResponsePacket_t atResponsePacket;

// IDE generated FreeRTOS objects
extern osMessageQueueId_t commandQueueHandle;
extern osMessageQueueId_t responseQueueHandle;
extern osMutexId_t modemMutexHandle;

void ModemReset(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	osDelay(500UL);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
	osDelay(3000UL);
}

ModemStatus_t ModemInit(void)
{
	uint8_t initResponse[20];
	uint8_t tries = 0U;

	while (tries < 10U)
	{
		serial_write_data(6U, (uint8_t *)"ATE1\r\n");
		osDelay(100UL);

		serial_read_data(20U, initResponse);
		if (memcmp(initResponse, "ATE1\r\r\nOK\r\n", (size_t)11) == 0)
		{
			return MODEM_OK;
		}

		if (memcmp("OK\r\n", initResponse, (size_t)4) == 0)
		{
			return MODEM_OK;
		}

		tries++;
		osDelay(1000UL);
	}

	return MODEM_NO_RESPONSE;
}

void DoModemTask(void)
{
	uint8_t nextUnsolicitedResponsePos;
	uint32_t urcReceiveStartTimeMs;

	while (true)
	{
		osDelay(MODEM_SERVER_LOOP_PERIOD_MS);

		// check for out of sequence data arriving as this means a URC has arrived
		if (serial_received_bytes_waiting() > 0U && osMutexAcquire(modemMutexHandle, 0U) == osOK)
		{
			urcReceiveStartTimeMs = osKernelGetTickCount();
			nextUnsolicitedResponsePos = 0U;

			while (true)
			{
				// check for overflow
				if (nextUnsolicitedResponsePos == MODEM_MAX_URC_LENGTH)
				{
					nextUnsolicitedResponsePos = 0U;
				}

				// check for timeoutMs
				if (osKernelGetTickCount() > urcReceiveStartTimeMs + MODEM_URC_TIMEOUT_MS)
				{
					osMutexRelease(modemMutexHandle);
					break;
				}

				// check for more data waiting
				if (serial_received_bytes_waiting() > 0U)
				{
					serial_read_data(1U, &echoOrUrc[nextUnsolicitedResponsePos]);
					if (echoOrUrc[nextUnsolicitedResponsePos] == '\n')
					{
						// ignore \r\n blank lines
						if (memcmp(echoOrUrc, "\r\n", (size_t)2) != 0)
						{
							ServerHandleURC();
						}
						osMutexRelease(modemMutexHandle);
						break;
					}
					nextUnsolicitedResponsePos++;
				}
			}
		}

		if (osMessageQueueGet(commandQueueHandle, &atCommandPacket, 0U, 0UL) == osOK)
		{
			if (osMutexAcquire(modemMutexHandle, (uint32_t)atCommandPacket.timeoutMs) != osOK)
			{
				atResponsePacket.atResponse = MODEM_TIMEOUT;
				osMessageQueuePut(responseQueueHandle, &atResponsePacket, 0U, 0U);
			}
			else
			{
				switch (atCommandPacket.atCommand)
				{
					case MODEM_COMMAND_HELLO:
						ServerModemHello(atCommandPacket.timeoutMs);
						break;

					case MODEM_COMMAND_SIGNAL_STRENGTH:
						ServerGetSignalStrength(atCommandPacket.timeoutMs);
						break;

					case MODEM_COMMAND_NETWORK_REGISTRATION:
						ServerNetworkRegistrationStatus(atCommandPacket.timeoutMs);
						break;

					case MODEM_COMMAND_SET_MANUAL_DATA_READ:
						ServerSetManualDataReceive(atCommandPacket.timeoutMs);
						break;

					case MODEM_COMMAND_ACTIVATE_DATA_CONNECTION:
						ServerActivateDataConnection(atCommandPacket.timeoutMs);
						break;

					case MODEM_COMMAND_CONFIGURE_DATA_CONNECTION:
						ServerConfigureDataConnection(atCommandPacket.timeoutMs);
						break;

					case MODEM_COMMAND_DEACTIVATE_DATA_CONNECTION:
						ServerDeactivateDataConnection(atCommandPacket.timeoutMs);
						break;

					case MODEM_COMMAND_OPEN_TCP_CONNECTION:
						ServerOpenTcpConnection(atCommandPacket.timeoutMs);
						break;

					case MODEM_COMMAND_CLOSE_TCP_CONNECTION:
						ServerCloseTcpConnection(atCommandPacket.timeoutMs);
						break;

					case MODEM_COMMAND_GET_OWN_IP_ADDRESS:
						ServerGetOwnIpAddress(atCommandPacket.timeoutMs);
						break;

					case MODEM_COMMAND_TCP_WRITE:
						ServerTcpWrite(atCommandPacket.timeoutMs);
						break;

					case MODEM_COMMAND_GET_TCP_READ_DATA_WAITING_LENGTH:
						ServerGetTcpReadDataWaitingLength(atCommandPacket.timeoutMs);
						break;

					case MODEM_COMMAND_TCP_READ:
						ServerTcpRead(atCommandPacket.timeoutMs);
						break;

					case MODEM_COMMAND_POWER_DOWN:
						ServerPowerDown(atCommandPacket.timeoutMs);
						break;

					}
				osMutexRelease(modemMutexHandle);
			}
		}
	}
}

static void ServerHandleURC(void)
{
	if (memcmp(echoOrUrc, "CONNECT OK\r\n", (size_t)12) == 0)
	{
		tcpConnectedState = true;
	}
	else if (memcmp(echoOrUrc, "CLOSED\r\n", (size_t)8) == 0)
	{
		tcpConnectedState = false;
	}

	// add more URC handling here if needed
}

bool ModemGetTcpConnectedState(void)
{
	return tcpConnectedState;
}

static ModemStatus_t ServerSendBasicCommandResponse(char *command, uint32_t timeoutMs)
{
	uint8_t length = strlen(command);
	ModemStatus_t modemStatus;
	uint32_t startTime = osKernelGetTickCount();

	serial_write_data(length, (uint8_t *)command);
	serial_write_data(1U, (uint8_t *)"\r");

	modemStatus = ServerGetEcho(command, timeoutMs);
	if (modemStatus != MODEM_OK)
	{
		return modemStatus;
	}
	timeoutMs -= (osKernelGetTickCount() - startTime);

	return ServerGetStandardResponse(timeoutMs);
}

static ModemStatus_t ServerSendBasicCommandTextResponse(char *command, char *response, uint8_t response_length, uint32_t timeoutMs)
{
	uint8_t length = strlen(command);
	uint8_t i = 0U;
	uint32_t startTime = osKernelGetTickCount();
	ModemStatus_t modemStatus;

	serial_write_data(length, (uint8_t *)command);
	serial_write_data(1U, (uint8_t *)"\r");

	modemStatus = ServerGetEcho(command, timeoutMs);
	if (modemStatus != MODEM_OK)
	{
		return modemStatus;
	}
	timeoutMs -= (osKernelGetTickCount() - startTime);

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
			if (osKernelGetTickCount() > startTime + timeoutMs)
			{
				return MODEM_TIMEOUT;
			}
		}
	}
	timeoutMs -= (osKernelGetTickCount() - startTime);

	return ServerGetStandardResponse(timeoutMs);
}

static void ServerFlushReadBufferOnError(ModemStatus_t modemStatus)
{
	uint8_t byte;

	if (modemStatus >= 0)
	{
		// no error
		return;
	}

	// read bytes from buffer until buffer empty or a '\n' is received
	do
	{
		serial_read_data(1U, &byte);
	}
	while (byte != '\n' && serial_received_bytes_waiting() > 0U);
}

static ModemStatus_t ServerGetEcho(char *command, uint32_t timeoutMs)
{
	size_t length = strlen(command);
	uint8_t bytesRead = 0U;
	uint32_t startTime = osKernelGetTickCount();
	uint8_t byte;

	while (true)
	{
		if (serial_received_bytes_waiting() > 0U)
		{
			serial_read_data(1U, &echoOrUrc[bytesRead]);
			if (echoOrUrc[bytesRead] == '\n')
			{
				ServerHandleURC();
				bytesRead = 0U;
				continue;
			}

			if (bytesRead == length)
			{
				if (memcmp(command, echoOrUrc, length) == 0)
				{
					// echo received successfully
					break;
				}
			}

			bytesRead++;

			if (bytesRead == MODEM_MAX_AT_COMMAND_SIZE)
			{
				return MODEM_UNEXPECTED_RESPONSE;
			}
		}
		else
		{
			if (osKernelGetTickCount() > startTime + timeoutMs)
			{
				return MODEM_TIMEOUT;
			}
		}
	}

	// clean up trailing \r, \n
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
			if (osKernelGetTickCount() > startTime + timeoutMs)
			{
				return MODEM_TIMEOUT;
			}
		}
	}

	return MODEM_OK;
}

static ModemStatus_t ServerGetStandardResponse(uint32_t timeoutMs)
{
	size_t i = 0;
	uint32_t startTime = osKernelGetTickCount();
	uint8_t response[20];

	while (true)
	{
		if (serial_received_bytes_waiting() > 0U)
		{
			serial_read_data(1U, &response[i]);
			if (response[i] == '\n')
			{
				if (memcmp(response, "\r\n", (size_t)2) == 0)
				{
					i = 0;
					continue;
				}
				else if (memcmp(response, "OK\r\n", (size_t)4) == 0)
				{
					return MODEM_OK;
				}
				else if (memcmp(response, "SHUT OK\r\n", (size_t)9) == 0)
				{
					return MODEM_SHUT_OK;
				}
				else if (memcmp(response, "CLOSE OK\r\n", (size_t)10) == 0)
				{
					return MODEM_CLOSE_OK;
				}
				else if (memcmp(response, "SEND OK\r\n", (size_t)9) == 0)
				{
					return MODEM_SEND_OK;
				}
				else if (memcmp(response, "ERROR\r\n", (size_t)7) == 0)
				{
					return MODEM_ERROR;
				}
				else if (memcmp(response, "CLOSED\r\n", (size_t)8) == 0)
				{
					return MODEM_CLOSED;
				}
				else if (memcmp(response, "NORMAL POWER DOWN\r\n", (size_t)8) == 0)
				{
					return MODEM_POWERED_DOWN;
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
			if (osKernelGetTickCount() > startTime + timeoutMs)
			{
				return MODEM_TIMEOUT;
			}
		}
	}
}

static ModemStatus_t ClientSendBasicCommandResponse(AtCommand_t atCommand, uint32_t timeoutMs)
{
	AtCommandPacket_t atCommandPacket;
	AtResponsePacket_t atResponsePacket;

	atCommandPacket.atCommand = atCommand;
	atCommandPacket.timeoutMs = timeoutMs;

	if (osMessageQueuePut(commandQueueHandle, &atCommandPacket, 0U, 0U) != osOK)
	{
		return MODEM_FATAL_ERROR;
	}
	if (osMessageQueueGet(responseQueueHandle, &atResponsePacket, 0U, osWaitForever) != osOK)
	{
		return MODEM_FATAL_ERROR;
	}

	return atResponsePacket.atResponse;
}

// modem hello client and server functions

ModemStatus_t ModemHello(uint32_t timeoutMs)
{
	return ClientSendBasicCommandResponse(MODEM_COMMAND_HELLO, timeoutMs);
}

static void ServerModemHello(uint32_t timeoutMs)
{
	atResponsePacket.atResponse = ServerSendBasicCommandResponse("AT", timeoutMs);
	ServerFlushReadBufferOnError(atResponsePacket.atResponse);
	osMessageQueuePut(responseQueueHandle, &atResponsePacket, 0U, 0U);
}

// get signal strength client and server functions

typedef struct
{
	uint8_t signalStrength;
} GetSignalStrengthResponseData_t;

ModemStatus_t ModemGetSignalStrength(uint8_t *strength, uint32_t timeoutMs)
{
	AtCommandPacket_t atCommandPacket;
	AtResponsePacket_t atResponsePacket;
	GetSignalStrengthResponseData_t signalStrengthResponse;

	if (!strength)
	{
		return MODEM_BAD_PARAMETER;
	}

	atCommandPacket.atCommand = MODEM_COMMAND_SIGNAL_STRENGTH;
	atCommandPacket.timeoutMs = timeoutMs;

	if (osMessageQueuePut(commandQueueHandle, &atCommandPacket, 0U, 0U) != osOK)
	{
		return MODEM_FATAL_ERROR;
	}
	if (osMessageQueueGet(responseQueueHandle, &atResponsePacket, 0U, osWaitForever) != osOK)
	{
		return MODEM_FATAL_ERROR;
	}

	memcpy(&signalStrengthResponse, atResponsePacket.data, sizeof(signalStrengthResponse));
	*strength = signalStrengthResponse.signalStrength;

	return atResponsePacket.atResponse;
}

static void ServerGetSignalStrength(uint32_t timeoutMs)
{
	GetSignalStrengthResponseData_t signalStrengthResponseData;
	char responseText[20];
	char *dummy;

	atResponsePacket.atResponse = ServerSendBasicCommandTextResponse("AT+CSQ", responseText, sizeof(responseText), timeoutMs);
	if (atResponsePacket.atResponse == MODEM_OK)
	{
		if (memcmp(responseText, "+CSQ: ", (size_t)6) != 0)
		{
			atResponsePacket.atResponse = MODEM_UNEXPECTED_RESPONSE;
		}
		else
		{
			signalStrengthResponseData.signalStrength = (uint8_t)strtol(responseText + 6UL, &dummy, 10);
			memcpy(atResponsePacket.data, &signalStrengthResponseData, sizeof(signalStrengthResponseData));
		}
	}
	ServerFlushReadBufferOnError(atResponsePacket.atResponse);
	osMessageQueuePut(responseQueueHandle, &atResponsePacket, 0U, 0U);
}

// get network registration status client and server functions

typedef struct
{
	bool registrationStatus;
} GetRegistrationStatusResponseData_t;

ModemStatus_t ModemGetNetworkRegistrationStatus(bool *registered, uint32_t timeoutMs)
{
	AtCommandPacket_t atCommandPacket;
	AtResponsePacket_t atResponsePacket;
	GetRegistrationStatusResponseData_t getRegistrationStatusResponse;

	if (!registered)
	{
		return MODEM_BAD_PARAMETER;
	}

	atCommandPacket.atCommand = MODEM_COMMAND_NETWORK_REGISTRATION;
	atCommandPacket.timeoutMs = timeoutMs;

	if (osMessageQueuePut(commandQueueHandle, &atCommandPacket, 0U, 0U) != osOK)
	{
		return MODEM_FATAL_ERROR;
	}
	if (osMessageQueueGet(responseQueueHandle, &atResponsePacket, 0U, osWaitForever) != osOK)
	{
		return MODEM_FATAL_ERROR;
	}

	memcpy(&getRegistrationStatusResponse, atResponsePacket.data, sizeof(GetRegistrationStatusResponseData_t));
	*registered = getRegistrationStatusResponse.registrationStatus;

	return atResponsePacket.atResponse;
}

static void ServerNetworkRegistrationStatus(uint32_t timeoutMs)
{
	GetRegistrationStatusResponseData_t registrationStatusResponseData;
	uint8_t registrationStatusInt;
	char responseText[20];
	char *dummy;

	atResponsePacket.atResponse = ServerSendBasicCommandTextResponse("AT+CREG?", responseText, sizeof(responseText), timeoutMs);
	if (atResponsePacket.atResponse == MODEM_OK)
	{
		if (memcmp(responseText, "+CREG: 0,", (size_t)9) != 0)
		{
			atResponsePacket.atResponse = MODEM_UNEXPECTED_RESPONSE;
		}
		else
		{
			registrationStatusInt = (uint8_t)strtol(responseText + 9UL, &dummy, 10);
			if (registrationStatusInt == 1U || registrationStatusInt == 5U)
			{
				registrationStatusResponseData.registrationStatus = true;
			}
			else
			{
				registrationStatusResponseData.registrationStatus = false;
			}
			memcpy(atResponsePacket.data, &registrationStatusResponseData, sizeof(registrationStatusResponseData));
		}
	}

	ServerFlushReadBufferOnError(atResponsePacket.atResponse);

	osMessageQueuePut(responseQueueHandle, &atResponsePacket, 0U, 0U);
}

// set manual data read client and server functions

ModemStatus_t ModemSetManualDataRead(uint32_t timeoutMs)
{
	return ClientSendBasicCommandResponse(MODEM_COMMAND_SET_MANUAL_DATA_READ, timeoutMs);
}

static void ServerSetManualDataReceive(uint32_t timeoutMs)
{
	atResponsePacket.atResponse = ServerSendBasicCommandResponse("AT+CIPRXGET=1", timeoutMs);
	ServerFlushReadBufferOnError(atResponsePacket.atResponse);
	osMessageQueuePut(responseQueueHandle, &atResponsePacket, 0U, 0U);
}

// power down modem

ModemStatus_t ModemPowerDown(uint32_t timeoutMs)
{
	return ClientSendBasicCommandResponse(MODEM_COMMAND_POWER_DOWN, timeoutMs);
}

static void ServerPowerDown(uint32_t timeoutMs)
{
	atResponsePacket.atResponse = ServerSendBasicCommandResponse("AT+CPOWD=1", timeoutMs);
	ServerFlushReadBufferOnError(atResponsePacket.atResponse);
	osMessageQueuePut(responseQueueHandle, &atResponsePacket, 0U, 0U);
}

// activate data connection client and server functions

ModemStatus_t ModemActivateDataConnection(uint32_t timeoutMs)
{
	return ClientSendBasicCommandResponse(MODEM_COMMAND_ACTIVATE_DATA_CONNECTION, timeoutMs);
}

static void ServerActivateDataConnection(uint32_t timeoutMs)
{
	atResponsePacket.atResponse = ServerSendBasicCommandResponse("AT+CIICR", timeoutMs);
	ServerFlushReadBufferOnError(atResponsePacket.atResponse);
	osMessageQueuePut(responseQueueHandle, &atResponsePacket, 0U, 0U);
}

// configure data connection

typedef struct
{
	char apn[MODEM_MAX_APN_LENGTH];
	char username[MODEM_MAX_USERNAME_LENGTH];
	char password[MODEM_MAX_PASSWORD_LENGTH];
} ConfigureDataConnectionCommandData_t;

ModemStatus_t ModemConfigureDataConnection(const char *apn, const char *username, const char *password, uint32_t timeoutMs)
{
	AtCommandPacket_t atCommandPacket;
	AtResponsePacket_t atResponsePacket;
	ConfigureDataConnectionCommandData_t configureDataConnectionCommandData;

	if (apn == NULL ||
			username == NULL ||
			password == NULL ||
			strlen(apn) > (size_t)MODEM_MAX_APN_LENGTH ||
			strlen(username) > (size_t)MODEM_MAX_USERNAME_LENGTH ||
			strlen(password) > (size_t)MODEM_MAX_PASSWORD_LENGTH)
	{
		return MODEM_BAD_PARAMETER;
	}

	strcpy(configureDataConnectionCommandData.apn, apn);
	strcpy(configureDataConnectionCommandData.username, username);
	strcpy(configureDataConnectionCommandData.password, password);

	memcpy(atCommandPacket.data, &configureDataConnectionCommandData, sizeof(configureDataConnectionCommandData));
	atCommandPacket.atCommand = MODEM_COMMAND_CONFIGURE_DATA_CONNECTION;
	atCommandPacket.timeoutMs = timeoutMs;

	if (osMessageQueuePut(commandQueueHandle, &atCommandPacket, 0U, 0U) != osOK)
	{
		return MODEM_FATAL_ERROR;
	}
	if (osMessageQueueGet(responseQueueHandle, &atResponsePacket, 0U, osWaitForever) != osOK)
	{
		return MODEM_FATAL_ERROR;
	}

	return atResponsePacket.atResponse;
}

static void ServerConfigureDataConnection(uint32_t timeoutMs)
{
	ConfigureDataConnectionCommandData_t configureDataConnectionCommandData;
	char atCommandBuf[MODEM_MAX_AT_COMMAND_SIZE + 1];

	memcpy(&configureDataConnectionCommandData, atCommandPacket.data, sizeof(configureDataConnectionCommandData));
	strcpy(atCommandBuf, "AT+CSTT=\"");
	strcat(atCommandBuf, configureDataConnectionCommandData.apn);
	strcat(atCommandBuf, "\",\"");
	strcat(atCommandBuf, configureDataConnectionCommandData.username);
	strcat(atCommandBuf, "\",\"");
	strcat(atCommandBuf, configureDataConnectionCommandData.password);
	strcat(atCommandBuf, "\"\r");

	atResponsePacket.atResponse = ServerSendBasicCommandResponse(atCommandBuf, timeoutMs);
	ServerFlushReadBufferOnError(atResponsePacket.atResponse);
	osMessageQueuePut(responseQueueHandle, &atResponsePacket, 0U, 0U);
}

// deactivate data connection client and server functions

ModemStatus_t ModemDeactivateDataConnection(uint32_t timeoutMs)
{
	return ClientSendBasicCommandResponse(MODEM_COMMAND_DEACTIVATE_DATA_CONNECTION, timeoutMs);
}

static void ServerDeactivateDataConnection(uint32_t timeoutMs)
{
	atResponsePacket.atResponse = ServerSendBasicCommandResponse("AT+CIPSHUT", timeoutMs);
	ServerFlushReadBufferOnError(atResponsePacket.atResponse);
	osMessageQueuePut(responseQueueHandle, &atResponsePacket, 0U, 0U);
}

// open TCP connection client and server functions

typedef struct
{
	char url[MODEM_MAX_URL_ADDRESS_SIZE];
	uint16_t port;
} OpenTcpConnectionCommandData_t;

ModemStatus_t ModemOpenTcpConnection(const char *url, uint16_t port, uint32_t timeoutMs)
{
	AtCommandPacket_t atCommandPacket;
	AtResponsePacket_t atResponsePacket;
	OpenTcpConnectionCommandData_t openTcpConnectionCommandData;
	uint32_t startTime = osKernelGetTickCount();

	if (url == NULL || strlen(url) > (size_t)MODEM_MAX_URL_ADDRESS_SIZE)
	{
		return MODEM_BAD_PARAMETER;
	}

	if (tcpConnectedState)
	{
		return MODEM_TCP_ALREADY_CONNECTED;
	}

	strcpy(openTcpConnectionCommandData.url, url);
	openTcpConnectionCommandData.port = port;

	memcpy(atCommandPacket.data, &openTcpConnectionCommandData, sizeof(openTcpConnectionCommandData));
	atCommandPacket.atCommand = MODEM_COMMAND_OPEN_TCP_CONNECTION;
	atCommandPacket.timeoutMs = timeoutMs;

	if (osMessageQueuePut(commandQueueHandle, &atCommandPacket, 0U, 0U) != osOK)
	{
		return MODEM_FATAL_ERROR;
	}
	if (osMessageQueueGet(responseQueueHandle, &atResponsePacket, 0U, osWaitForever) != osOK)
	{
		return MODEM_FATAL_ERROR;
	}
	timeoutMs -= (osKernelGetTickCount() - startTime);

	if (atResponsePacket.atResponse == MODEM_OK)
	{
		while (true)
		{
			if (ModemGetTcpConnectedState())
			{
				break;
			}

			osDelay(500UL);
			if (osKernelGetTickCount() > startTime + timeoutMs)
			{
				return MODEM_TIMEOUT;
			}
		}
	}
	else
	{
		return atResponsePacket.atResponse;
	}

	return MODEM_OK;
}

static void ServerOpenTcpConnection(uint32_t timeoutMs)
{
	OpenTcpConnectionCommandData_t openTcpConnectionCommandData;
	char atCommandBuf[MODEM_MAX_AT_COMMAND_SIZE + 1];
	char portBuf[6];

	memcpy(&openTcpConnectionCommandData, atCommandPacket.data, sizeof(openTcpConnectionCommandData));
	itoa(openTcpConnectionCommandData.port, portBuf, 10);
	strcpy(atCommandBuf, "AT+CIPSTART=\"TCP\",\"");
	strcat(atCommandBuf, openTcpConnectionCommandData.url);
	strcat(atCommandBuf, "\",\"");
	strcat(atCommandBuf, portBuf);
	strcat(atCommandBuf, "\"\r");

	atResponsePacket.atResponse = ServerSendBasicCommandResponse(atCommandBuf, timeoutMs);
	ServerFlushReadBufferOnError(atResponsePacket.atResponse);
	osMessageQueuePut(responseQueueHandle, &atResponsePacket, 0U, 0U);
}

// close TCP connection client and server functions

ModemStatus_t ModemCloseTcpConnection(uint32_t timeoutMs)
{
	uint32_t startTime = osKernelGetTickCount();
	ModemStatus_t modemStatus;

	modemStatus = ClientSendBasicCommandResponse(MODEM_COMMAND_CLOSE_TCP_CONNECTION, timeoutMs);
	if (modemStatus != MODEM_OK)
	{
		return modemStatus;
	}
	timeoutMs -= (osKernelGetTickCount() - startTime);

	while (ModemGetTcpConnectedState())
	{
		osDelay(500UL);
		if (osKernelGetTickCount() > startTime + timeoutMs)
		{
			return MODEM_TIMEOUT;
		}
	}

	return MODEM_OK;
}

static void ServerCloseTcpConnection(uint32_t timeoutMs)
{
	atResponsePacket.atResponse = ServerSendBasicCommandResponse("AT+CIPCLOSE", timeoutMs);
	if (atResponsePacket.atResponse == MODEM_CLOSE_OK)
	{
		tcpConnectedState = false;
	}
	ServerFlushReadBufferOnError(atResponsePacket.atResponse);
	osMessageQueuePut(responseQueueHandle, &atResponsePacket, 0U, 0U);
}

// get own ip address client and server functions

typedef struct
{
	char ipAddress[MODEM_MAX_IP_ADDRESS_LENGTH];
} GetOwnIpAddressResponseData_t;

ModemStatus_t ModemGetOwnIpAddress(char *ipAddress, uint8_t length, uint32_t timeoutMs)
{
	AtCommandPacket_t atCommandPacket;
	AtResponsePacket_t atResponsePacket;
	GetOwnIpAddressResponseData_t getOwnIpAddressResponseData;

	if (ipAddress == NULL || length < 16U)
	{
		return MODEM_BAD_PARAMETER;
	}

	atCommandPacket.atCommand = MODEM_COMMAND_GET_OWN_IP_ADDRESS;
	atCommandPacket.timeoutMs = timeoutMs;

	if (osMessageQueuePut(commandQueueHandle, &atCommandPacket, 0U, 0U) != osOK)
	{
		return MODEM_FATAL_ERROR;
	}
	if (osMessageQueueGet(responseQueueHandle, &atResponsePacket, 0U, osWaitForever) != osOK)
	{
		return MODEM_FATAL_ERROR;
	}

	memcpy(&getOwnIpAddressResponseData, atResponsePacket.data, sizeof(getOwnIpAddressResponseData));
	strcpy(ipAddress, getOwnIpAddressResponseData.ipAddress);

	return atResponsePacket.atResponse;
}

static void ServerGetOwnIpAddress(uint32_t timeoutMs)
{
	GetOwnIpAddressResponseData_t getOwnIpAddressResponseData;
	uint8_t i = 0U;
	uint32_t startTime = osKernelGetTickCount();
	ModemStatus_t modemStatus;

	serial_write_data(9U, (uint8_t *)"AT+CIFSR\r");

	modemStatus = ServerGetEcho("AT+CIFSR", timeoutMs);
	if (modemStatus == MODEM_OK)
	{
		while (true)
		{
			if (serial_received_bytes_waiting() > 0U)
			{
				serial_read_data(1U, (uint8_t *)&getOwnIpAddressResponseData.ipAddress[i]);
				if (getOwnIpAddressResponseData.ipAddress[i] == '\n')
				{
					getOwnIpAddressResponseData.ipAddress[i - 1] = '\0';
					memcpy(atResponsePacket.data, &getOwnIpAddressResponseData, sizeof(getOwnIpAddressResponseData));
					modemStatus = MODEM_OK;
					break;
				}
				i++;

				if (i == MODEM_MAX_IP_ADDRESS_LENGTH - 1)
				{
					modemStatus = MODEM_OVERFLOW;
					break;
				}
			}
			else
			{
				if (osKernelGetTickCount() > startTime + timeoutMs)
				{
					modemStatus = MODEM_TIMEOUT;
					break;
				}
			}
		}
	}

	atResponsePacket.atResponse = modemStatus;
	ServerFlushReadBufferOnError(atResponsePacket.atResponse);
	osMessageQueuePut(responseQueueHandle, &atResponsePacket, 0U, 0U);
}

// TCP write client and server functions

typedef struct
{
	uint8_t data[MODEM_MAX_TCP_WRITE_SIZE];
	uint8_t length;
} TcpWriteCommandData_t;

ModemStatus_t ModemTcpWrite(const uint8_t *data, uint32_t length, uint32_t timeoutMs)
{
	ModemStatus_t modemStatus = MODEM_OK;
	uint32_t startTime = osKernelGetTickCount();
	uint32_t lengthWritten = 0UL;
	uint8_t sectionLengthToWrite;

	if (length == 0UL)
	{
		return MODEM_OK;
	}

	if (data == NULL)
	{
		return MODEM_BAD_PARAMETER;
	}

	while (lengthWritten < length)
	{
		if (length >= (uint32_t)MODEM_MAX_TCP_WRITE_SIZE)
		{
			sectionLengthToWrite = MODEM_MAX_TCP_WRITE_SIZE;
		}
		else
		{
			sectionLengthToWrite = (uint8_t)length;
		}

		modemStatus = ClientTcpWriteSection(data + lengthWritten, sectionLengthToWrite, timeoutMs);
		if (modemStatus != MODEM_SEND_OK)
		{
			break;
		}

		lengthWritten += (uint32_t)sectionLengthToWrite;
		timeoutMs -= (osKernelGetTickCount() - startTime);
	}

	return modemStatus;
}

static ModemStatus_t ClientTcpWriteSection(const uint8_t *data, uint8_t length, uint32_t timeoutMs)
{
	AtCommandPacket_t atCommandPacket;
	AtResponsePacket_t atResponsePacket;
	TcpWriteCommandData_t tcpWriteCommandData;

	memcpy(tcpWriteCommandData.data, data, length);
	tcpWriteCommandData.length = length;

	memcpy(atCommandPacket.data, &tcpWriteCommandData, sizeof(tcpWriteCommandData));
	atCommandPacket.atCommand = MODEM_COMMAND_TCP_WRITE;
	atCommandPacket.timeoutMs = timeoutMs;

	if (osMessageQueuePut(commandQueueHandle, &atCommandPacket, 0U, 0U) != osOK)
	{
		return MODEM_FATAL_ERROR;
	}
	if (osMessageQueueGet(responseQueueHandle, &atResponsePacket, 0U, osWaitForever) != osOK)
	{
		return MODEM_FATAL_ERROR;
	}

	return atResponsePacket.atResponse;
}

static void ServerTcpWrite(uint32_t timeoutMs)
{
	TcpWriteCommandData_t tcpWriteCommandData;
	char atCommandBuf[MODEM_MAX_AT_COMMAND_SIZE + 1];
	char lengthBuf[6];
	char prompt[5];
	ModemStatus_t modemStatus;
	uint32_t startTime = osKernelGetTickCount();
	uint8_t i = 0U;
	uint8_t dummy;

	memcpy(&tcpWriteCommandData, atCommandPacket.data, sizeof(tcpWriteCommandData));
	itoa(tcpWriteCommandData.length, lengthBuf, 10);
	strcpy(atCommandBuf, "AT+CIPSEND=");
	strcat(atCommandBuf, lengthBuf);
	strcat(atCommandBuf, "\r");

	serial_write_data((uint16_t)strlen(atCommandBuf), (uint8_t *)atCommandBuf);

	modemStatus = ServerGetEcho(atCommandBuf, timeoutMs);

	// the response from the modem can either be a prompt '> ' or the string 'ERROR\r\n'
	uint8_t promptExpectedLength = 2U;
	uint8_t promptNextPos = 0U;
	if (modemStatus == MODEM_OK)
	{
		while (true)
		{
			if (serial_received_bytes_waiting() >= 1U)
			{
				serial_read_data(1U, (uint8_t *)&prompt[promptNextPos]);
				if (prompt[promptNextPos] == 'E')
				{
					promptExpectedLength += 5U;
				}
				promptNextPos++;
				if (promptNextPos == promptExpectedLength)
				{
					if (promptExpectedLength == 2U)
					{
						if (memcmp(prompt, "> ", (size_t)2) == 0)
						{
							modemStatus = MODEM_OK;
						}
					}
					else
					{
						if (memcmp(prompt, "ERROR\r\n", (size_t)7) == 0)
						{
							modemStatus = MODEM_ERROR;
						}
						else
						{
							modemStatus = MODEM_UNEXPECTED_RESPONSE;
						}
					}
					break;
				}
			}
			else
			{
				if (osKernelGetTickCount() > startTime + timeoutMs)
				{
					modemStatus = MODEM_TIMEOUT;
					break;
				}
			}
		}
	}

	if (modemStatus == MODEM_OK)
	{
		serial_write_data((uint16_t)tcpWriteCommandData.length, tcpWriteCommandData.data);
		while (i < tcpWriteCommandData.length)
		{
			if (serial_read_data(1U, &dummy) == 1U)
			{
				i++;
			}
			else
			{
				if (osKernelGetTickCount() > startTime + timeoutMs)
				{
					modemStatus = MODEM_TIMEOUT;
					break;
				}
			}
		}
	}

	if (modemStatus == MODEM_OK)
	{
		modemStatus = ServerGetStandardResponse(timeoutMs);
	}

	atResponsePacket.atResponse = modemStatus;
	ServerFlushReadBufferOnError(atResponsePacket.atResponse);
	osMessageQueuePut(responseQueueHandle, &atResponsePacket, 0U, 0U);
}

// get tcp read data waiting length client and server functions

typedef struct
{
	uint16_t length;
} GetTcpReadDataWaitinghResponseData_t;

ModemStatus_t ModemGetTcpReadDataWaitingLength(uint32_t *length, uint32_t timeoutMs)
{
	AtCommandPacket_t atCommandPacket;
	AtResponsePacket_t atResponsePacket;
	GetTcpReadDataWaitinghResponseData_t getTcpReadDataWaitinghResponseData;

	if (!length)
	{
		return MODEM_BAD_PARAMETER;
	}

	atCommandPacket.atCommand = MODEM_COMMAND_GET_TCP_READ_DATA_WAITING_LENGTH;
	atCommandPacket.timeoutMs = timeoutMs;

	if (osMessageQueuePut(commandQueueHandle, &atCommandPacket, 0U, 0U) != osOK)
	{
		return MODEM_FATAL_ERROR;
	}
	if (osMessageQueueGet(responseQueueHandle, &atResponsePacket, 0U, osWaitForever) != osOK)
	{
		return MODEM_FATAL_ERROR;
	}

	memcpy(&getTcpReadDataWaitinghResponseData, atResponsePacket.data, sizeof(getTcpReadDataWaitinghResponseData));
	*length = getTcpReadDataWaitinghResponseData.length;

	return atResponsePacket.atResponse;
}

static void ServerGetTcpReadDataWaitingLength(uint32_t timeoutMs)
{
	GetTcpReadDataWaitinghResponseData_t getTcpReadDataWaitinghResponseData;
	char responseText[25];
	char *dummy;

	atResponsePacket.atResponse = ServerSendBasicCommandTextResponse("AT+CIPRXGET=4", responseText, sizeof(responseText), timeoutMs);
	if (atResponsePacket.atResponse == MODEM_OK)
	{
		if (memcmp(responseText, "+CIPRXGET: 4,", (size_t)13) != 0)
		{
			atResponsePacket.atResponse = MODEM_UNEXPECTED_RESPONSE;
		}
		else
		{
			getTcpReadDataWaitinghResponseData.length = (uint16_t)strtol(responseText + 13UL, &dummy, 10);
			memcpy(atResponsePacket.data, &getTcpReadDataWaitinghResponseData, sizeof(getTcpReadDataWaitinghResponseData));
		}
	}
	ServerFlushReadBufferOnError(atResponsePacket.atResponse);
	osMessageQueuePut(responseQueueHandle, &atResponsePacket, 0U, 0U);
}

// tcp read client and server functions

typedef struct
{
	uint8_t lengthToRead;
} TcpReadCommandData_t;

typedef struct
{
	uint8_t lengthRead;
	uint8_t data[MODEM_MAX_TCP_READ_SIZE];
} TcpReadResponseData_t;

ModemStatus_t ModemTcpRead(uint32_t lengthToRead, uint32_t *lengthRead, uint8_t *buffer, uint32_t timeoutMs)
{
	ModemStatus_t modemStatus;
	uint32_t startTime = osKernelGetTickCount();
	uint8_t sectionLengthRead;
	uint8_t sectionLengthToRead;

	if (!lengthRead || !buffer)
	{
		return MODEM_BAD_PARAMETER;
	}

	if (lengthToRead == 0UL)
	{
		*lengthRead = 0UL;
		return MODEM_OK;
	}

	*lengthRead = 0UL;
	while (*lengthRead < lengthToRead)
	{
		if (lengthToRead >= (uint32_t)MODEM_MAX_TCP_READ_SIZE)
		{
			sectionLengthToRead = MODEM_MAX_TCP_READ_SIZE;
		}
		else
		{
			sectionLengthToRead = (uint8_t)lengthToRead;
		}
		modemStatus = ClientTcpReadSection(sectionLengthToRead, &sectionLengthRead, buffer + *lengthRead, timeoutMs);

		if (modemStatus != MODEM_OK)
		{
			break;
		}

		*lengthRead += (uint32_t)sectionLengthRead;
		timeoutMs -= (osKernelGetTickCount() - startTime);
	}

	return modemStatus;
}

static ModemStatus_t ClientTcpReadSection(uint8_t lengthToRead, uint8_t *lengthRead, uint8_t *buffer, uint32_t timeoutMs)
{
	AtCommandPacket_t atCommandPacket;
	AtResponsePacket_t atResponsePacket;
	TcpReadCommandData_t tcpReadCommandData;
	TcpReadResponseData_t tcpReadResponseData;

	tcpReadCommandData.lengthToRead = lengthToRead;
	memcpy(atCommandPacket.data, &tcpReadCommandData, sizeof(tcpReadCommandData));

	atCommandPacket.atCommand = MODEM_COMMAND_TCP_READ;
	atCommandPacket.timeoutMs = timeoutMs;

	if (osMessageQueuePut(commandQueueHandle, &atCommandPacket, 0U, 0U) != osOK)
	{
		return MODEM_FATAL_ERROR;
	}
	if (osMessageQueueGet(responseQueueHandle, &atResponsePacket, 0U, osWaitForever) != osOK)
	{
		return MODEM_FATAL_ERROR;
	}

	memcpy(&tcpReadResponseData, atResponsePacket.data, sizeof(tcpReadResponseData));
	*lengthRead = tcpReadResponseData.lengthRead;
	memcpy(buffer, tcpReadResponseData.data, tcpReadResponseData.lengthRead);

	return atResponsePacket.atResponse;
}

static void ServerTcpRead(uint32_t timeoutMs)
{
	TcpReadCommandData_t tcpReadCommandData;
	TcpReadResponseData_t tcpReadResponseData;
	char commandText[25];
	char responseText[25];
	char numberBuf[5];
	ModemStatus_t modemStatus;
	uint8_t i = 0U;
	uint32_t startTime = osKernelGetTickCount();

	memcpy(&tcpReadCommandData, atCommandPacket.data, sizeof(tcpReadCommandData));

	strcpy(commandText, "AT+CIPRXGET=2,");
	itoa(tcpReadCommandData.lengthToRead, numberBuf, 10);
	strcat(commandText, numberBuf);
	strcat(commandText, "\r");

	serial_write_data(strlen(commandText), (uint8_t *)commandText);

	modemStatus = ServerGetEcho(commandText, timeoutMs);
	if (modemStatus != MODEM_OK)
	{
		modemStatus = MODEM_TIMEOUT;
	}

	if (modemStatus == MODEM_OK)
	{
		while (true)
		{
			if (serial_received_bytes_waiting() > 0U)
			{
				serial_read_data(1U, (uint8_t *)&responseText[i]);
				if (responseText[i] == '\n')
				{
					modemStatus = MODEM_OK;
					break;
				}
				i++;

				if (i == sizeof(responseText))
				{
					modemStatus = MODEM_OVERFLOW;
					break;
				}
			}
			else
			{
				if (osKernelGetTickCount() > startTime + timeoutMs)
				{
					modemStatus = MODEM_TIMEOUT;
					break;
				}
			}
		}
	}

	if (modemStatus == MODEM_OK)
	{
		char *dummy;

		if (memcmp(responseText, "+CIPRXGET: 2,", (size_t)13) != 0)
		{
			modemStatus = MODEM_UNEXPECTED_RESPONSE;
		}
		else
		{
			tcpReadResponseData.lengthRead = (uint16_t)strtol(responseText + 13UL, &dummy, 10);
		}
	}

	if (modemStatus == MODEM_OK)
	{
		i = 0U;
		while (i < tcpReadResponseData.lengthRead)
		{
			if (serial_read_data(1U, &tcpReadResponseData.data[i]) > 0U)
			{
				i++;
			}
			else
			{
				if (osKernelGetTickCount() > startTime + timeoutMs)
				{
					modemStatus = MODEM_TIMEOUT;
					break;
				}
			}
		}

		memcpy(atResponsePacket.data, &tcpReadResponseData, sizeof(tcpReadResponseData));
	}

	if (modemStatus == MODEM_OK)
	{
		modemStatus = ServerGetStandardResponse(timeoutMs);
	}

	atResponsePacket.atResponse = modemStatus;
	ServerFlushReadBufferOnError(atResponsePacket.atResponse);
	osMessageQueuePut(responseQueueHandle, &atResponsePacket, 0U, 0U);
}

const char *ModemStatusToText(ModemStatus_t modemStatus)
{
	switch (modemStatus)
	{
	case MODEM_OK:
		return "MODEM_OK";

	case MODEM_CLOSE_OK:
		return "MODEM_CLOSE_OK";

	case MODEM_SHUT_OK:
		return "MODEM_SHUT_OK";

	case MODEM_SEND_OK:
		return "MODEM_SEND_OK";

	case MODEM_ERROR:
		return "MODEM_ERROR";

	case MODEM_CLOSED:
		return "MODEM_CLOSED";

	case MODEM_TIMEOUT:
		return "MODEM_TIMEOUT";

	case MODEM_NO_RESPONSE:
		return "MODEM_NO_RESPONSE";

	case MODEM_UNEXPECTED_RESPONSE:
		return "MODEM_UNEXPECTED_RESPONSE";

	case MODEM_OVERFLOW:
		return "MODEM_OVERFLOW";

	case MODEM_BAD_PARAMETER:
		return "MODEM_BAD_PARAMETER";

	case MODEM_TCP_ALREADY_CONNECTED:
		return "MODEM_TCP_ALREADY_CONNECTED";

	case MODEM_FATAL_ERROR:
		return "MODEM_FATAL_ERROR";

	case MODEM_POWERED_DOWN:
		return "MODEM_POWERED_DOWN";

	default:
		return "MODEM_UNKNOWN_STATUS";
	}
}
