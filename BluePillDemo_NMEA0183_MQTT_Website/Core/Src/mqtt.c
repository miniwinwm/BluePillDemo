#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "cmsis_os.h"
#include "mqtt.h"
#include "modem.h"
#include "stm32f1xx_hal.h"

static uint8_t EncodeRemainingLength(uint32_t remainingLength, uint8_t buffer[4]);
static uint32_t DecodeRemainingLength(uint8_t buffer[4]);

static PublishCallback_t publishCallback;
static PingResponseCallback_t pingCallback;
static SubscribeResponseCallback_t subscribeCallback;
static UnsubscribeResponseCallback_t unsubscribeCallback;

void MqttSetPublishCallback(PublishCallback_t callback)
{
	publishCallback = callback;
}

void MqttSetPingResponseCallback(PingResponseCallback_t callback)
{
	pingCallback = callback;
}

void MqttSetSubscribeResponseCallback(SubscribeResponseCallback_t callback)
{
	subscribeCallback = callback;
}

void MqttSetUnsubscribeResponseCallback(UnsubscribeResponseCallback_t callback)
{
	unsubscribeCallback = callback;
}

MqttStatus_t MqttConnect(char *clientId, char *username, char *password, uint16_t keepAlive, uint32_t timeoutMs)
{
	uint8_t remainingLengthBuffer[4];
	uint32_t remainingLength;
	uint32_t packetLength;
	uint8_t remainingLengthLength;
	uint8_t *packet;
	uint16_t p = 0U;
	uint32_t startTime = osKernelGetTickCount();
	MqttStatus_t mqttStatus;

	// check parameters
	if (!clientId)
	{
		return MQTT_BAD_PARAMETER;
	}

	// calculate remaining length
	remainingLength = 12UL + (uint32_t)strlen(clientId);
	if (username)
	{
		remainingLength += 2UL;
		remainingLength += (uint32_t)strlen(username);
	}
	if (password)
	{
		remainingLength += 2UL;
		remainingLength += (uint32_t)strlen(password);
	}

	// encode remaining length
	remainingLengthLength = EncodeRemainingLength(remainingLength, remainingLengthBuffer);

	// calculate length of packet and allocate memory
	packetLength = 1UL + (uint32_t)remainingLengthLength + remainingLength;
	packet = pvPortMalloc(packetLength);
	if (!packet)
	{
		return MQTT_NO_MEMORY;
	}

	// packet type
	packet[p] = MQTT_CONNECT_REQ_PACKET_ID;
	p++;

	// remaining length
	memcpy(&packet[p], remainingLengthBuffer, (size_t)remainingLengthLength);
	p += (uint32_t)remainingLengthLength;

	memcpy(&packet[p], "\x00\x04MQTT\x04", 7);
	p += 7UL;

	// flags
	packet[p] = 0x02U;
	if (username)
	{
		packet[p] |= 0x80U;
	}
	if (password)
	{
		packet[p] |= 0x40U;
	}
	p++;

	// keepalive time
	packet[p] = (uint8_t)(keepAlive >> 8);
	p++;
	packet[p] = (uint8_t)keepAlive;
	p++;

	// client id length
	packet[p] = strlen(clientId) / 0xffU;
	p++;
	packet[p] = strlen(clientId) & 0xffU;
	p++;

	// client id
	memcpy(&packet[p], clientId, strlen(clientId));
	p += (uint32_t)strlen(clientId);

	// username if supplied
	if (username)
	{
		packet[p] = strlen(username) / 0xffU;
		p++;
		packet[p] = strlen(username) & 0xffU;
		p++;

		memcpy(&packet[p], username, strlen(username));
		p += (uint32_t)strlen(username);
	}

	// password if supplied
	if (password)
	{
		packet[p] = strlen(password) / 0xffU;
		p++;
		packet[p] = strlen(password) & 0xffU;
		p++;

		memcpy(&packet[p], password, strlen(password));
		p += (uint32_t)strlen(password);
	}

	// send packet
	if (ModemTcpWrite(packet, packetLength, timeoutMs) != MODEM_SEND_OK)
	{
		vPortFree(packet);
		return MQTT_TCP_ERROR;
	}

	// deallocate
	vPortFree(packet);

	timeoutMs -= (osKernelGetTickCount() - startTime);

	// wait for response
	while (true)
	{
		mqttStatus = MqttHandleResponse(timeoutMs);
		if (mqttStatus != MQTT_NO_RESPONSE)
		{
			break;
		}

		osDelay(250UL);

		if (osKernelGetTickCount() > startTime + timeoutMs)
		{
			mqttStatus = MQTT_TIMEOUT;
			break;
		}
	}

	return mqttStatus;
}

