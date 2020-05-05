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

#include <math.h>
#include <string.h>
#include <ctype.h>
#include "seatalk.h"
#include "stm32f1xx_hal.h"

#define M_PI_F 3.1415926f
#define M_PI_2_F 1.5707963f

static float depth_data;
static float temperature_data;
static float boat_speed_data;
static float trip_data;
static float log_data;
static float speed_over_ground_data;
static int16_t course_over_ground_data;
static float apparent_wind_angle_data;
static float apparent_wind_speed_data;
static float true_wind_speed_data;
static float true_wind_angle_data;
static seatalk_date_t date_data;
static seatalk_time_t gmt_data;
static int16_t wind_direction_magnetic_data;
static int16_t heading_magnetic_data;
static int16_t latitude_degrees_data;
static int16_t longitude_degrees_data;
static float latitude_minutes_data;
static float longitude_minutes_data;
static uint8_t altitude_data;
static int16_t geoidal_separation_data;
static uint8_t signal_quality_data;
static uint8_t hdop_data;
static uint8_t number_of_satellites_data;
static autopilot_state_t autopilot_state;
static autopilot_command_t autopilot_command;

static volatile uint8_t seatalk_messages_in[SEATALK_NUMBER_OF_MESSAGES_IN][SEATALK_MAX_MESSAGE_SIZE + 1];
static volatile uint8_t seatalk_messages_out[SEATALK_NUMBER_OF_MESSAGES_OUT][SEATALK_MAX_MESSAGE_SIZE];
static volatile uint8_t seatalk_byte_to_write;
static volatile uint8_t seatalk_command_bit;
static volatile uint8_t seatalk_transmit_state = TS_SUCCESS;
static uint8_t seatalk_out_next_read_pos;
static uint8_t seatalk_out_next_write_pos;
static uint8_t seatalk_out_space = SEATALK_NUMBER_OF_MESSAGES_OUT;
static uint8_t seatalk_sentence[SEATALK_MAX_MESSAGE_SIZE];
static seatalk_message_handler handler;

static void seatalk_do_read(void);
static void seatalk_do_write(void);
static bool write_seatalk_sentence(uint8_t length, volatile uint8_t* command);

extern TIM_HandleTypeDef htim2;

void seatalk_init(seatalk_message_handler callback)
{	
	handler = callback;

	SEATALK_DATA_WRITE(0U);

	HAL_TIM_Base_Start_IT(&htim2);
}

