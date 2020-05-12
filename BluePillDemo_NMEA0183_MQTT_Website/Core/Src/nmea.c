#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include "nmea.h"
#include "buffered_serial2.h"
#include "printf.h"
#include "cmsis_os.h"

typedef struct
{
	const char message_header[4];
	nmea_message_type_t message_type;
} nmea_message_type_map_t;

static const nmea_receive_message_details_t *receive_message_details[NMEA_MAXIMUM_RECEIVE_MESSAGE_DETAILS];
static char message_data_to_read_buffer[NMEA_MAX_MESSAGE_LENGTH + 1];
static uint8_t decode(char *buffer);
static const nmea_receive_message_details_t *get_receive_message_details(nmea_message_type_t message_type);
static char *my_strtok(char *s1, const char *delimit);
static nmea_message_type_t get_message_type_from_header(char *header);
static uint8_t count_commas(const char *text);
static uint32_t my_xtoi(char *hex_string);
static bool verify_checksum(char *message);
static uint16_t receive_data(uint16_t buffer_length, uint8_t *data);
static bool check_received_message(char *message_data, uint8_t min_commas, uint8_t max_commas);

// map of nmea message headers to types - receive message types only
static const nmea_message_type_map_t nmea_message_type_map[] = {
		{"RMC", nmea_message_RMC},
		{"DPT", nmea_message_DPT}
		// todo add more NMEA0183 message types here
		};

#define NMEA_MESSAGE_MAP_ENTRIES (sizeof(nmea_message_type_map) / sizeof(nmea_message_type_map_t))

static uint8_t decode(char *buffer)
{
    uint8_t bytes_used = 0U;
    uint8_t next_message_next_position = 0U;
    char next_byte;
    uint8_t next_byte_position = 0U;
    bool in_message = false;
    char next_message[NMEA_MAX_MESSAGE_LENGTH + 1];
    nmea_message_type_t message_type;
    nmea_receive_message_callback_t receive_message_callback;
    const nmea_receive_message_details_t *receive_message_details;

    while (next_byte_position < NMEA_MAX_MESSAGE_LENGTH)
    {
        next_byte = buffer[next_byte_position];

        if (next_byte == 0)
        {
            break;
        }

        if (!in_message)
        {
            if (next_byte == '$' || next_byte == '!' )
            {
                in_message = true;
                next_message[0] = next_byte;
                next_message_next_position = 1U;
            }
            else
            {
                bytes_used++;
            }
        }
        else
        {
            next_message[next_message_next_position] = next_byte;
            next_message_next_position++;

            if (next_byte == '\n')
            {
                in_message = false;
                next_message[next_message_next_position] = 0;

                bytes_used += (uint8_t)strlen(next_message);

                if (strlen(next_message) >= (size_t)(NMEA_MIN_MESSAGE_LENGTH))
                {
                    if (verify_checksum(next_message))
                    {
                        message_type = get_message_type_from_header(&next_message[3]);

                        receive_message_details = get_receive_message_details(message_type);
                        if (receive_message_details != NULL)
                        {
                        	receive_message_callback = receive_message_details->receive_message_callback;

                            if (receive_message_callback)
                            {
                            	receive_message_callback(next_message);
                            }
                        }
                    }
                }
            }
        }

        next_byte_position++;
    }

    if (next_byte_position == NMEA_MAX_MESSAGE_LENGTH && bytes_used == 0U)
    {
        bytes_used = NMEA_MAX_MESSAGE_LENGTH;
    }

    return bytes_used;
}

void nmea_process(void)
{
    uint8_t bytes_to_read;
    uint16_t bytes_read;
    uint8_t unread_length;
    uint8_t bytes_used;

	do
	{
		unread_length = (uint8_t)strlen(&message_data_to_read_buffer[0]);
		bytes_to_read = NMEA_MAX_MESSAGE_LENGTH - unread_length;
		bytes_read = receive_data(bytes_to_read, (uint8_t*)&message_data_to_read_buffer[unread_length]);
		if (bytes_read > 0U)
		{
			message_data_to_read_buffer[unread_length + bytes_read] = 0;
			bytes_used = decode(&message_data_to_read_buffer[0]);

			if (bytes_used > 0U)
			{
				(void)memmove(&message_data_to_read_buffer[0],
						&message_data_to_read_buffer[bytes_used],
						(size_t)((NMEA_MAX_MESSAGE_LENGTH + 1) - bytes_used));
			}
		}
	} while (bytes_to_read == bytes_read);
}

static nmea_message_type_t get_message_type_from_header(char *header)
{
	for (uint8_t i = 0U; i < NMEA_MESSAGE_MAP_ENTRIES; i++)
	{
		if (strncmp(header, nmea_message_type_map[i].message_header, (size_t)3) == 0)
		{
			return nmea_message_type_map[i].message_type;
		}
	}

    return nmea_message_min;
}

