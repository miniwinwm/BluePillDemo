#ifndef INC_GRAPHICS_H_
#define INC_GRAPHICS_H_

#include <ili9341.h>
#include <stdint.h>

#define GRAPHICS_STANDARD_CHARACTER_HEIGHT 		9
#define GRAPHICS_STANDARD_CHARACTER_WIDTH 		5
#define GRAPHICS_LARGE_CHARACTER_HEIGHT 		16
#define GRAPHICS_LARGE_CHARACTER_WIDTH 			11
#define GRAPHICS_MAX_SHAPE_CORNERS				10U

/**
 * Initialize the LCD module. Call once and first.
 */
void GraphicsInit(void);

/**
 * Plot a single pixel using the provided colour.
 *
 * @param x X coordinate
 * @param y Y coordinate
 * @param colour The colour in 565 format
 * @note Clipped as required
 */
void GraphicsPixel(int16_t x, int16_t y, colour_t colour);

/**
 * Clear the whole display to the provided colour.
 *
 * @param colour The colour in 565 format
 */
void GraphicsClear(colour_t colour);

/**
 * Plot a vertical line using the provided colour.
 *
 * @param x X coordinate of line
 * @param yStart Start Y coordinate of line
 * @param yEnd End Y coordinate of line
 * @param colour The colour in 565 format
 * @note Clipped as required
 */
void GraphicsVline(int16_t x, int16_t yStart, int16_t yEnd, colour_t colour);

/**
 * Plot a horizontal line using the provided colour.
 *
 * @param xStart Start X coordinate of line
 * @param xEnd End X coordinate of line
 * @param y Y coordinate of line
 * @param colour The colour in 565 format
 * @note Clipped as required
 */
void GraphicsHline(int16_t xStart, int16_t xEnd, int16_t y, colour_t colour);

/**
 * Plot an empty rectangle using the provided colour.
 *
 * @param x Left edge coordinate
 * @param y Top edge coordinate
 * @param width Width of rectangle
 * @param height Height of rectangle
 * @param colour The colour in 565 format
 * @note Clipped as required
 */
void GraphicsRectangle(int16_t x, int16_t y, uint16_t width, uint16_t height, colour_t colour);

/**
 * Plot a standard single horizontal character using the provided colour.
 *
 * @param x Left coordinate of character box
 * @param y Top coordinate of character box
 * @param c The ASCII character
 * @param colour The colour in 565 format
 * @note Clipped as required
 */
void GraphicsStandardCharacter(int16_t x, int16_t y, char c, colour_t colour);

/**
 * Plot a large single horizontal character using the provided colour.
 *
 * @param x Left coordinate of character box
 * @param y Top coordinate of character box
 * @param c The ASCII character
 * @param colour The colour in 565 format
 * @note Clipped as required
 */
void GraphicsLargeCharacter(int16_t x, int16_t y, char c, colour_t colour);

/**
 * Plot a standard single vertical using the provided colour.
 *
 * @param x Left coordinate of character box
 * @param y Top coordinate of character box
 * @param c The ASCII character
 * @param colour The colour in 565 format
 * @note Clipped as required
 */
void GraphicsStandardCharacterVert(int16_t x, int16_t y, char c, colour_t colour);

/**
 * Plot a large single vertical using the provided colour.
 *
 * @param x Left coordinate of character box
 * @param y Top coordinate of character box
 * @param c The ASCII character
 * @param colour The colour in 565 format
 * @note Clipped as required
 */
void GraphicsLargeCharacterVert(int16_t x, int16_t y, char c, colour_t colour);

/**
 * Plot a horizontal string of standard characters using the provided colour.
 *
 * @param x Left coordinate of first character box
 * @param y Top coordinate of first character box
 * @param s Null terminated string of ASCII characters
 * @param colour The colour in 565 format
 * @note Clipped as required
 * @note The length of the string in pixels is (uint16_t)strlen(s) * (GRAPHICS_STANDARD_CHARACTER_WIDTH + 1U)
 */
void GraphicsStandardString(int16_t x, int16_t y, const char *s, colour_t colour);