void seatalk_parse_next_message(void)
{
	static uint8_t i = 0U;
	static uint32_t heading_magnetic_time_received = 0x7fffffffUL;
	static uint32_t speed_over_ground_time_received = 0x7fffffffUL;
	static uint32_t boat_speed_time_received = 0x7fffffffUL;
	static uint32_t apparent_wind_speed_time_received = 0x7fffffffUL;
	static uint32_t apparent_wind_angle_time_received = 0x7fffffffUL;
	float Y;
	float a;
	float b;
	float ground_wind_angle;
	uint8_t seatalk_message_type;
	uint8_t seatalk_message_byte_2;
	uint8_t seatalk_message_byte_3;
	uint8_t seatalk_message_byte_4;
	uint8_t seatalk_message_byte_5;
	uint8_t seatalk_message_byte_6;
	uint8_t seatalk_message_byte_7;
	uint32_t time_s = HAL_GetTick();

	// now look at next seatalk_messages_in in the seatalk_messages_in list to see if there are any new ones ready to processing
	if (seatalk_messages_in[i][0] == MS_READY)
	{
		seatalk_messages_in[i][0] = MS_READING;
		seatalk_message_type = seatalk_messages_in[i][1];
		seatalk_message_byte_2 = seatalk_messages_in[i][2];
		seatalk_message_byte_3 = seatalk_messages_in[i][3];
		seatalk_message_byte_4 = seatalk_messages_in[i][4];
		seatalk_message_byte_5 = seatalk_messages_in[i][5];
		seatalk_message_byte_6 = seatalk_messages_in[i][6];
		seatalk_message_byte_7 = seatalk_messages_in[i][7];

		switch (seatalk_message_type)
		{
		case SEATALK_APPARENT_WIND_ANGLE:
			apparent_wind_angle_data = ((float)((((uint16_t)seatalk_message_byte_3) << 8) + seatalk_message_byte_4)) / 2.0f;
			apparent_wind_angle_time_received = time_s;
			handler(SEATALK_APPARENT_WIND_ANGLE);
			break;

		case SEATALK_APPARENT_WIND_SPEED:
			apparent_wind_speed_data = (float)(seatalk_message_byte_3 & 0x7FU) + (float)(seatalk_message_byte_4 / 10.0f);
			apparent_wind_speed_time_received = time_s;
			handler(SEATALK_APPARENT_WIND_SPEED);
			break;

		case SEATALK_SOG:
			speed_over_ground_data = (seatalk_message_byte_3 + ((uint16_t)(seatalk_message_byte_4) << 8)) / 10.0f;
			speed_over_ground_time_received = time_s;
			handler(SEATALK_SOG);
			break;

		case SEATALK_COG:
			course_over_ground_data = ((int16_t)((seatalk_message_byte_2 >> 4) & 0x03U)) * 90U;
			course_over_ground_data += ((int16_t)(seatalk_message_byte_3 & 0x3FU) * 2);
			course_over_ground_data += ((int16_t)(((seatalk_message_byte_2 >> 4) & 0x0CU) >> 3));
			handler(SEATALK_COG);
			break;

		case SEATALK_HEADING_MAGNETIC_1:
			if ((seatalk_message_byte_5 & 0x0F) == 0x00U)
			{
				autopilot_state = APS_STANDBY;
			}
			else if ((seatalk_message_byte_5 & 0x0F) == 0x02U)
			{
				autopilot_state = APS_AUTO;
			}
			else if ((seatalk_message_byte_5 & 0x0F) == 0x04U)
			{
				autopilot_state = APS_VANE;
			}
			else if ((seatalk_message_byte_5 & 0x0F) == 0x08U)
			{
				autopilot_state = APS_TRACK;
			}
			else
			{
				autopilot_state = APS_UNKNOWN;
			}
			handler(SEATALK_HEADING_MAGNETIC_1);
			break;

		case SEATALK_KEYSTROKE:
			if (seatalk_message_byte_3 == 0x01U && seatalk_message_byte_4 == 0xfeU)
			{
				autopilot_command = AUTOPILOT_COMMAND_AUTO;
			}
			else if (seatalk_message_byte_3 == 0x02U && seatalk_message_byte_4 == 0xfdU)
			{
				autopilot_command = AUTOPILOT_COMMAND_STANDBY;
			}
			else if (seatalk_message_byte_3 == 0x03U && seatalk_message_byte_4 == 0xfcU)
			{
				autopilot_command = AUTOPILOT_COMMAND_TRACK;
			}
			else if (seatalk_message_byte_3 == 0x23U && seatalk_message_byte_4 == 0xdcU)
			{
				autopilot_command = AUTOPILOT_COMMAND_VANE;
			}
			handler(SEATALK_KEYSTROKE);
			break;

		case SEATALK_HEADING_MAGNETIC_2:
			heading_magnetic_data = ((int16_t)((seatalk_message_byte_2 & 0x30U) >> 4)) * 90U +
					(seatalk_message_byte_3 & 0x3FU) * 2U +
					(seatalk_message_byte_2 & 0xC0U ? ((seatalk_message_byte_2 & 0xC0U) == 0xC0U ? 2U : 1U) : 0U);
			heading_magnetic_time_received = time_s;
			handler(SEATALK_HEADING_MAGNETIC_2);
			break;

		case SEATALK_DEPTH:
			depth_data = (float)seatalk_message_byte_4;
			depth_data += ((float)seatalk_message_byte_5) * 256.0f;
			depth_data /= 32.808f;
			handler(SEATALK_DEPTH);
			break;

		case SEATALK_TEMPERATURE:
			temperature_data = (float)seatalk_message_byte_3;
			temperature_data += ((float)seatalk_message_byte_4) * 256.0f;
			temperature_data -= 100.0f;
			temperature_data /= 10.0f;
			handler(SEATALK_TEMPERATURE);
			break;

		case SEATALK_BOATSPEED:
			boat_speed_data = (float)seatalk_message_byte_3;
			boat_speed_data += ((float)seatalk_message_byte_4) * 256.0f;
			boat_speed_data /= 10.0f;
			boat_speed_time_received = time_s;
			handler(SEATALK_BOATSPEED);
			break;

		case SEATALK_TRIPLOG:
			log_data = (float)seatalk_message_byte_3;
			log_data += ((float)seatalk_message_byte_4) * 256.0f;
			log_data += ((float)(seatalk_message_byte_2 >> 4)) * 65536.0f;
			log_data /= 10.0f;
			trip_data = (float)seatalk_message_byte_5;
			trip_data += ((float)seatalk_message_byte_6) * 256.0f;
			trip_data += ((float)(seatalk_message_byte_7 & 0x0fU)) * 65536.0f;
			trip_data /= 100.0f;
			handler(SEATALK_TRIPLOG);
			break;

		case SEATALK_DATE:
			date_data.year = seatalk_message_byte_4;
			date_data.month = seatalk_message_byte_2 >> 4;
			date_data.date = seatalk_message_byte_3;
			handler(SEATALK_DATE);
			break;

		case SEATALK_GMT:
			gmt_data.hour = seatalk_message_byte_4;
			gmt_data.minute = (seatalk_message_byte_3 & 0xFCU) >> 2;
			gmt_data.second = ((seatalk_message_byte_3 & 0x03U) << 4) + ((seatalk_message_byte_2 & 0xf0U) >> 4);
			if (gmt_data.second > 59)
			{
				gmt_data.second = 59;
			}
			handler(SEATALK_GMT);
			break;

		case SEATALK_LATITUDE:
			latitude_degrees_data = (int16_t)seatalk_message_byte_3;
			latitude_minutes_data = seatalk_message_byte_4;
			latitude_minutes_data += ((uint16_t)(seatalk_message_byte_5 & 0x7fU)) << 8;
			latitude_minutes_data /= 100.0f;
			if (seatalk_message_byte_5 & 0x80U)
			{
				latitude_degrees_data =- latitude_degrees_data;
				latitude_minutes_data =- latitude_minutes_data;
			}
			handler(SEATALK_LATITUDE);
			break;

		case SEATALK_LONGITUDE:
			longitude_degrees_data = (int16_t)seatalk_message_byte_3;
			longitude_minutes_data = seatalk_message_byte_4;
			longitude_minutes_data += ((uint16_t)(seatalk_message_byte_5 & 0x7fU)) << 8;
			longitude_minutes_data /= 100.0f;
			if (!(seatalk_message_byte_5 & 0x80U))
			{
				longitude_degrees_data = -longitude_degrees_data;
				longitude_minutes_data = -longitude_minutes_data;
			}
			handler(SEATALK_LONGITUDE);
			break;

		case SEATALK_GPS_INFO:
			if (seatalk_message_byte_2 == 0x57U)
			{
				altitude_data = seatalk_message_byte_6;
				geoidal_separation_data = 16 * (int16_t)seatalk_message_byte_7;
				handler(SEATALK_GPS_INFO);

				if ((seatalk_message_byte_3 & 0x10U) == 0x10U)
				{
					signal_quality_data = seatalk_message_byte_3 & 0x0fU;
					handler(SEATALK_SIGNAL_QUALITY);
				}

				if ((seatalk_message_byte_4 & 0x80U) == 0x80U)
				{
					hdop_data = (seatalk_message_byte_4 & 0x7cU) >> 2;
					handler(SEATALK_HDOP);
				}

				if ((seatalk_message_byte_4 & 0x02U) == 0x02U)
				{
					number_of_satellites_data = ((seatalk_message_byte_3 & 0xe0U) >> 4) + (seatalk_message_byte_4 & 0x01U);
					handler(SEATALK_NUMBER_OF_SATS);
				}
			}
			break;
		}

		seatalk_messages_in[i][0] = MS_DONE;

		// now calculate derived data

		// wind direction calculations
		if (seatalk_message_type == SEATALK_APPARENT_WIND_ANGLE ||
				seatalk_message_type == SEATALK_APPARENT_WIND_SPEED ||
				seatalk_message_type == SEATALK_SOG ||
				seatalk_message_type == SEATALK_HEADING_MAGNETIC_2)
		{
			if (time_s - speed_over_ground_time_received < 10UL &&
					time_s - apparent_wind_speed_time_received < 10UL &&
					time_s - apparent_wind_angle_time_received < 10UL &&
					time_s - heading_magnetic_time_received < 10UL)
			{
				if (speed_over_ground_data < 0.01f)
				{
					ground_wind_angle = apparent_wind_angle_data;
				}
				else
				{
					Y = M_PI_2_F - apparent_wind_angle_data / DEGREES_TO_RADIANS;
					a = apparent_wind_speed_data * cosf(Y);
					if (a == 0.0f)
					{
						ground_wind_angle = M_PI_F;
					}
					else
					{
						b = apparent_wind_speed_data * sinf(Y) - speed_over_ground_data;
						ground_wind_angle = M_PI_2_F - atanf(b / a);
					}
					ground_wind_angle *= DEGREES_TO_RADIANS;
					if (ground_wind_angle < 0.0f)
					{
						ground_wind_angle += 360.0f;
					}
				}

				wind_direction_magnetic_data = ground_wind_angle + heading_magnetic_data;
				if (wind_direction_magnetic_data >= 360)
				{
					wind_direction_magnetic_data -= 360;
				}
				if (wind_direction_magnetic_data < 0)
				{
					wind_direction_magnetic_data += 360;
				}
				handler(SEATALK_MAGNETIC_WIND_DIRECTION);
			}
		}

		// true wind speed/angle and velocity made good calculation
		if (seatalk_message_type == SEATALK_APPARENT_WIND_ANGLE ||
				seatalk_message_type == SEATALK_APPARENT_WIND_SPEED ||
				seatalk_message_type == SEATALK_BOATSPEED)
		{
			if (time_s - boat_speed_time_received < 10UL &&
					time_s - apparent_wind_speed_time_received < 10UL &&
					time_s - apparent_wind_angle_time_received < 10UL)
			{
				if (boat_speed_data < 0.01f)
				{
					true_wind_speed_data = apparent_wind_speed_data;
					true_wind_angle_data = apparent_wind_angle_data;
				}
				else
				{
					Y = M_PI_2_F - apparent_wind_angle_data / DEGREES_TO_RADIANS;
					a = apparent_wind_speed_data * cosf(Y);
					b = apparent_wind_speed_data * sinf(Y) - boat_speed_data;
					true_wind_speed_data = sqrtf(a * a + b * b);
					if (a == 0.0f)
					{
						true_wind_angle_data = M_PI_F;
					}
					else
					{
						true_wind_angle_data = M_PI_2_F - atanf(b / a);
					}
					true_wind_angle_data *= DEGREES_TO_RADIANS;
					if (true_wind_angle_data < 0.0f)
					{
						true_wind_angle_data += 360.0f;
					}
				}
				handler(SEATALK_TRUE_WIND_ANGLE_SPEED);
			}
		}
	}

	i++;
	if (i == SEATALK_NUMBER_OF_MESSAGES_IN)
	{
		i = 0U;
	}
}