static uint32_t my_xtoi(char *hex_string)
{
	uint32_t i = 0UL;

	while (*hex_string)
	{
		char c = toupper(*hex_string++);
		if ((c < '0') || (c > 'F') || ((c > '9') && (c < 'A')))
		{
			break;
		}
		c -= '0';

		if (c > 9)
		{
			c -= 7;
		}

		i = (i << 4) + (uint32_t)c;
	}

	return i;
}

static bool verify_checksum(char *message)
{
    size_t length;
    uint8_t calculated_checksum = 0U;
    uint32_t read_checksum;
    char *checksum_start;

    length = strlen(message);

    if (length < (size_t)12)
    {
        return false;
    }

    if ((checksum_start = strchr(message, '*')) == NULL)
    {
        return false;
    }

    message++;
    while (*message != '*')
    {
        calculated_checksum ^= *message++;
    }

    read_checksum = my_xtoi(checksum_start + 1);

    return ((uint32_t)calculated_checksum == read_checksum);
}

static uint8_t count_commas(const char *text)
{
    uint8_t comma_count = 0U;

    while (*text)
    {
        if (*text == ',')
        {
        	comma_count++;
        }
        text++;
    }

    return comma_count;
}

static char *my_strtok(char *s1, const char *delimit)
{
    static char *last_token = NULL;
    char *tmp;

    if (s1 == NULL)
    {
        s1 = last_token;
        if (s1 == NULL)
        {
            return NULL;
        }
    }
    else
    {
        s1 += strspn(s1, delimit);
    }

    tmp = strpbrk(s1, delimit);
    if (tmp)
    {
        *tmp = 0;
        last_token = tmp + 1;
    }
    else
    {
    	last_token = NULL;
    }

    return s1;
}

uint8_t nmea_count_set_bits(uint32_t n, uint8_t start_bit, uint8_t length)
{
    uint8_t count = 0U;
    uint32_t mask;
    uint8_t bit;

    mask = 1UL;
    for (bit = 0U; bit < start_bit + length; bit++)
    {
        if (bit >= start_bit && (n & mask) > 0UL)
        {
        	count++;
        }
        mask <<= 1;
    }

    return count;
}

void nmea_enable_receive_message(const nmea_receive_message_details_t *nmea_receive_message_details)
{
    uint16_t i;

    if (nmea_receive_message_details->message_type >= nmea_message_max)
    {
        return;
    }

    for (i = 0U; i < NMEA_MAXIMUM_RECEIVE_MESSAGE_DETAILS; i++)
    {
        if (receive_message_details[i] != NULL &&
                receive_message_details[i]->message_type == nmea_receive_message_details->message_type)
        {
            return;
        }
    }

    for (i = 0U; i < NMEA_MAXIMUM_RECEIVE_MESSAGE_DETAILS; i++)
    {
        if (receive_message_details[i] == NULL)
        {
            break;
        }
    }

    if (i == NMEA_MAXIMUM_RECEIVE_MESSAGE_DETAILS)
    {
        return;
    }

    receive_message_details[i] = nmea_receive_message_details;
}


static const nmea_receive_message_details_t *get_receive_message_details(nmea_message_type_t message_type)
{
   uint16_t i;

   for (i = 0U; i < NMEA_MAXIMUM_RECEIVE_MESSAGE_DETAILS; i++)
   {
       if (receive_message_details[i] != NULL)
       {
           if (receive_message_details[i]->message_type == message_type)
           {
               return receive_message_details[i];
           }
       }
   }

   return NULL;
}

static bool check_received_message(char *message_data, uint8_t min_commas, uint8_t max_commas)
{
	size_t length = strlen(message_data);
	uint8_t comma_count = count_commas(message_data);;

	if (length < (size_t)NMEA_MIN_MESSAGE_LENGTH || message_data[length - (size_t)2] != '\r')
	{
		return false;
	}

	if (comma_count < min_commas || comma_count > max_commas)
	{
		return false;
	}

    if (strlen(my_strtok(message_data, ",")) != (size_t)6)
    {
        return false;
    }

	return true;
}

