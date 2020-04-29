#ifndef INC_MODEM_H_
#define INC_MODEM_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
	MODEM_OK,
	MODEM_ERROR,
	MODEM_TIMEOUT,
	MODEM_NO_RESPONSE,
	MODEM_UNEXPECTED_RESPONSE,
	MODEM_OVERFLOW,
	MODEM_BAD_PARAMETER
} ModemStatus_t;

typedef enum
{
	HTTP_GET = 0,
	HTTP_POST = 1,
	HTTP_HEAD = 2
} HttpAction_t;

ModemStatus_t ModemInit(void);
ModemStatus_t ModemHello(void);
ModemStatus_t ModemGetSignalStrength(uint8_t *strength);
ModemStatus_t ModemGetNetworkRegistered(bool *registered);
ModemStatus_t ModemHttpInit(char *apn, char *username, char *password);
ModemStatus_t ModemHttpClose(void);
ModemStatus_t ModemHttpSendURL(char *url);
ModemStatus_t ModemHttpSendAction(HttpAction_t httpAction, uint16_t *httpResponseCode, uint16_t *responseLength);
ModemStatus_t ModemHttpReadResponse(uint16_t startPosition, uint16_t length, uint8_t *responseBuffer);
ModemStatus_t ModemDisableNewSMSNotification(void);

#endif