void seatalk_send_next_message(void)
{
	static const uint8_t randoms[] = {22U, 13U, 14U, 18U, 23U, 30U, 21U, 37U, 30U};
	static uint8_t next_random_position = 0U;
	static uint32_t next_send_millisecond_time = 0UL;

	if (seatalk_out_space == SEATALK_NUMBER_OF_MESSAGES_OUT)
	{
		return;
	}

	if (HAL_GetTick() < next_send_millisecond_time)
	{
		return;
	}

	if (write_seatalk_sentence((seatalk_messages_out[seatalk_out_next_read_pos][1] + 3U) & 0x0fU, seatalk_messages_out[seatalk_out_next_read_pos]) == true)
	{
		seatalk_out_space++;
		seatalk_out_next_read_pos++;
		if (seatalk_out_next_read_pos == SEATALK_NUMBER_OF_MESSAGES_OUT)
		{
			seatalk_out_next_read_pos = 0U;
		}

		next_send_millisecond_time += 10UL;
	}
	else
	{
		next_send_millisecond_time += (uint32_t)randoms[next_random_position];
		next_random_position++;
		if (next_random_position == sizeof(randoms) / sizeof(uint8_t))
		{
			next_random_position = 0U;
		}
	}
}

void seatalk_queue_message_to_send(uint8_t *message)
{
	if (seatalk_out_space == 0U)
	{
		return;
	}

	(void)memcpy((void *)seatalk_messages_out[seatalk_out_next_write_pos], message, (size_t)((message[1] & 0x0fU) + 3U));

	seatalk_out_space--;
	seatalk_out_next_write_pos++;
	if (seatalk_out_next_write_pos == SEATALK_NUMBER_OF_MESSAGES_OUT)
	{
		seatalk_out_next_write_pos = 0U;
	}
}