MqttStatus_t MqttPing(uint32_t timeoutMs)
{
	// create packet
	const uint8_t packet[2] = {MQTT_PING_REQ_PACKET_ID, 0x00U};

	// send packet
	if (ModemTcpWrite(packet, 2UL, timeoutMs) != MODEM_SEND_OK)
	{
		return MQTT_TCP_ERROR;
	}

	return MQTT_OK;
}

MqttStatus_t MqttSubscribe(char *topic, uint16_t packetIdentifier, uint32_t timeoutMs)
{
	uint8_t *packet;
	uint32_t remainingLength;
	uint32_t packetLength;
	uint8_t remainingLengthBuffer[4];
	uint8_t remainingLengthLength;
	uint8_t p = 0U;

	// check parameters
	if (!topic || strlen(topic) == (size_t)0 || strlen(topic) > (size_t)250)
	{
		return MQTT_BAD_PARAMETER;
	}

	// encode remaining length
	remainingLength = 5UL + (uint32_t)strlen(topic);
	remainingLengthLength = EncodeRemainingLength(remainingLength, remainingLengthBuffer);

	// calculate packet length and allocate memory
	packetLength = 1UL + (uint32_t)remainingLengthLength + remainingLength;
	packet = pvPortMalloc(packetLength);
	if (!packet)
	{
		return MQTT_NO_MEMORY;
	}

	// packet type
	packet[p] = MQTT_SUBSCRIBE_REQ_PACKET_ID | 0x02U;
	p++;

	// remaining length
	memcpy(&packet[p], remainingLengthBuffer, (size_t)remainingLengthLength);
	p += (uint32_t)remainingLengthLength;

	// packet identifier
	packet[p] = (uint8_t)(packetIdentifier >> 8);
	p++;
	packet[p] = (uint8_t)packetIdentifier;
	p++;

	// topic length
	packet[p] = 0x00;
	p++;
	packet[p] = (uint8_t)strlen(topic);
	p++;

	// topic
	memcpy(&packet[p], topic, strlen(topic));
	p += strlen(topic);

	// qos
	packet[p] = 0x00U;

	// send packet
	if (ModemTcpWrite(packet, packetLength, timeoutMs) != MODEM_SEND_OK)
	{
		vPortFree(packet);
		return MQTT_TCP_ERROR;
	}

	// deallocate
	vPortFree(packet);

	return MQTT_OK;
}

