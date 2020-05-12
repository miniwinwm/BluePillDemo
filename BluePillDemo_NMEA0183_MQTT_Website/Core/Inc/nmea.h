#ifndef NMEA_H
#define NMEA_H

#include <stdint.h>
#include <stdbool.h>

#define NMEA_MAXIMUM_RECEIVE_MESSAGE_DETAILS           		6U
#define NMEA_MAX_MESSAGE_LENGTH     						82U
#define NMEA_MIN_MESSAGE_LENGTH     						9U

#define NMEA_RMC_UTC_PRESENT            					0x00000001UL
#define NMEA_RMC_STATUS_PRESENT         					0x00000002UL
#define NMEA_RMC_LATITUDE_PRESENT      						0x00000004UL
#define NMEA_RMC_LONGITUDE_PRESENT      					0x00000008UL
#define NMEA_RMC_SOG_PRESENT            					0x00000010UL
#define NMEA_RMC_COG_PRESENT            					0x00000020UL
#define NMEA_RMC_DATE_PRESENT          	 					0x00000040UL
#define NMEA_RMC_MAG_VARIATION_PRESENT  					0x00000080UL
#define NMEA_RMC_MAG_DIRECTION_PRESENT  					0x00000100UL
#define NMEA_RMC_MODE_PRESENT           					0x00000200UL
#define NMEA_RMC_NAV_STATUS_PRESENT     					0x00000400UL
#define NMEA_DPT_DEPTH_PRESENT                				0x00000001UL
#define NMEA_DPT_DEPTH_OFFSET_PRESENT         				0x00000002UL
#define NMEA_DPT_DEPTH_MAX_RANGE_PRESENT      				0x00000004UL
// todo add more NMEA0183 message fields here

typedef struct
{
    uint8_t hours;          /* 0-23 */
    uint8_t minutes;        /* 0-59 */
    float seconds;        	/* 0-59.9999 */
} nmea_utc_time_t;

typedef struct
{
    uint8_t date;           /* 1 to 31 */
    uint8_t month;          /* 1 to 12 */
    uint16_t year;          /* 4 digit form */
} nmea_date_t;

typedef enum
{
    nmea_error_none,
	nmea_error_param,
    nmea_error_message,
	nmea_error_overflow,
} nmea_error_t;

typedef enum
{
    nmea_message_min,            /* must be first value */
    nmea_message_RMC,
    nmea_message_DPT,
// todo add more NMEA0183 message types here

    nmea_message_max             /* must be last value */
} nmea_message_type_t;

typedef void (*nmea_receive_message_callback_t)(char *message);

typedef struct
{
    nmea_message_type_t message_type;
    nmea_receive_message_callback_t receive_message_callback;
} nmea_receive_message_details_t;

typedef struct
{
    char magnetic_variation_direction;
    char status;
    uint32_t data_available;
    nmea_utc_time_t utc;
    char mode;
    char navigation_status;
    float latitude;
    float longitude;
    float SOG;
    float COG;
    nmea_date_t date;
    float magnetic_variation;
} nmea_message_data_RMC_t;

typedef struct
{
    uint32_t data_available;
    float depth;
    float depth_offset;
    float depth_maximum_range;
} nmea_message_data_DPT_t;
// todo add more NMEA0183 message data structures here

void nmea_enable_receive_message(const nmea_receive_message_details_t *nmea_receive_message_details);
void nmea_process(void);
uint8_t nmea_count_set_bits(uint32_t n, uint8_t start_bit, uint8_t length);
nmea_error_t nmea_decode_RMC(char *message_data, nmea_message_data_RMC_t *result);
nmea_error_t nmea_decode_DPT(char *message_data, nmea_message_data_DPT_t *result);
// todo add more NMEA0183 message decoder function prototypes here

#endif