static bool write_seatalk_sentence(uint8_t length, volatile uint8_t* command)
{
	uint8_t i;

	if (seatalk_transmit_state < TS_SUCCESS)
	{
		return TS_FAILURE;
	}

	for (i = 0U; i < length; i++)
	{
		if (i == 0U)
		{
			seatalk_command_bit = 1U;
		}
		else
		{
			seatalk_command_bit = 0U;
		}

		seatalk_byte_to_write = command[i];

		seatalk_transmit_state = TS_GO;

		while (seatalk_transmit_state < TS_SUCCESS)
		{
		}

		if (seatalk_transmit_state == TS_FAILURE)
		{
			break;
		}
	}

	return seatalk_transmit_state == TS_SUCCESS;
}

void seatalk_target_waypoint_id_send(char *waypoint_id_data)
{
	uint8_t next_character;

	seatalk_sentence[0] = SEATALK_WAYPOINT_ID;
	seatalk_sentence[1] = 0x05U;
	memset(seatalk_sentence + 2, 0U, (size_t)5);

	//character 1
	next_character = toupper(waypoint_id_data[0]);
	if (next_character == '\0')
	{
		next_character = '0';
	}
	next_character -= 0x30U;
	seatalk_sentence[2] |= next_character & 0x3fU;

	//character 2
	next_character = toupper(waypoint_id_data[1]);
	if (next_character == '\0')
	{
		next_character = '0';
	}
	next_character -= 0x30U;
	seatalk_sentence[2] |= next_character << 6;
	seatalk_sentence[4] |= next_character >> 2;

	//character 3
	next_character = toupper(waypoint_id_data[2]);
	if (next_character == '\0')
	{
		next_character = '0';
	}
	next_character -= 0x30U;
	seatalk_sentence[4] |= next_character << 4;
	seatalk_sentence[6] |= next_character >> 4;

	// char 4
	next_character = toupper(waypoint_id_data[3]);
	if (next_character == '\0')
	{
		next_character = '0';
	}
	next_character -= 0x30U;
	seatalk_sentence[6] |= (next_character & 0x3fU) << 2;

	seatalk_sentence[3] = 0xffU - seatalk_sentence[2];
	seatalk_sentence[5] = 0xffU - seatalk_sentence[4];
	seatalk_sentence[7] = 0xffU - seatalk_sentence[6];

	seatalk_queue_message_to_send(seatalk_sentence);
}