MqttStatus_t MqttUnsubscribe(char *topic, uint16_t packetIdentifier, uint32_t timeoutMs)
{
	uint8_t *packet;
	uint32_t remainingLength;
	uint32_t packetLength;
	uint8_t remainingLengthBuffer[4];
	uint8_t remainingLengthLength;
	uint8_t p = 0U;

	// check parameters
	if (!topic || strlen(topic) == (size_t)0 || strlen(topic) > (size_t)250)
	{
		return MQTT_BAD_PARAMETER;
	}

	// encode remaining length
	remainingLength = 4UL + (uint32_t)strlen(topic);
	remainingLengthLength = EncodeRemainingLength(remainingLength, remainingLengthBuffer);

	// calculate packet length and allocate memory
	packetLength = 1UL + (uint32_t)remainingLengthLength + remainingLength;
	packet = pvPortMalloc(packetLength);
	if (!packet)
	{
		return MQTT_NO_MEMORY;
	}

	// packet type
	packet[p] = MQTT_UNSUBSCRIBE_REQ_PACKET_ID | 0x02U;
	p++;

	// remaining length
	memcpy(&packet[p], remainingLengthBuffer, (size_t)remainingLengthLength);
	p += (uint32_t)remainingLengthLength;

	// packet identifier
	packet[p] = (uint8_t)(packetIdentifier >> 8);
	p++;
	packet[p] = (uint8_t)packetIdentifier;
	p++;

	// topic length
	packet[p] = 0x00U;
	p++;
	packet[p] = (uint8_t)strlen(topic);
	p++;

	// topic
	memcpy(&packet[p], topic, strlen(topic));
	p += strlen(topic);

	// send packet
	if (ModemTcpWrite(packet, packetLength, timeoutMs) != MODEM_SEND_OK)
	{
		vPortFree(packet);
		return MQTT_TCP_ERROR;
	}

	// deallocate
	vPortFree(packet);

	return MQTT_OK;
}

MqttStatus_t MqttPublish(char *topic, uint8_t *payload, uint32_t payloadLength, bool retain, uint32_t timeoutMs)
{
	uint32_t remainingLength;
	uint32_t packetLength;
	uint8_t remainingLengthBuffer[4] = {0};
	uint8_t remainingLengthLength;
	uint8_t *packet;
	uint32_t p = 0U;

	// check parameters
	if (!topic || !payload || strlen(topic) == (size_t)0 || strlen(topic) > (size_t)250)
	{
		return MQTT_BAD_PARAMETER;
	}

	// encode remaining length
	remainingLength = 2UL + (uint32_t)strlen(topic) + payloadLength;
	remainingLengthLength = EncodeRemainingLength(remainingLength, remainingLengthBuffer);

	// calculate packet length and allocate memory
	packetLength = 1UL + (uint32_t)remainingLengthLength + remainingLength;
	packet = pvPortMalloc(packetLength);
	if (!packet)
	{
		return MQTT_NO_MEMORY;
	}

	// packet type
	packet[p] = MQTT_PUBLISH_PACKET_ID;
	if (retain)
	{
		packet[p] |= 0x01U;
	}
	p++;

	// remaining length
	memcpy(&packet[p], remainingLengthBuffer, (size_t)remainingLengthLength);
	p += (uint32_t)remainingLengthLength;

	// topic length
	packet[p] = 0x00U;
	p++;
	packet[p] = (uint8_t)strlen(topic);
	p++;

	// topic
	memcpy(&packet[p], topic, strlen(topic));
	p += strlen(topic);

	// payload
	memcpy(&packet[p], payload, payloadLength);
	p += payloadLength;

	// send packet
	if (ModemTcpWrite(packet, packetLength, timeoutMs) != MODEM_SEND_OK)
	{
		vPortFree(packet);
		return MQTT_TCP_ERROR;
	}

	// deallocate
	vPortFree(packet);

	return MQTT_OK;
}

MqttStatus_t MqttDisconnect(uint32_t timeoutMs)
{
	const uint8_t packet[4] = {MQQT_DISCONNECT_PACKET_ID, 0x00U, 0x00U, 0x00U};

	ModemStatus_t modemStatus = ModemTcpWrite(packet, sizeof(packet), timeoutMs);
	if (modemStatus != MODEM_SEND_OK)
	{
		return MQTT_TCP_ERROR;
	}

	return MQTT_OK;
}

