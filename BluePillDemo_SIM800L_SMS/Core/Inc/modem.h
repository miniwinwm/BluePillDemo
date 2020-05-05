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

typedef struct
{
	char text[161];
	char from[30];
	char date[12];
	char time[12];
} SmsMessage_t;

typedef void (*new_sms_callback_t)(SmsMessage_t *new_sms);

ModemStatus_t ModemInit(void);
ModemStatus_t ModemHello(void);
ModemStatus_t ModemSetSMSTextMode(void);
ModemStatus_t ModemGetSignalStrength(uint8_t *strength);
ModemStatus_t ModemGetNetworkRegistrationStatus(bool *registered);
ModemStatus_t ModemDisableNewSMSNotification(void);
ModemStatus_t ModemSendSMS(char *address, char *message);
ModemStatus_t ModemListAllUnreadSMS(new_sms_callback_t new_sms_callback);
ModemStatus_t ModemDeleteAllReadAndSentSMS(void);

#endif