void seatalk_navigate_to_waypoint_info_send(float cross_track_error_data,
		float range_to_destination_data,
		int16_t bearing_to_destination_magnetic_data,
		char direction_to_steer_data)
{
	uint16_t seatalk_cross_track_error;
	int16_t bearing_remainder;
	uint16_t seatalk_distance_to_destination;

	// byte 0 - 0x85
	seatalk_sentence[0] = SEATALK_NAV_TO_WAYPOINT_INFO;

	// byte 1 - X6
	seatalk_cross_track_error = (uint16_t)(cross_track_error_data * 100.0f);
	if (seatalk_cross_track_error > 0x0fffU)
	{
		seatalk_cross_track_error = 0xfffU;
	}
	seatalk_sentence[1] = seatalk_cross_track_error & 0x0fU;
	seatalk_sentence[1] <<= 4;
	seatalk_sentence[1] |= 0x06U;

	// byte 2 - XX
	seatalk_sentence[2] = seatalk_cross_track_error >> 4;

	// byte 3 - VU
	seatalk_sentence[3] = bearing_to_destination_magnetic_data / 90;
	bearing_remainder = bearing_to_destination_magnetic_data - (seatalk_sentence[3] * 90);
	bearing_remainder *= 2;
	seatalk_sentence[3] |= (bearing_remainder & 0x0f) << 4;

	// byte 4 - ZW
	seatalk_sentence[4] = bearing_remainder >> 4;

	if (range_to_destination_data < 10.0f)
	{
		seatalk_distance_to_destination = (uint16_t)(range_to_destination_data * 100.0f);
	}
	else
	{
		seatalk_distance_to_destination = (uint16_t)(range_to_destination_data * 10.0f);
	}
	if (seatalk_distance_to_destination > 0xfffU)
	{
		seatalk_distance_to_destination = 0xfffU;
	}

	seatalk_sentence[4] |= (seatalk_distance_to_destination & 0x0f) << 4;

	// byte 5 - ZZ
	seatalk_sentence[5] = seatalk_distance_to_destination >> 4;

	// byte 6 - YF
	if (range_to_destination_data < 10.0f)
	{
		seatalk_sentence[6] = 0x10U;
	}
	else
	{
		seatalk_sentence[6] = 0x00U;
	}

	if (direction_to_steer_data == 'L')
	{
		seatalk_sentence[6] |= 0x07U;
	}
	else
	{
		seatalk_sentence[6] |= 0x47U;
	}

	if (cross_track_error_data >= 0.3f)
	{
		seatalk_sentence[6] |= 0x08U;
	}

	// byte 7 - 0x00
	seatalk_sentence[7] = 0x00U;

	// byte 8 - yf
	seatalk_sentence[8] = 0xffU - seatalk_sentence[6];

	seatalk_queue_message_to_send(seatalk_sentence);
}