MqttStatus_t MqttHandleResponse(uint32_t timeoutMs)
{
	uint8_t packetType;
	uint32_t lengthRead;
	uint8_t remainingLengthBuffer[4];
	uint32_t startTime = osKernelGetTickCount();
	uint32_t bytesWaiting;
	uint8_t i = 0U;
	uint32_t bytesRead;
	uint32_t remainingLength;
	uint8_t *remainingData = NULL;
	MqttStatus_t mqttStatus;

	if (ModemGetTcpReadDataWaitingLength(&bytesWaiting, timeoutMs) == MODEM_OK)
	{
		timeoutMs -= (osKernelGetTickCount() - startTime);

		if (bytesWaiting > 0UL)
		{
			// read response type
			if (ModemTcpRead(1UL, &lengthRead, &packetType, timeoutMs) != MODEM_OK)
			{
				return MQTT_TCP_ERROR;
			}
			timeoutMs -= (osKernelGetTickCount() - startTime);

			// read remaining length
			while (true)
			{
				if (ModemGetTcpReadDataWaitingLength(&bytesWaiting, timeoutMs) != MODEM_OK)
				{
					return MQTT_TCP_ERROR;
				}
				timeoutMs -= (osKernelGetTickCount() - startTime);

				if (bytesWaiting > 0UL)
				{
					if (ModemTcpRead(1UL, &bytesRead, &remainingLengthBuffer[i], timeoutMs) != MODEM_OK)
					{
						return MQTT_TCP_ERROR;
					}
					timeoutMs -= (osKernelGetTickCount() - startTime);

					if ((remainingLengthBuffer[i] & 0x80U) == 0x00U)
					{
						break;
					}

					i++;
					if (i == 5U)
					{
						return MQTT_UNEXPECTED_RESPONSE;
					}
				}
				else
				{
					osDelay(250UL);

					if (osKernelGetTickCount() > startTime + timeoutMs)
					{
						return MQTT_TIMEOUT;
					}
				}
			}

			// decode remaining length
			remainingLength = DecodeRemainingLength(remainingLengthBuffer);
			if (remainingLength > 0UL)
			{
				remainingData = pvPortMalloc(remainingLength);
				if (!remainingData)
				{
					return MQTT_NO_MEMORY;
				}

				// read remaining data
				while (true)
				{
					if (ModemGetTcpReadDataWaitingLength(&bytesWaiting, timeoutMs) != MODEM_OK)
					{
						vPortFree(remainingData);
						return MQTT_TCP_ERROR;
					}
					timeoutMs -= (osKernelGetTickCount() - startTime);

					if (bytesWaiting >= remainingLength)
					{
						if (ModemTcpRead(remainingLength, &bytesRead, remainingData, timeoutMs) != MODEM_OK)
						{
							vPortFree(remainingData);
							return MQTT_TCP_ERROR;
						}
						timeoutMs -= (osKernelGetTickCount() - startTime);
						break;
					}
					else
					{
						osDelay(250UL);

						if (osKernelGetTickCount() > startTime + timeoutMs)
						{
							vPortFree(remainingData);
							return MQTT_TIMEOUT;
						}
					}
				}
			}

			if ((packetType & MQTT_PACKET_ID_MASK) == MQTT_PUBLISH_PACKET_ID && publishCallback)
			{
				if (remainingLength < 6UL)
				{
					mqttStatus = MQTT_UNEXPECTED_RESPONSE;
				}
				else
				{
					// publish response
					publishCallback((char *)&remainingData[2], 					// topic starts at byte 3
							remainingData[1], 									// topic length is byte 0, byte 1 but byte 0 is always 0
							remainingData + (uint32_t)remainingData[1] + 2UL, 	// payload starts at byte (topic length + 2)
							remainingLength - remainingData[1] - 2UL);
					mqttStatus = MQTT_PUBLISH;
				}
			}
			else if ((packetType & MQTT_PACKET_ID_MASK) == MQTT_PING_RESP_PACKET_ID && pingCallback)
			{
				// ping ack
				pingCallback();
				mqttStatus = MQTT_PING_ACK;
			}
			else if ((packetType & MQTT_PACKET_ID_MASK) == MQTT_SUBSCRIBE_ACK_PACKET_ID && subscribeCallback)
			{
				// subscribe ack
				if (remainingLength != 3UL || (remainingData[2] != 0x00U && remainingData[2] != 0x80U))
				{
					mqttStatus = MQTT_UNEXPECTED_RESPONSE;
				}
				else
				{
					subscribeCallback(((uint16_t)remainingData[0] << 8) + (uint16_t)remainingData[1], remainingData[2] == 0x00U);
					mqttStatus = MQTT_SUBSCRIBE_ACK;
				}
			}
			else if ((packetType & MQTT_PACKET_ID_MASK) == MQTT_UNSUBSCRIBE_ACK_PACKET_ID && unsubscribeCallback)
			{
				// unsubscribe ack
				if (remainingLength != 2UL)
				{
					mqttStatus = MQTT_UNEXPECTED_RESPONSE;
				}
				else
				{
					unsubscribeCallback(((uint16_t)remainingData[0] << 8) + (uint16_t)remainingData[1]);
					mqttStatus = MQTT_SUBSCRIBE_ACK;
				}
			}
			else if ((packetType & MQTT_PACKET_ID_MASK) == MQTT_CONNECT_ACK_PACKET_ID)
			{
				if (remainingLength != 2U)
				{
					mqttStatus = MQTT_UNEXPECTED_RESPONSE;
				}
				if (remainingData[0] != 0U)
				{
					mqttStatus = MQTT_CONNECTION_REFUSED;
				}
				else
				{
					mqttStatus = MQTT_OK;
				}
			}
			else
			{
				mqttStatus = MQTT_OK;
			}

			vPortFree(remainingData);
			return mqttStatus;
		}
	}

	return MQTT_NO_RESPONSE;
}

