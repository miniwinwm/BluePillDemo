#ifndef INC_MODEM_H_
#define INC_MODEM_H_

#include <stdbool.h>
#include <stdint.h>

#define MODEM_MAX_URC_LENGTH			50U			// maximum length of URC accepted
#define MODEM_URC_TIMEOUT_MS			25UL			// how long to wait for URC reception to finish after it has started
#define MODEM_SERVER_LOOP_PERIOD_MS		25UL
#define MODEM_MAX_APN_LENGTH			25
#define MODEM_MAX_USERNAME_LENGTH		25
#define MODEM_MAX_PASSWORD_LENGTH		25
#define MODEM_MAX_AT_COMMAND_SIZE		100U
#define MODEM_MAX_AT_RESPONSE_SIZE		100U
#define MODEM_MAX_URL_ADDRESS_SIZE		70
#define MODEM_MAX_IP_ADDRESS_LENGTH		20
#define MODEM_MAX_TCP_WRITE_SIZE		99U
#define MODEM_MAX_TCP_READ_SIZE			99U

typedef enum
{
	// ok statuses
	MODEM_OK = 0,
	MODEM_CLOSE_OK = 1,
	MODEM_SHUT_OK = 2,
	MODEM_SEND_OK = 3,
	MODEM_CLOSED = 4,
	MODEM_POWERED_DOWN = 5,

	// error statuses
	MODEM_ERROR = -1,
	MODEM_TIMEOUT = -2,
	MODEM_NO_RESPONSE = -3,
	MODEM_UNEXPECTED_RESPONSE = -4,
	MODEM_OVERFLOW = -5,
	MODEM_BAD_PARAMETER = -6,
	MODEM_TCP_ALREADY_CONNECTED = -7,
	MODEM_FATAL_ERROR = -8
} ModemStatus_t;

typedef enum
{
	MODEM_COMMAND_HELLO,							// AT
	MODEM_COMMAND_NETWORK_REGISTRATION,				// AT+CREG?
	MODEM_COMMAND_SIGNAL_STRENGTH,					// AT+CSQ
	MODEM_COMMAND_SET_MANUAL_DATA_READ,				// AT+CIPRXGET (mode = 1)
	MODEM_COMMAND_CONFIGURE_DATA_CONNECTION,		// AT+CSTT
	MODEM_COMMAND_ACTIVATE_DATA_CONNECTION,			// AT+CIICR
	MODEM_COMMAND_GET_OWN_IP_ADDRESS,				// AT+CIFSR
	MODEM_COMMAND_OPEN_TCP_CONNECTION,				// AT+CIPSTART
	MODEM_COMMAND_TCP_WRITE,						// AT+CIPSEND
	MODEM_COMMAND_GET_TCP_READ_DATA_WAITING_LENGTH,	// AT+CIPRXGET (mode = 4)
	MODEM_COMMAND_TCP_READ,							// AT+CIPRXGET (mode = 2)
	MODEM_COMMAND_CLOSE_TCP_CONNECTION,				// AT+CIPCLOSE
	MODEM_COMMAND_DEACTIVATE_DATA_CONNECTION,		// AT+CIPSHUT
	MODEM_COMMAND_POWER_DOWN						// AT+CPOWD
} AtCommand_t;

typedef struct
{
	uint32_t timeoutMs;
	AtCommand_t atCommand;
	uint8_t data[MODEM_MAX_AT_COMMAND_SIZE];
} AtCommandPacket_t;

typedef struct
{
	ModemStatus_t atResponse;
	uint8_t data[MODEM_MAX_AT_RESPONSE_SIZE];
} AtResponsePacket_t;

void DoModemTask(void);
void ModemReset(void);
ModemStatus_t ModemInit(void);
ModemStatus_t ModemHello(uint32_t timeoutMs);
ModemStatus_t ModemGetNetworkRegistrationStatus(bool *registered, uint32_t timeoutMs);
ModemStatus_t ModemGetSignalStrength(uint8_t *strength, uint32_t timeoutMs);
ModemStatus_t ModemSetManualDataRead(uint32_t timeoutMs);
ModemStatus_t ModemConfigureDataConnection(const char *apn, const char *username, const char *password, uint32_t timeoutMs);
ModemStatus_t ModemActivateDataConnection(uint32_t timeoutMs);
ModemStatus_t ModemGetOwnIpAddress(char *ipAddress, uint8_t length, uint32_t timeoutMs);
ModemStatus_t ModemOpenTcpConnection(const char *url, uint16_t port, uint32_t timeoutMs);
ModemStatus_t ModemTcpWrite(const uint8_t *data, uint32_t length, uint32_t timeoutMs);
ModemStatus_t ModemGetTcpReadDataWaitingLength(uint32_t *length, uint32_t timeoutMs);
ModemStatus_t ModemTcpRead(uint32_t lengthToRead, uint32_t *lengthRead, uint8_t *buffer, uint32_t timeoutMs);
ModemStatus_t ModemCloseTcpConnection(uint32_t timeoutMs);
ModemStatus_t ModemDeactivateDataConnection(uint32_t timeoutMs);
ModemStatus_t ModemPowerDown(uint32_t timeoutMs);
const char *ModemStatusToText(ModemStatus_t modemStatus);
bool ModemGetTcpConnectedState(void);

#endif