/**
 * Plot a horizontal string of large characters using the provided colour.
 *
 * @param x Left coordinate of first character box
 * @param y Top coordinate of first character box
 * @param s Null terminated string of ASCII characters
 * @param colour The colour in 565 format
 * @note Clipped as required
 * @note The length of the string in pixels is (uint16_t)strlen(s) * (GRAPHICS_LARGE_CHARACTER_WIDTH + 1U)
 */
void GraphicsLargeString(int16_t x, int16_t y, const char *s, colour_t colour);

/**
 * Plot a vertical string of standard characters using the provided colour.
 *
 * @param x Left coordinate of first character box
 * @param y Top coordinate of first character box
 * @param s Null terminated string of ASCII characters
 * @param colour The colour in 565 format
 * @note Clipped as required
 * @note The length of the string in pixels is (uint16_t)strlen(s) * (GRAPHICS_STANDARD_CHARACTER_WIDTH + 1U)
 */
void GraphicsStandardStringVert(int16_t x, int16_t y, const char *s, colour_t colour);

/**
 * Plot a vertical string of large characters using the provided colour.
 *
 * @param x Left coordinate of first character box
 * @param y Top coordinate of first character box
 * @param s Null terminated string of ASCII characters
 * @param colour The colour in 565 format
 * @note Clipped as required
 * @note The length of the string in pixels is (uint16_t)strlen(s) * (GRAPHICS_LARGE_CHARACTER_WIDTH + 1U)
 */
void GraphicsLargeStringVert(int16_t x, int16_t y, const char *s, colour_t colour);

/**
 * Plot an arbitrary line
 *
 * @param x1 X coordinate of first point
 * @param y1 Y coordinate of first point
 * @param x2 X coordinate of second point
 * @param y2 Y coordinate of second point
 * @param colour The colour in 565 format
 * @note Clipped as required
 */
void GraphicsLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, colour_t colour);

/**
 * Plot an empty circle
 *
 * @param x X coordinate of circle centre
 * @param x Y coordinate of circle centre
 * @param radius Radius of circle
 * @param colour The colour in 565 format
 * @note Clipped as required
 */
void GraphicsCircle(int16_t x, int16_t y, uint16_t radius, colour_t colour);

/**
 * Plot a monochrome bitmap
 *
 * @param x Left coordinate of bounding box
 * @param y Top coordinate of bounding box
 * @param width Width of bitmap
 * @param height Height of bitmap
 * @param fgColour The foreground colour in 565 format
 * @param bgColour The backgroundground colour in 565 format
 * @param imageData Byte array of image data 8 pixels per byte
 * @note NOT CLIPPED. Image must be completely on screen
 */
void GraphicsMonochromeBitmap(int16_t x, int16_t y, uint16_t width, uint16_t height, colour_t fgColour, colour_t bgColour, const uint8_t *imageData);

/**
 * Plot a colour bitmap
 *
 * @param x Left coordinate of bounding box
 * @param y Top coordinate of bounding box
 * @param width Width of bitmap
 * @param height Height of bitmap
 * @param imageData Byte array of image data 2 bytes per pixel in 565 format
 * @note NOT CLIPPED. Image must be completely on screen
 */
void GraphicsColourBitmap(int16_t x, int16_t y, uint16_t width, uint16_t height, const uint8_t *imageData);

/**
 * Plot a filled empty circle
 *
 * @param x X coordinate of circle centre
 * @param x Y coordinate of circle centre
 * @param radius Radius of circle
 * @param colour The colour in 565 format
 * @note Clipped as required
 */
void GraphicsFilledCircle(int16_t x, int16_t y, uint16_t radius, colour_t colour);

/**
 * Plot a filled empty rectangle using the provided colour.
 *
 * @param x Left edge coordinate
 * @param y Top edge coordinate
 * @param width Width of rectangle
 * @param height Height of rectangle
 * @param colour The colour in 565 format
 * @note Clipped as required
 */
void GraphicsFilledRectangle(int16_t x, int16_t y, uint16_t width, uint16_t height, colour_t colour);

/**
 * Rotate a polygon's data
 *
 * @param corners Number of corners to polygon
 * @param xArray Array of X coordinates of polygon's corners
 * @param yArray Array of Y coordinates of polygon's corners
 * @param angleDegrees Angle in degrees to rotate data
 * @note This routine does not plot anything
 */