static uint8_t EncodeRemainingLength(uint32_t remainingLength, uint8_t buffer[4])
{
	uint8_t i = 0U;
	uint8_t encodedByte;

	do
	{
		encodedByte = (uint8_t)(remainingLength % (uint32_t)0x80);
		remainingLength = remainingLength / (uint32_t)0x80;

		// if there are more data to encode, set the top bit of this byte
		if (remainingLength > 0UL)
		{
			encodedByte = encodedByte | 0x80U;
		}
		buffer[i] = encodedByte;
		i++;
	}
	while (remainingLength > 0UL);

	return i;
}

static uint32_t DecodeRemainingLength(uint8_t buffer[4])
{
    uint32_t multiplier = 1UL;
    uint32_t value = 0UL;
    uint8_t i = 0U;
    uint8_t encodedByte;

    do
    {
         encodedByte = buffer[i];
         i++;
         value += (encodedByte & 0x7fU) * multiplier;
         multiplier *= 0x80UL;
    }
    while ((encodedByte & 0x80U) != 0U);

	return value;
}

const char *MqttStatusToText(MqttStatus_t mqttStatus)
{
	switch (mqttStatus)
	{
	case MQTT_OK:
		return "MQTT_OK";

	case MQTT_CONNECTION_REFUSED:
		return "MQTT_CONNECTION_REFUSED";

	case MQTT_TIMEOUT:
		return "MQTT_TIMEOUT";

	case MQTT_NO_RESPONSE:
		return "MQTT_NO_RESPONSE";

	case MQTT_UNEXPECTED_RESPONSE:
		return "MQTT_UNEXPECTED_RESPONSE";

	case MQTT_BAD_PARAMETER:
		return "MQTT_BAD_PARAMETER";

	case MQTT_NO_MEMORY:
		return "MQTT_NO_MEMORY";

	case MQTT_TCP_ERROR:
		return "MQTT_TCP_ERROR";

	case MQTT_SUBSCRIBE_FAILURE:
		return "MQTT_SUBSCRIBE_FAILURE";

	case MQTT_PING_ACK:
		return "MQTT_PING_ACK";

	case MQTT_SUBSCRIBE_ACK:
		return "MQTT_SUBSCRIBE_ACK";

	case MQTT_PUBLISH:
		return "MQTT_PUBLISH";

	default:
		return "UNKNOWN_STATUS";
	}
}
