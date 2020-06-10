#ifndef INC_TOUCH_H_
#define INC_TOUCH_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * Tests if screen si being touched
 *
 * @return true if touched else false
 */
bool TouchIsTouched(void);

/**
 * Calibrates the touch screen asking the user for 3 points
 */
void TouchCalibrate(void);

/**
 * Gets a calibrated touch point in screen coordinates if the screen is being touched
 *
 * @param x Pointer to returned x coordinate of touched point
 * @param y Pointer to returned y coordinate of touched point
 * @return true if touched else false
 *
 */
bool TouchGetCalibratedPoint(int16_t* x, int16_t* y);

#endif
