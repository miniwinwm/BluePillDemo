#ifndef INC_MQTT_H_
#define INC_MQTT_H_

#include <stdint.h>
#include <stdbool.h>

#define MQTT_CONNECT_REQ_PACKET_ID		0x10U
#define MQTT_CONNECT_ACK_PACKET_ID		0x20U
#define MQTT_PUBLISH_PACKET_ID			0x30U
#define MQTT_SUBSCRIBE_REQ_PACKET_ID	0x80U
#define MQTT_SUBSCRIBE_ACK_PACKET_ID	0x90U
#define MQTT_UNSUBSCRIBE_REQ_PACKET_ID	0xa0U
#define MQTT_UNSUBSCRIBE_ACK_PACKET_ID	0xb0U
#define MQTT_PING_REQ_PACKET_ID			0xc0U
#define MQTT_PING_RESP_PACKET_ID		0xd0U
#define MQQT_DISCONNECT_PACKET_ID		0xe0
#define MQTT_PACKET_ID_MASK				0xf0U

typedef enum
{
	// ok statuses
	MQTT_OK = 0,
	MQTT_NO_RESPONSE = 1,
	MQTT_PING_ACK = 2,
	MQTT_SUBSCRIBE_ACK = 3,
	MQTT_PUBLISH = 4,

	// error statuses
	MQTT_CONNECTION_REFUSED = -1,
	MQTT_TIMEOUT = -2,
	MQTT_UNEXPECTED_RESPONSE = -3,
	MQTT_BAD_PARAMETER = -4,
	MQTT_NO_MEMORY = -5,
	MQTT_TCP_ERROR = -6,
	MQTT_SUBSCRIBE_FAILURE = -7
} MqttStatus_t;

typedef void (*PublishCallback_t)(const char *topic, uint8_t topicLength, const uint8_t *payload, uint32_t payloadLength);
typedef void (*PingResponseCallback_t)(void);
typedef void (*SubscribeResponseCallback_t)(uint16_t packetIdentifier, bool success);
typedef void (*UnsubscribeResponseCallback_t)(uint16_t packetIdentifier);

void MqttSetPublishCallback(PublishCallback_t callback);
void MqttSetPingResponseCallback(PingResponseCallback_t callback);
void MqttSetSubscribeResponseCallback(SubscribeResponseCallback_t callback);
void MqttSetUnsubscribeResponseCallback(UnsubscribeResponseCallback_t callback);

MqttStatus_t MqttConnect(char *clientId, char *username, char *password, uint16_t keepAlive, uint32_t timeoutMs);
MqttStatus_t MqttPing(uint32_t timeoutMs);
MqttStatus_t MqttPublish(char *topic, uint8_t *payload, uint32_t payloadLength, bool retain, uint32_t timeoutMs);
MqttStatus_t MqttSubscribe(char *topic, uint16_t packetIdentifier, uint32_t timeoutMs);
MqttStatus_t MqttUnsubscribe(char *topic, uint16_t packetIdentifier, uint32_t timeoutMs);
MqttStatus_t MqttDisconnect(uint32_t timeoutMs);
MqttStatus_t MqttHandleResponse(uint32_t timeoutMs);
const char *MqttStatusToText(MqttStatus_t mqttStatus);

#endif