void seatalk_arrival_info_send(char *waypoint_id_data, bool arrival_circle_entered_data, bool perpendicular_passed_data)
{
	uint8_t byte;
	uint8_t i;
	bool end_reached = false;

	seatalk_sentence[0] = SEATALK_ARRIVAL_INFO;
	byte = 0x04U;
	if (arrival_circle_entered_data)
	{
		byte |= 0x40U;
	}
	if (perpendicular_passed_data)
	{
		byte |= 0x20U;
	}
	seatalk_sentence[1] = byte;
	seatalk_sentence[2] = 0x00U;

	for (i = 0U; i < 4U; i++)
	{
		if (end_reached)
		{
			seatalk_sentence[i + 3U] = '0';
			continue;
		}

		if (waypoint_id_data[i] == '\0')
		{
			seatalk_sentence[i + 3U] = '0';
			end_reached = true;
		}
		else
		{
			seatalk_sentence[i + 3U] = toupper(waypoint_id_data[i]);
		}
	}

	seatalk_queue_message_to_send(seatalk_sentence);
}

void seatalk_autopilot_send(autopilot_command_t autopilot_command)
{
	if (autopilot_command == AUTOPILOT_COMMAND_UNKNOWN)
	{
		return;
	}
	seatalk_sentence[0] = SEATALK_KEYSTROKE;
	seatalk_sentence[1] = 0x11U;
	seatalk_sentence[2] = (uint8_t)autopilot_command;
	seatalk_sentence[3] = 0xffU - (uint8_t)autopilot_command;

	seatalk_queue_message_to_send(seatalk_sentence);
}

void seatalk_speed_over_ground_send(float sog)
{
	uint16_t sog_int;

	// byte 0 - 0x52
	seatalk_sentence[0] = SEATALK_SOG;
	seatalk_sentence[1] = 0x01U;

	sog_int = (uint16_t)(sog *= 10.0f);
	seatalk_sentence[2] = (uint8_t)(sog_int & 0x00ffU);
	seatalk_sentence[3] = (uint8_t)((sog_int & 0xff00U) >> 8);

	seatalk_queue_message_to_send(seatalk_sentence);
}

void seatalk_course_over_ground_send(float cog)
{
	uint16_t quadrant;
	uint16_t remainder_1;
	float remainder_2;

	// byte 0 - 0x53
	seatalk_sentence[0] = SEATALK_COG;

	quadrant = (uint8_t)(cog / 90.0f);
	seatalk_sentence[1] = (uint8_t)quadrant << 4;

	remainder_1 = (uint16_t)cog - quadrant * 90U;
	seatalk_sentence[2] = remainder_1 / 2U;

	remainder_2 = cog - ((float)quadrant * 90.0f) - (float)(2U * (remainder_1 / 2U));
	remainder_2 *= 2.0f;

	seatalk_sentence[1] |= (uint8_t)remainder_2 << 6;

	seatalk_queue_message_to_send(seatalk_sentence);
}

float seatalk_depth_data_retrieve(void)
{
	return depth_data;
}

float seatalk_temperature_data_retrieve(void)
{
	return temperature_data;
}

float seatalk_speed_over_ground_data_retrieve(void)
{
	return speed_over_ground_data;
}

int16_t seatalk_course_over_ground_data_retrieve(void)
{
	return course_over_ground_data;
}

float seatalk_boat_speed_data_retrieve(void)
{
	return boat_speed_data;
}

float seatalk_trip_data_retrieve(void)
{
	return trip_data;
}

float seatalk_log_data_retrieve(void)
{
	return log_data;
}

float seatalk_apparent_wind_speed_retrieve(void)
{
	return apparent_wind_speed_data;
}

float seatalk_apparent_wind_angle_retrieve(void)
{
	return apparent_wind_angle_data;
}

float seatalk_true_wind_speed_retrieve(void)
{
	return true_wind_speed_data;
}

float seatalk_true_wind_angle_retrieve(void)
{
	return true_wind_angle_data;
}

