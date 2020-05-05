/*
MIT License

Copyright (c) John Blaiklock 2020 Boat Data Hub

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef SEATALK_H
#define SEATALK_H

#include <stdint.h>
#include <stdbool.h>

#define DEGREES_TO_RADIANS 					57.259
#define SEATALK_NUMBER_OF_MESSAGES_IN		10
#define SEATALK_NUMBER_OF_MESSAGES_OUT		5
#define SEATALK_MAX_MESSAGE_SIZE 			18

// these are Seatalk message command identifiers
#define SEATALK_DEPTH 						0x00U
#define SEATALK_APPARENT_WIND_ANGLE 		0x10U
#define SEATALK_APPARENT_WIND_SPEED 		0x11U
#define SEATALK_SOG							0x52U
#define SEATALK_COG							0x53U
#define SEATALK_HEADING_MAGNETIC_1			0x84U
#define SEATALK_HEADING_MAGNETIC_2			0x9cU
#define SEATALK_GMT							0x54U
#define	SEATALK_DATE						0x56U
#define SEATALK_DEPTH						0x00U
#define SEATALK_TEMPERATURE					0x27U
#define SEATALK_BOATSPEED					0x20U
#define SEATALK_TRIPLOG						0x25U
#define SEATALK_VARIATION					0x99U
#define SEATALK_LATITUDE					0x50U
#define SEATALK_LONGITUDE					0x51U
#define SEATALK_NAV_TO_WAYPOINT_INFO		0x85U
#define SEATALK_GPS_INFO					0xa5U
#define SEATALK_KEYSTROKE					0x86U
#define SEATALK_ARRIVAL_INFO				0xA2U
#define SEATALK_WAYPOINT_ID					0x82U

// these are user defined
#define SEATALK_USER_DEFINED_MESSAGE_MIN	0xc0U
#define SEATALK_TRUE_WIND_ANGLE_SPEED		SEATALK_USER_DEFINED_MESSAGE_MIN
#define SEATALK_TRUE_WIND_DIRECTION			(SEATALK_USER_DEFINED_MESSAGE_MIN + 1U)
#define SEATALK_MAGNETIC_WIND_DIRECTION		(SEATALK_USER_DEFINED_MESSAGE_MIN + 2U)
#define SEATALK_HEADING_TRUE				(SEATALK_USER_DEFINED_MESSAGE_MIN + 3U)
#define SEATALK_HDOP				        (SEATALK_USER_DEFINED_MESSAGE_MIN + 4U)
#define SEATALK_NUMBER_OF_SATS		        (SEATALK_USER_DEFINED_MESSAGE_MIN + 5U)
#define SEATALK_SIGNAL_QUALITY		        (SEATALK_USER_DEFINED_MESSAGE_MIN + 6U)


#define SEATALK_DATA 						(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5))
#define SEATALK_DATA_WRITE(X) 				(HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, X))

typedef void(*seatalk_message_handler)(uint8_t);

enum message_state_t
{
	MS_DONE = 0,
	MS_READY,
	MS_READING
};

typedef enum
{
	SEATALK_RS_HAVE_NOTHING,
	SEATALK_RS_WAIT_HALF_A_BIT,
	SEATALK_RS_HAVE_STARTBIT,
	SEATALK_RS_PARITY = SEATALK_RS_HAVE_STARTBIT + 8,
	SEATALK_RS_WAIT_FOR_STOP,
	SEATALK_RS_HAVE_BYTE
} seatalk_receiver_state_t;

typedef enum
{
	APS_UNKNOWN,
	APS_STANDBY,
	APS_AUTO,
	APS_VANE,
	APS_TRACK
} autopilot_state_t;

typedef enum
{
	AUTOPILOT_COMMAND_UNKNOWN = 0,
	AUTOPILOT_COMMAND_STANDBY = 2,
	AUTOPILOT_COMMAND_AUTO = 1,
	AUTOPILOT_COMMAND_VANE = 23,
	AUTOPILOT_COMMAND_TRACK = 3,
	AUTOPILOT_COMMAND_MINUS_1 = 5,
	AUTOPILOT_COMMAND_MINUS_10 = 6,
	AUTOPILOT_COMMAND_PLUS_1 = 7,
	AUTOPILOT_COMMAND_PLUS_10 = 8
} autopilot_command_t;

typedef enum
{
	TS_GO,
	TS_STARTBIT,
	TS_PARITY=TS_STARTBIT + 9,
	TS_STOP,
	TS_SUCCESS,
	TS_FAILURE
} seatalk_transmitter_state_t;

typedef enum
{
	SEATALK_BS_WAITING_FOR_COMMAND,
	SEATALK_BS_WAITING_FOR_SIZE,
	SEATALK_BS_RECEIVING_MESSAGE
} seatalk_bus_state_t;

typedef struct
{
	uint8_t year;
	uint8_t month;
	uint8_t date;
} seatalk_date_t;

typedef struct
{
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
} seatalk_time_t;

void seatalk_init(seatalk_message_handler callback);
void seatalk_timer_interrupt_handler(void);
void seatalk_parse_next_message(void);
void seatalk_send_next_message(void);
void seatalk_queue_message_to_send(uint8_t *message);
void seatalk_autopilot_send(autopilot_command_t autopilot_command);
void seatalk_arrival_info_send(char *waypoint_id_data, bool arrival_circle_entered_data, bool perpendicular_passed_data);
void seatalk_target_waypoint_id_send(char *waypoint_id_data);
void seatalk_navigate_to_waypoint_info_send(float cross_track_error_data,
		float range_to_destination_data,
		int16_t bearing_to_destination_magnetic_data,
		char direction_to_steer_data);
void seatalk_speed_over_ground_send(float sog);
void seatalk_course_over_ground_send(float cog);
float seatalk_depth_data_retrieve(void);
float seatalk_temperature_data_retrieve(void);
float seatalk_speed_over_ground_data_retrieve(void);
int16_t seatalk_course_over_ground_data_retrieve(void);
float seatalk_boat_speed_data_retrieve(void);
float seatalk_trip_data_retrieve(void);
float seatalk_log_data_retrieve(void);
float seatalk_apparent_wind_speed_retrieve(void);
float seatalk_apparent_wind_angle_retrieve(void);
float seatalk_true_wind_speed_retrieve(void);
float seatalk_true_wind_angle_retrieve(void);
seatalk_time_t seatalk_time_retrieve(void);
seatalk_date_t seatalk_date_retrieve(void);
int16_t seatalk_wind_direction_magnetic_retrieve(void);
int16_t seatalk_heading_magnetic_retrieve(void);
int16_t seatalk_latitude_degrees_retrieve(void);
float seatalk_latitude_minutes_retrieve(void);
int16_t seatalk_longitude_degrees_retrieve(void);
float seatalk_longitude_minutes_retrieve(void);
uint8_t seatalk_altitude_retrieve(void);
uint8_t seatalk_geoidal_separation_retrieve(void);
uint8_t seatalk_signal_quality_retrieve(void);
uint8_t seatalk_hdop_retrieve(void);
uint8_t seatalk_number_of_satellites_retrieve(void);
autopilot_state_t seatalk_autopilot_state_retrieve(void);
autopilot_command_t seatalk_autopilot_command_retrieve(void);

#endif