nmea_error_t nmea_decode_RMC(char *message_data, nmea_message_data_RMC_t *result)
{
    char *next_token;
    uint8_t comma_count = count_commas(message_data);
    uint32_t data_available = 0UL;

    if (!check_received_message(message_data, 11U, 13U))
    {
    	return nmea_error_message;
    }

    next_token = my_strtok(NULL, ",");
    if (strlen(next_token) > (size_t)0)
    {
        result->utc.hours = (uint8_t)((float)atof(next_token) / 10000.0f);
        result->utc.minutes = (uint8_t)(((float)atof(next_token) - (float)(result->utc.hours * 10000.0f)) / 100.0f);
        result->utc.seconds = (float)atof(next_token) - (float)(result->utc.hours * 10000.0f) - (float)(result->utc.minutes * 100.0f);
        data_available |= NMEA_RMC_UTC_PRESENT;
    }

    next_token=my_strtok(NULL, ",");
    if (strlen(next_token) > (size_t)0)
    {
        result->status = *next_token;
        data_available |= NMEA_RMC_STATUS_PRESENT;
    }

    next_token = my_strtok(NULL, ",");
    if (strlen(next_token) > (size_t)0)
    {
        result->latitude = atof(next_token);
        data_available |= NMEA_RMC_LATITUDE_PRESENT;
    }

    next_token = my_strtok(NULL, ",");
    if (*next_token == 'S')
    {
        result->latitude = -result->latitude;
    }

    next_token = my_strtok(NULL, ",");
    if (strlen(next_token) > (size_t)0)
    {
        result->longitude = atof(next_token);
        data_available |= NMEA_RMC_LONGITUDE_PRESENT;
    }

    next_token = my_strtok(NULL, ",");
    if (*next_token == 'W')
    {
        result->longitude = -result->longitude;
    }

    next_token = my_strtok(NULL, ",");
    if (strlen(next_token) > (size_t)0)
    {
        result->SOG = (float)atof(next_token);
        data_available |= NMEA_RMC_SOG_PRESENT;
    }

    next_token = my_strtok(NULL, ",");
    if (strlen(next_token) > (size_t)0)
    {
        result->COG = (float)atof(next_token);
        data_available |= NMEA_RMC_COG_PRESENT;
    }

    next_token = my_strtok(NULL, ",");
    if (strlen(next_token) > (size_t)0)
    {
        result->date.date = (uint8_t)(atoi(next_token) / 10000);
        result->date.month = (uint8_t)((atoi(next_token) - (uint32_t)(result->date.date) * 10000UL) / 100UL);
        result->date.year = (uint16_t)(atoi(next_token) - (uint32_t)(result->date.date) * 10000UL - (uint32_t)(result->date.month) * 100UL);
        result->date.year += 2000U;
        data_available |= NMEA_RMC_DATE_PRESENT;
    }

    next_token = my_strtok(NULL, ",");
    if (strlen(next_token) > (size_t)0)
    {
        result->magnetic_variation = (float)atof(next_token);
        data_available |= NMEA_RMC_MAG_VARIATION_PRESENT;
    }

    next_token = my_strtok(NULL, ",*\r");
    if (strlen(next_token) > (size_t)0)
    {
        result->magnetic_variation_direction = *next_token;
        data_available |= NMEA_RMC_MAG_DIRECTION_PRESENT;
    }

    if (comma_count >= 12U)
    {
        next_token = my_strtok(NULL, ",*\r");
        if (strlen(next_token) > 0U)
        {
            result->mode = *next_token;
            data_available |= NMEA_RMC_MODE_PRESENT;
        }

        if (comma_count == 13U)
        {
            next_token = my_strtok(NULL, "*\r");
            if (strlen(next_token) > (size_t)0)
            {
                result->navigation_status = *next_token;
                data_available |= NMEA_RMC_NAV_STATUS_PRESENT;
            }
        }
    }

    result->data_available = data_available;

    return nmea_error_none;
}

nmea_error_t nmea_decode_DPT(char *message_data, nmea_message_data_DPT_t *result)
{
    char *next_token;
    uint8_t comma_count = count_commas(message_data);
    uint32_t data_available = 0UL;

    if (!check_received_message(message_data, 2U, 3U))
    {
    	return nmea_error_message;
    }

    /* depth */
    next_token = my_strtok(NULL, ",");
    if(strlen(next_token) > 0)
    {
    	result->depth = (float)atof(next_token);
    	data_available |= NMEA_DPT_DEPTH_PRESENT;
    }

    /* depthOffset */
    next_token = my_strtok(NULL, ",*\r");
    if(strlen(next_token) > 0)
    {
    	result->depth_offset = (float)atof(next_token);
    	data_available |= NMEA_DPT_DEPTH_OFFSET_PRESENT;
    }

    if (comma_count == 3)
    {
        /* max range */
    	next_token = my_strtok(NULL, "*\r");
        if(strlen(next_token) > 0)
        {
        	result->depth_maximum_range = (float)atof(next_token);
        	data_available |= NMEA_DPT_DEPTH_MAX_RANGE_PRESENT;
        }
    }

    result->data_available = data_available;

    return nmea_error_none;
}

// todo add more NMEA0183 message decoders here

static uint16_t receive_data(uint16_t buffer_length, uint8_t *data)
{
	uint16_t bytes_read = serial2_read_data(buffer_length, data);


	return bytes_read;
}