seatalk_time_t seatalk_time_retrieve(void)
{
	return gmt_data;
}

seatalk_date_t seatalk_date_retrieve(void)
{
	return date_data;
}

int16_t seatalk_wind_direction_magnetic_retrieve(void)
{
	return wind_direction_magnetic_data;
}

int16_t seatalk_heading_magnetic_retrieve(void)
{
	return heading_magnetic_data;
}

int16_t seatalk_latitude_degrees_retrieve(void)
{
	return latitude_degrees_data;
}

float seatalk_latitude_minutes_retrieve(void)
{
	return latitude_minutes_data;
}

int16_t seatalk_longitude_degrees_retrieve(void)
{
	return longitude_degrees_data;
}

float seatalk_longitude_minutes_retrieve(void)
{
	return longitude_minutes_data;
}

uint8_t seatalk_altitude_retrieve(void)
{
	return altitude_data;
}

uint8_t seatalk_geoidal_separation_retrieve(void)
{
	return geoidal_separation_data;
}

uint8_t seatalk_signal_quality_retrieve(void)
{
	return signal_quality_data;
}

uint8_t seatalk_hdop_retrieve(void)
{
	return hdop_data;
}

uint8_t seatalk_number_of_satellites_retrieve(void)
{
	return number_of_satellites_data;
}

autopilot_state_t seatalk_autopilot_state_retrieve(void)
{
	return autopilot_state;
}

autopilot_command_t seatalk_autopilot_command_retrieve(void)
{
	return autopilot_command;
}

static void seatalk_do_read(void)
{
	static uint8_t receive_tick_count = 0U;
	static uint8_t last_message_character_position_written = 0U;
	static uint8_t parity;
	static seatalk_receiver_state_t receive_state = SEATALK_RS_HAVE_NOTHING;
	static uint8_t byte_to_receive;
	static uint8_t next_message = 0U;
	static seatalk_bus_state_t bus_state = SEATALK_BS_WAITING_FOR_COMMAND;

	switch (receive_state)
	{
		case SEATALK_RS_HAVE_NOTHING:
			// Check for start bit of a received char.
			if (SEATALK_DATA == 0U)
			{
				receive_state = SEATALK_RS_WAIT_HALF_A_BIT;
				receive_tick_count = 0U;
			}
			break;

		case SEATALK_RS_WAIT_HALF_A_BIT:
			receive_tick_count++;
			if (receive_tick_count == 4U)
			{
				if (SEATALK_DATA == 0U)
				{
					receive_tick_count = 0U;
					receive_state = SEATALK_RS_HAVE_STARTBIT;
					byte_to_receive = 0U;
				}
				else
				{
					receive_state = SEATALK_RS_HAVE_NOTHING;
				}
			}
			break;

		default:
			receive_tick_count++;
			if (receive_tick_count == 8U)
			{
				receive_tick_count = 0U;
				receive_state++;
				byte_to_receive >>= 1;
				if (SEATALK_DATA)
				{
					byte_to_receive |= 0x80U;
				}
			}
			break;

		case SEATALK_RS_PARITY:
			receive_tick_count++;
			if (receive_tick_count == 8U)
			{
				receive_tick_count = 0U;
				receive_state++;
				parity = SEATALK_DATA;
			}
			break;

		case SEATALK_RS_WAIT_FOR_STOP:
			receive_tick_count++;
			if (receive_tick_count == 8U)
			{
				switch (bus_state)
				{
					case SEATALK_BS_WAITING_FOR_COMMAND:
						if (parity)
						{
							// move on to next message
							next_message++;
							if (next_message == SEATALK_NUMBER_OF_MESSAGES_IN)
							{
								next_message = 0U;
							}

							// if next message is being read, move on again
							if (seatalk_messages_in[next_message][0] == MS_READING)
							{
								next_message++;
								if (next_message == SEATALK_NUMBER_OF_MESSAGES_IN)
								{
									next_message = 0U;
								}
							}
							seatalk_messages_in[next_message][1] = byte_to_receive;
							bus_state = SEATALK_BS_WAITING_FOR_SIZE;
						}
						break;

					case SEATALK_BS_WAITING_FOR_SIZE:
						if (parity)
						{
							// got an unexpected command message so must be a collision, abandon this message
							bus_state = SEATALK_BS_WAITING_FOR_COMMAND;
						}
						else
						{
							seatalk_messages_in[next_message][2] = byte_to_receive;
							last_message_character_position_written = 2U;
							bus_state = SEATALK_BS_RECEIVING_MESSAGE;
						}
						break;

					case SEATALK_BS_RECEIVING_MESSAGE:
						if (parity)
						{
							// got an unexpected command message so must be a collision, abandon this message
							bus_state = SEATALK_BS_WAITING_FOR_COMMAND;
						}
						else
						{
							last_message_character_position_written++;
							if (last_message_character_position_written == SEATALK_MAX_MESSAGE_SIZE)
							{
								bus_state = SEATALK_BS_WAITING_FOR_COMMAND;
							}
							else
							{
								seatalk_messages_in[next_message][last_message_character_position_written] = byte_to_receive;
								if (last_message_character_position_written == ((seatalk_messages_in[next_message][2]) & 0x0fU) + 3U)
								{
									// mark previous message as ready
									seatalk_messages_in[next_message][0] = MS_READY;
									bus_state = SEATALK_BS_WAITING_FOR_COMMAND;
								}
							}
						}
						break;
				}
				receive_state = SEATALK_RS_HAVE_NOTHING;
			}
			break;
	}
}