void GraphicsRotateShape(uint8_t corners, int16_t *xArray, int16_t *yArray, int16_t angleDegrees);

/**
 * Scale a polygon's data
 *
 * @param corners Number of corners to polygon
 * @param xArray Array of X coordinates of polygon's corners
 * @param yArray Array of Y coordinates of polygon's corners
 * @param scaleX Scale x 10 to scale the X coordinates, i.e. 10 is unity scaling
 * @param scaleY Scale x 10 to scale the Y coordinates, i.e. 10 is unity scaling
 * @note This routine does not plot anything
 */
void GraphicsScaleShape(uint8_t corners, int16_t *xArray, int16_t *yArray, int16_t scaleX, int16_t scaleY);

/**
 * Draw a polygon outline
 *
 * @param corners Number of corners to polygon
 * @param xArray Array of X coordinates of polygon's corners
 * @param yArray Array of Y coordinates of polygon's corners
 * @param xOffset Amount to offset each X coordinate
 * @param yOffset Amount to offset each Y coordinate
 * @param colour The colour in 565 format
 * @note Clipped as required
 */
void GraphicsDrawShape(uint8_t corners, const int16_t *xArray, const int16_t *yArray, int16_t xOffset, int16_t yOffset, colour_t colour);

/**
 * Draw a filled polygon
 *
 * @param corners Number of corners to polygon
 * @param xArray Array of X coordinates of polygon's corners
 * @param yArray Array of Y coordinates of polygon's corners
 * @param xOffset Amount to offset each X coordinate
 * @param yOffset Amount to offset each Y coordinate
 * @param colour The colour in 565 format
 * @note Clipped as required
 * @note GRAPHICS_MAX_SHAPE_CORNERS must be set in this file to the maximum number of corners to any polygon for which this function is used
 */
void GraphicsDrawFilledShape(uint8_t corners, const int16_t *xArray, const int16_t *yArray, int16_t x, int16_t y, colour_t colour);

/**
 * Draw an arc
 *
 * @param x X coordinate of centre of circle that the arc is part of
 * @param y Y coordinate of centre of circle that the arc is part of
 * @param startAngle The start angle of the arc in degrees. 0 is straight up increasing clockwise
 * @param endAngle The end angle of the arc in degrees. 0 is straight up increasing clockwise
 * @param colour The colour in 565 format
 * @note Clipped as required
 */
void GraphicsArc(int16_t x, int16_t y, int16_t radius, int16_t startAngle, int16_t endAngle, colour_t colour);

/**
 * Draw an empty circle segment
 *
 * @param x X coordinate of centre of circle that the segment is part of
 * @param y Y coordinate of centre of circle that the segment is part of
 * @param startAngle The start angle of the segment in degrees. 0 is straight up increasing clockwise
 * @param endAngle The end angle of the segment in degrees. 0 is straight up increasing clockwise
 * @param colour The colour in 565 format
 * @note Clipped as required
 */
void GraphicsSegment(int16_t x, int16_t y, uint16_t radius, int16_t startAngle, int16_t endAngle, colour_t colour);

/**
 * Draw a filled empty circle segment
 *
 * @param x X coordinate of centre of circle that the segment is part of
 * @param y Y coordinate of centre of circle that the segment is part of
 * @param startAngle The start angle of the segment in degrees. 0 is straight up increasing clockwise
 * @param endAngle The end angle of the segment in degrees. 0 is straight up increasing clockwise
 * @param angleStepSize A segment is drawn as a set of triangles. This controls the angle of rach triangle.
 * @param colour The colour in 565 format
 * @note Clipped as required
 */
void GraphicsFilledSegment(int16_t x, int16_t y, uint16_t radius, int16_t startAngle, int16_t endAngle, uint16_t angleStepSize, colour_t colour);

/**
 * Draw a filled round cornered rectangle
 *
 * @param x Left edge coordinate
 * @param y Top edge coordinate
 * @param width Width of rectangle
 * @param height Height of rectangle
 * @param cornerRadius Radius of each corner
 * @param colour The colour in 565 format
 * @note Clipped as required
 */
void GraphicsRoundedRectangle(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t cornerRadius, colour_t colour);

#endif /* INC_GRAPHICS_H_ */