static void seatalk_do_write(void)
{
	static uint8_t transmit_tick_count;
	static uint8_t b;
	static uint16_t wait_counter = 0U;

	switch (seatalk_transmit_state)
	{
		case TS_SUCCESS:
		case TS_FAILURE:
			break;

		case TS_GO:
			if (seatalk_command_bit)
			{
				if (SEATALK_DATA == 0U)
				{
					wait_counter = 0U;
					seatalk_transmit_state = TS_FAILURE;
				}
				else
				{
					wait_counter++;
					if (wait_counter == 768U)
					{
						wait_counter = 0U;
						seatalk_transmit_state = TS_STARTBIT;
						transmit_tick_count = 0U;
					}
				}
			}
			else
			{
				seatalk_transmit_state = TS_STARTBIT;
				transmit_tick_count = 0U;
			}
			break;

		case TS_STARTBIT:
			if (transmit_tick_count == 0U)
			{
				SEATALK_DATA_WRITE(1U);
				transmit_tick_count++;
			}
			else
			{
				transmit_tick_count++;
				if (transmit_tick_count == 8U)
				{
					if (SEATALK_DATA != 0U)
					{
						SEATALK_DATA_WRITE(0U);
						seatalk_transmit_state = TS_FAILURE;
					}
					else
					{
						seatalk_transmit_state++;
						transmit_tick_count = 0U;
					}
				}
			}
			break;

		default:
			if (transmit_tick_count == 0U)
			{
				b = !(seatalk_byte_to_write & 0x01u);
				seatalk_byte_to_write >>= 1;
				SEATALK_DATA_WRITE(b);
				transmit_tick_count++;
			}
			else
			{
				transmit_tick_count++;
				if (transmit_tick_count == 8U)
				{
					if (SEATALK_DATA == b)
					{
						SEATALK_DATA_WRITE(0U);
						seatalk_transmit_state = TS_FAILURE;
					}
					else
					{
						seatalk_transmit_state++;
						transmit_tick_count = 0U;
					}
				}
			}
			break;

		case TS_PARITY:
			if (transmit_tick_count == 0U)
			{
				SEATALK_DATA_WRITE(!seatalk_command_bit);
				transmit_tick_count++;
			}
			else
			{
				transmit_tick_count++;
				if (transmit_tick_count == 8U)
				{
					if (SEATALK_DATA != seatalk_command_bit)
					{
						SEATALK_DATA_WRITE(0U);
						seatalk_transmit_state = TS_FAILURE;
					}
					else
					{
						seatalk_transmit_state = TS_STOP;
						transmit_tick_count = 0U;
					}
				}
			}
			break;

		case TS_STOP:
			if (transmit_tick_count == 0U)
			{
				SEATALK_DATA_WRITE(0U);
				transmit_tick_count++;
			}
			else
			{
				transmit_tick_count++;
				if (transmit_tick_count == 8U)
				{
					if (SEATALK_DATA == 0U)
					{
						SEATALK_DATA_WRITE(0U);
						seatalk_transmit_state = TS_FAILURE;
					}
					else
					{
						seatalk_transmit_state = TS_SUCCESS;
					}
				}
			}
			break;
	}
}

void seatalk_timer_interrupt_handler(void)
{
	seatalk_do_read();
	seatalk_do_write();
}
