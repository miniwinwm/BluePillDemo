#include <stdint.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "graphics.h"
#include "st7789.h"

#define DEGREES_IN_RAD 57.2958f

static const uint8_t Font9_Table[];
static void arcPoint(int16_t x, int16_t y, int16_t arcX, int16_t arcY, int16_t startAngle, int16_t endAngle, colour_t colour);
static void filledRectangleClip(int16_t x, int16_t y, uint16_t width, uint16_t height, colour_t colour);

static int16_t triangleCornersX[3];
static int16_t triangleCornersY[3];

void GraphicsInit(void)
{
	ST7789Reset();
	ST7789Init();
}

void GraphicsClear(colour_t colour)
{
	filledRectangleClip(0U, 0U, (uint16_t)ST7789_LCD_WIDTH, (uint16_t)ST7789_LCD_HEIGHT, colour);
}

void GraphicsPixel(int16_t x, int16_t y, colour_t colour)
{
	ST7789Pixel((uint16_t)x, (uint16_t)y, colour);
}

void GraphicsVline(int16_t x, int16_t yStart, int16_t yEnd, colour_t colour)
{
	int16_t temp;

	if (yStart > yEnd)
	{
		temp = yEnd;
		yEnd = yStart;
		yStart = temp;
	}

	filledRectangleClip(x, yStart, 1U, (uint16_t)((yEnd - yStart) + 1), colour);
}

void GraphicsHline(int16_t xStart, int16_t xEnd, int16_t y, colour_t colour)
{
	int16_t temp;

	if (xStart > xEnd)
	{
		temp = xEnd;
		xEnd = xStart;
		xStart = temp;
	}

	filledRectangleClip(xStart, y, (uint16_t)((xEnd - xStart) + 1), 1U, colour);
}

void GraphicsRectangle(int16_t x, int16_t y, uint16_t width, uint16_t height, colour_t colour)
{
	GraphicsHline(x, x + (int16_t)width - 1, y, colour);
	GraphicsHline(x, x + (int16_t)width - 1, y + (int16_t)height - 1, colour);
	GraphicsVline(x, y, y + (int16_t)height - 1, colour);
	GraphicsVline(x + (int16_t)width - 1, y, y + (int16_t)height - 1, colour);
}

void GraphicsFilledRectangle(int16_t x, int16_t y, uint16_t width, uint16_t height, colour_t colour)
{
	filledRectangleClip(x, y, width, height, colour);
}

void GraphicsCharacter(int16_t x, int16_t y, char c, colour_t colour)
{
	int8_t charX;
	int8_t charY;
	uint8_t rowByte;
	uint8_t mask;
	uint16_t byte_pos = ((int16_t)c - (int16_t)' ') * GRAPHICS_STANDARD_CHARACTER_HEIGHT;

   	for (charY = 0; charY < GRAPHICS_STANDARD_CHARACTER_HEIGHT; charY++ )
    {
		mask = 0x80U;
		rowByte = Font9_Table[byte_pos];
		for (charX = 0; charX < GRAPHICS_STANDARD_CHARACTER_WIDTH; charX++)
		{
	   		if (rowByte & mask)
	   		{
	   			ST7789Pixel(x + (int16_t)charX, y + (int16_t)charY, colour);
	   		}
	   		mask >>= 1;
		}
		byte_pos++;
    }
}

void GraphicsCharacterVert(int16_t x, int16_t y, char c, colour_t colour)
{
	int8_t charX;
	int8_t charY;
	uint8_t rowByte;
	uint8_t mask;
	uint16_t byte_pos = ((int16_t)c - (int16_t)' ') * GRAPHICS_STANDARD_CHARACTER_HEIGHT;

   	for (charY = 0; charY < GRAPHICS_STANDARD_CHARACTER_HEIGHT; charY++ )
    {
		mask = 0x80U;
		rowByte = Font9_Table[byte_pos];
		for (charX = 0; charX < GRAPHICS_STANDARD_CHARACTER_WIDTH; charX++)
		{
	   		if (rowByte & mask)
	   		{
	   			ST7789Pixel(x + GRAPHICS_STANDARD_CHARACTER_HEIGHT - (int16_t)charY - 1U, y + (int16_t)charX, colour);
	   		}
	   		mask >>= 1;
		}
		byte_pos++;
    }
}

void GraphicsLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, colour_t colour)
{
	int16_t F;
	int16_t x;
	int16_t y;
	int16_t dy;
	int16_t dx;
	int16_t dy2;
	int16_t dx2;
	int16_t dy2_minus_dx2;
	int16_t dy2_plus_dx2;

	if (x1 > x2)
	{
		F = x1;
		x1 = x2;
		x2 = F;

		F = y1;
		y1 = y2;
		y2 = F;
	}

	dy = y2 - y1;
	dx = x2 - x1;
	dy2 = (dy << 1);
	dx2 = (dx << 1);
	dy2_minus_dx2 = dy2 - dx2;
	dy2_plus_dx2 = dy2 + dx2;

	if (dy >= 0)
	{
		if (dy <= dx)
		{
			F = dy2 - dx;
			x = x1;
			y = y1;
			while (x <= x2)
			{
				ST7789Pixel(x, y, colour);

				if (F <= 0)
				{
					F += dy2;
				}
				else
				{
					y++;
					F += dy2_minus_dx2;
				}
				x++;
			}
		}
		else
		{
			F = dx2 - dy;
			y = y1;
			x = x1;
			while (y <= y2)
			{
				ST7789Pixel(x, y, colour);
				if (F <= 0)
				{
					F += dx2;
				}
				else
				{
					x++;
					F -= dy2_minus_dx2;
				}
			y++;
			}
		}
	}
	else
	{
		if (dx >= -dy)
		{
			F = -dy2 - dx;
			x = x1;
			y = y1;
			while (x <= x2)
			{
				ST7789Pixel(x, y, colour);
				if ( F <= 0)
				{
					F -= dy2;
				}
				else
				{
					y--;
					F -= dy2_plus_dx2;
				}
				x++;
			}
		}
		else
		{
			F = dx2 + dy;
			y = y1;
			x = x1;
			while (y >= y2)
			{
				ST7789Pixel(x, y, colour);
				if (F <= 0)
				{
					F += dx2;
				}
				else
				{
					x++;
					F += dy2_plus_dx2;
				}
				y--;
			}
		}
	}
}

void GraphicsCircle(int16_t x, int16_t y, uint16_t radius, colour_t colour)
{
	int16_t xPoint = 0;
	int16_t yPoint = (int16_t)radius;
	int16_t decision = 1 - radius;

	// plot top, bottom, left and right
	ST7789Pixel(x, y + radius, colour);
	ST7789Pixel(x, y - radius, colour);
	ST7789Pixel(x + radius, y, colour);
	ST7789Pixel(x - radius, y, colour);

	while (yPoint > xPoint)
	{
		if (decision < 0)
		{
			decision += (xPoint << 1) + 3;
			xPoint++;
		}
		else
		{
			decision += ((xPoint - yPoint) << 1) + 5;
			xPoint++;
			yPoint--;
		}

		// plot all quadrants
		ST7789Pixel(x + xPoint, y + yPoint, colour);
		ST7789Pixel(x - xPoint, y + yPoint, colour);
		ST7789Pixel(x + xPoint, y - yPoint, colour);
		ST7789Pixel(x - xPoint, y - yPoint, colour);
		ST7789Pixel(x + yPoint, y + xPoint, colour);
		ST7789Pixel(x - yPoint, y + xPoint, colour);
		ST7789Pixel(x + yPoint, y - xPoint, colour);
		ST7789Pixel(x - yPoint, y - xPoint, colour);
	}
}

void GraphicsMonochromeBitmap(int16_t x, int16_t y, uint16_t width, uint16_t height, colour_t fgColour, colour_t bgColour, const uint8_t *imageData)
{
	ST7789DrawMonoBitmap(x, y, width, height, imageData, fgColour, bgColour);
}

void GraphicsColourBitmap(int16_t x, int16_t y, uint16_t width, uint16_t height, const uint8_t *imageData)
{
	ST7789DrawColourBitmap(x, y, width, height, imageData);
}

void GraphicsString(int16_t x, int16_t y, const char *s, colour_t colour)
{
	size_t c;
	size_t length;

	length = strlen(s);
	for (c = (size_t)0; c < length; c++)
	{
		GraphicsCharacter(x + (int16_t)c * (GRAPHICS_STANDARD_CHARACTER_WIDTH + 1), y, s[c], colour);
	}
}

void GraphicsStringVert(int16_t x, int16_t y, const char *s, colour_t colour)
{
	size_t c;
	size_t length;

	length = strlen(s);
	for (c = (size_t)0; c < length; c++)
	{
		GraphicsCharacterVert(x, y + (int16_t)c * (GRAPHICS_STANDARD_CHARACTER_WIDTH + 1), s[c], colour);
	}
}

void GraphicsFilledCircle(int16_t x, int16_t y, uint16_t radius, colour_t colour)
{
	int16_t xPoint = 0U;
	int16_t yPoint = (int16_t)radius;
	int16_t decision = 1 - (int16_t)radius;

	// plot centre line
	filledRectangleClip(x, y - (int16_t)radius, 1U, radius * 2U + 1U, colour);

	while (yPoint > xPoint)
	{
		if (decision < 0)
		{
			decision += (xPoint << 1) + 3;
			xPoint++;
		}
		else
		{
			decision += ((xPoint-yPoint) << 1) + 5;
			xPoint++;
			yPoint--;
		}

		// plot all quadrants
		filledRectangleClip(x + xPoint, y - yPoint, 1U, (uint16_t)yPoint * 2U + 1U, colour);
		filledRectangleClip(x - xPoint, y - yPoint, 1U, (uint16_t)yPoint * 2U + 1U, colour);
		filledRectangleClip(x + yPoint, y - xPoint, 1U, (uint16_t)xPoint * 2U + 1U, colour);
		filledRectangleClip(x - yPoint, y - xPoint, 1U, (uint16_t)xPoint * 2U + 1U, colour);
	}
}

void GraphicsRotateShape(uint8_t corners, int16_t *xArray, int16_t *yArray, int16_t angleDegrees)
{
	uint8_t i;
	int16_t x;
	float angleRadians;

	angleRadians = (float)angleDegrees / DEGREES_IN_RAD;
	for (i = 0U; i < corners; i++)
	{
		x = (int16_t)((float)xArray[i] * cosf(angleRadians) - (float)yArray[i] * sinf(angleRadians));
		yArray[i] = (int16_t)((float)xArray[i] * sinf(angleRadians) + (float)yArray[i] * cosf(angleRadians));
		xArray[i] = x;
	}
}

void GraphicsScaleShape(uint8_t corners, int16_t *xArray, int16_t *yArray, int16_t scaleX, int16_t scaleY)
{
	uint8_t i;

	for (i = 0U; i < corners; i++)
	{
		xArray[i] = xArray[i] * scaleX / 10;
		yArray[i] = yArray[i] * scaleY / 10;
	}
}

void GraphicsDrawShape(uint8_t corners, const int16_t *xArray, const int16_t *yArray, int16_t xOffset, int16_t yOffset, colour_t colour)
{
	uint8_t i;

	for (i = 0U; i < corners - 1U; i++)
	{
		GraphicsLine(xArray[i] + xOffset, yArray[i] + yOffset, xArray[i + 1U] + xOffset, yArray[i + 1U] + yOffset, colour);
	}

	GraphicsLine(xArray[i] + xOffset, yArray[i] + yOffset, xArray[0] + xOffset, yArray[0] + yOffset, colour);
}

void GraphicsDrawFilledShape(uint8_t corners, const int16_t *xArray, const int16_t *yArray, int16_t x, int16_t y, colour_t colour)
{
	uint16_t i;
	uint16_t j;
	int16_t swap;
	uint16_t line_node_count;
	int16_t yNext;
	int16_t node_x[GRAPHICS_MAX_SHAPE_CORNERS - 1U];
	int16_t yMin = INT16_MAX;
	int16_t yMax = INT16_MIN;
	float temp;

	// find y range of shape
	for (i = 0U; i < corners; i++)
	{
		if (yArray[i] < yMin)
		{
			yMin = yArray[i];
		}
		if (yArray[i] > yMax)
		{
			yMax = yArray[i];
		}
	}

	//  Loop through the rows of the image
	for (yNext = yMin; yNext <= yMax; yNext++)
	{
		if (yNext + y > (int16_t)ST7789_LCD_HEIGHT)
		{
			return;
		}

		// build a list of nodes on this line
		line_node_count = 0U;
		j = corners - 1U;
		for (i = 0U; i < corners; i++)
		{
			if ((yArray[i] < yNext && yArray[j] >= yNext) || (yArray[j] < yNext && yArray[i] >= yNext))
			{
				temp = (float)(yNext - yArray[i]) / (float)((yArray[j] - yArray[i]));
				node_x[line_node_count++] = (int16_t)(xArray[i] + temp * ((xArray[j] - xArray[i])));
			}
			j = i;
		}

		// sort the nodes, via a simple bubble sort
		i = 0U;
		while (i + 1U < line_node_count)
		{
			if (node_x[i] > node_x[i + 1U])
			{
				swap = node_x[i];
				node_x[i] = node_x[i + 1U];
				node_x[i + 1U] = swap;
				if (i)
				{
					i--;
				}
			}
			else
			{
				i++;
			}
		}

		// fill the pixels between node pairs
		for (i = 0U; i < line_node_count; i += 2U)
		{
			node_x[i] += x;
			node_x[i + 1U] += x;

			if (node_x[i] > (int16_t)ST7789_LCD_WIDTH)
			{
				break;
			}
			if (node_x[i + 1U] > 0)
			{
				filledRectangleClip(node_x[i], yNext + y, (uint16_t)(node_x[i + 1U] - node_x[i] + 1), 1U , colour);
 			}
		}
	}
}

void GraphicsArc(int16_t x, int16_t y, int16_t radius, int16_t startAngle, int16_t endAngle, colour_t colour)
{
	int16_t xPoint = 0;
	int16_t yPoint = radius;
	int16_t decision = 1 - radius;

	if (startAngle < 0)
	{
		startAngle += 360;
	}

	if (endAngle < 0)
	{
		endAngle += 360;
	}

	// plot top, bottom, left and right
	arcPoint(x, y, x, y + radius, startAngle, endAngle, colour);  // bottom
	arcPoint(x, y, x, y - radius, startAngle, endAngle, colour);  // top
	arcPoint(x, y, x + radius, y, startAngle, endAngle, colour);  // right
	arcPoint(x, y, x - radius, y, startAngle, endAngle, colour);  // left

	while (yPoint > xPoint)
	{
		if (decision < 0)
		{
			decision += (xPoint << 1) + 3;
			xPoint++;
		}
		else
		{
			decision += ((xPoint - yPoint) << 1) + 5;
			xPoint++;
			yPoint--;
		}

		// plot all quadrants
		arcPoint(x, y, x + xPoint, y + yPoint, startAngle, endAngle, colour);
		arcPoint(x, y, x - xPoint, y + yPoint, startAngle, endAngle, colour);
		arcPoint(x, y, x + xPoint, y - yPoint, startAngle, endAngle, colour);
		arcPoint(x, y, x - xPoint, y - yPoint, startAngle, endAngle, colour);
		arcPoint(x, y, x + yPoint, y + xPoint, startAngle, endAngle, colour);
		arcPoint(x, y, x - yPoint, y + xPoint, startAngle, endAngle, colour);
		arcPoint(x, y, x + yPoint, y - xPoint, startAngle, endAngle, colour);
		arcPoint(x, y, x - yPoint, y - xPoint, startAngle, endAngle, colour);
	}
}

void GraphicsSegment(int16_t x, int16_t y, uint16_t radius, int16_t startAngle, int16_t endAngle, colour_t colour)
{
	GraphicsLine(x,
			y,
			x + (int16_t)(sinf((float)startAngle / DEGREES_IN_RAD) * (float)radius),
			y - (int16_t)(cosf((float)startAngle / DEGREES_IN_RAD) * (float)radius),
			colour);

	GraphicsLine(x,
			y,
			x + (int16_t)(sinf((float)endAngle / DEGREES_IN_RAD) * (float)radius),
			y - (int16_t)(cosf((float)endAngle / DEGREES_IN_RAD) * (float)radius),
			colour);

	GraphicsArc(x, y, radius, startAngle, endAngle, colour);
}

static void arcPoint(int16_t x, int16_t y, int16_t arcX, int16_t arcY, int16_t startAngle, int16_t endAngle, colour_t colour)
{
	// calculate the angle the current point makes with the circle centre
	int16_t angle = (int16_t)(DEGREES_IN_RAD * (atan2f((float)(y - arcY), (float)(x - arcX)))) - 90.0f;
	if (angle < 0)
	{
		angle += 360;
	}

	if (endAngle > startAngle)
	{
		if (angle >= startAngle && angle < endAngle)
		{
			ST7789Pixel(arcX, arcY, colour);
		}
	}
	else
	{
		if (!(angle >= endAngle && angle < startAngle))
		{
			ST7789Pixel(arcX, arcY, colour);
		}
	}
}

void GraphicsFilledSegment(int16_t x, int16_t y, uint16_t radius, int16_t startAngle, int16_t endAngle, uint16_t angleStepSize, colour_t colour)
{
	int16_t oldX = x + (int16_t)(sinf((float)startAngle / DEGREES_IN_RAD) * (float)radius);
	int16_t oldY = y - (int16_t)(cosf((float)startAngle / DEGREES_IN_RAD) * (float)radius);
	int16_t i;
	float angle;

	triangleCornersX[0] = x;
	triangleCornersY[0] = y;

	if (startAngle > endAngle)
	{
		endAngle += 360;
	}

	for (i = startAngle + (int16_t)angleStepSize; i <= endAngle; i += (int16_t)angleStepSize)
	{
		angle = (float)i / DEGREES_IN_RAD;
		triangleCornersX[1] = x + (int16_t)(sinf(angle) * (float)radius);
		triangleCornersY[1] = y - (int16_t)(cosf(angle) * (float)radius);
		triangleCornersX[2] = oldX;
		triangleCornersY[2] = oldY;

		GraphicsDrawFilledShape(3U, triangleCornersX, triangleCornersY, 0, 0, colour);

		oldX = triangleCornersX[1];
		oldY = triangleCornersY[1];
	}

	i -= angleStepSize;
	if (i < endAngle)
	{
		triangleCornersX[1] = x + (int16_t)(sinf(endAngle / DEGREES_IN_RAD) * (float)radius);
		triangleCornersY[1] = y - (int16_t)(cosf(endAngle / DEGREES_IN_RAD) * (float)radius);
		triangleCornersX[2] = x + (int16_t)(sinf(i / DEGREES_IN_RAD) * (float)radius);
		triangleCornersY[2] = y - (int16_t)(cosf(i / DEGREES_IN_RAD) * (float)radius);

		GraphicsDrawFilledShape(3U, triangleCornersX, triangleCornersY, 0, 0, colour);
	}
}

void GraphicsRoundedRectangle(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t cornerRadius, colour_t colour)
{
	if (width < cornerRadius * 2 || height < cornerRadius * 2)
	{
		return;
	}

	GraphicsFilledRectangle(x + (int16_t)cornerRadius, y, width - (2 * cornerRadius), height, colour);
	GraphicsFilledRectangle(x, y + (int16_t)cornerRadius, cornerRadius, height - (2 * cornerRadius), colour);
	GraphicsFilledRectangle(x + width - (int16_t)cornerRadius, y + (int16_t)cornerRadius, cornerRadius, height - (2 * cornerRadius), colour);
	GraphicsFilledSegment(x + (int16_t)cornerRadius, y + (int16_t)cornerRadius, cornerRadius, 270, 360, 15U, colour);
	GraphicsFilledSegment(x + width - (int16_t)cornerRadius - 1, y + (int16_t)cornerRadius, cornerRadius, 0, 90, 15U, colour);
	GraphicsFilledSegment(x + (int16_t)cornerRadius, y + height - (int16_t)cornerRadius - 1, cornerRadius, 180, 270, 15U, colour);
	GraphicsFilledSegment(x + width - (int16_t)cornerRadius - 1, y + height - (int16_t)cornerRadius - 1, cornerRadius, 90, 180, 15U, colour);
}

static void filledRectangleClip(int16_t x, int16_t y, uint16_t width, uint16_t height, colour_t colour)
{
	int16_t x_end;
	int16_t y_end;

	if (x > (int16_t)ST7789_LCD_WIDTH - 1 || y > (int16_t)ST7789_LCD_HEIGHT - 1)
	{
		return;
	}

	if (x + (int16_t)width < 0 || y + (int16_t)height < 0)
	{
		return;
	}

	if (x < 0)
	{
		width -= (uint16_t)-x;
		x = 0;
	}

	if (y < 0)
	{
		height -= (uint16_t)-y;
		y = 0;
	}

	x_end = x + (int16_t)width - 1;
	if (x_end >(int16_t)ST7789_LCD_WIDTH - 1)
	{
		x_end = (int16_t)ST7789_LCD_WIDTH - 1;
	}

	y_end = y + (int16_t)height - 1;
	if (y_end > (int16_t)ST7789_LCD_HEIGHT - 1)
	{
		y_end = (int16_t)ST7789_LCD_HEIGHT - 1;
	}

	// clipped sizes
	width = (uint16_t)(x_end - x + 1);
	height = (uint16_t)(y_end - y + 1);

	ST7789FilledRectangle((uint16_t)x, (uint16_t)y, width, height, colour);
}

/**
 * Standard 5x9 font in the format of the ST fonts
 */
static const uint8_t Font9_Table[] =
{
	// @0 ' ' (5 pixels wide)
	0x00, //
	0x00, //
	0x00, //
	0x00, //
	0x00, //
	0x00, //
	0x00, //
	0x00, //
	0x00, //

	// @9 '!' (5 pixels wide)
	0x20, //   #
	0x20, //   #
	0x20, //   #
	0x20, //   #
	0x20, //   #
	0x00, //
	0x20, //   #
	0x00, //
	0x00, //

	// @18 '"' (5 pixels wide)
	0x50, //  # #
	0x50, //  # #
	0x00, //
	0x00, //
	0x00, //
	0x00, //
	0x00, //
	0x00, //
	0x00, //

	// @27 '#' (5 pixels wide)
	0x50, //  # #
	0x50, //  # #
	0xf8, // #####
	0x50, //  # #
	0xf8, // #####
	0x50, //  # #
	0x50, //  # #
	0x00, //
	0x00, //

	// @36 '$' (5 pixels wide)
	0x20, //   #
	0x78, //  ####
	0xa0, // # #
	0x70, //  ###
	0x28, //   # #
	0xf0, // ####
	0x20, //   #
	0x00, //
	0x00, //

	// @45 '%' (5 pixels wide)
	0xc0, // ##
	0xc8, // ##  #
	0x10, //    #
	0x20, //   #
	0x40, //  #
	0x98, // #  ##
	0x18, //    ##
	0x00, //
	0x00, //

	// @54 '&' (5 pixels wide)
	0x60, //  ##
	0x90, // #  #
	0xa0, // # #
	0x40, //  #
	0xa8, // # # #
	0x90, // #  #
	0x68, //  ## #
	0x00, //
	0x00, //

	// @63 ''' (5 pixels wide)
	0x20, //   #
	0x20, //   #
	0x00, //
	0x00, //
	0x00, //
	0x00, //
	0x00, //
	0x00, //
	0x00, //

	// @72 '(' (5 pixels wide)
	0x10, //    #
	0x20, //   #
	0x40, //  #
	0x40, //  #
	0x40, //  #
	0x20, //   #
	0x10, //    #
	0x00, //
	0x00, //

	// @81 ')' (5 pixels wide)
	0x40, //  #
	0x20, //   #
	0x10, //    #
	0x10, //    #
	0x10, //    #
	0x20, //   #
	0x40, //  #
	0x00, //
	0x00, //

	// @90 '*' (5 pixels wide)
	0x00, //
	0x20, //   #
	0xa8, // # # #
	0x70, //  ###
	0xa8, // # # #
	0x20, //   #
	0x00, //
	0x00, //
	0x00, //

	// @99 '+' (5 pixels wide)
	0x00, //
	0x20, //   #
	0x20, //   #
	0xf8, // #####
	0x20, //   #
	0x20, //   #
	0x00, //
	0x00, //
	0x00, //

	// @108 ',' (5 pixels wide)
	0x00, //
	0x00, //
	0x00, //
	0x00, //
	0x00, //
	0x00, //
	0x20, //   #
	0x40, //  #
	0x00, //

	// @117 '-' (5 pixels wide)
	0x00, //
	0x00, //
	0x00, //
	0xf8, // #####
	0x00, //
	0x00, //
	0x00, //
	0x00, //
	0x00, //

	// @126 '.' (5 pixels wide)
	0x00, //
	0x00, //
	0x00, //
	0x00, //
	0x00, //
	0x00, //
	0x20, //   #
	0x00, //
	0x00, //

	// @135 '/' (5 pixels wide)
	0x10, //    #
	0x10, //    #
	0x20, //   #
	0x20, //   #
	0x20, //   #
	0x40, //  #
	0x40, //  #
	0x00, //
	0x00, //

	// @144 '0' (5 pixels wide)
	0x70, //  ###
	0x88, // #   #
	0x98, // #  ##
	0xa8, // # # #
	0xc8, // ##  #
	0x88, // #   #
	0x70, //  ###
	0x00, //
	0x00, //

	// @153 '1' (5 pixels wide)
	0x20, //   #
	0x60, //  ##
	0x20, //   #
	0x20, //   #
	0x20, //   #
	0x20, //   #
	0x20, //   #
	0x00, //
	0x00, //

	// @162 '2' (5 pixels wide)
	0x70, //  ###
	0x88, // #   #
	0x08, //     #
	0x10, //    #
	0x20, //   #
	0x40, //  #
	0xf8, // #####
	0x00, //
	0x00, //

	// @171 '3' (5 pixels wide)
	0x70, //  ###
	0x88, // #   #
	0x08, //     #
	0x30, //   ##
	0x08, //     #
	0x88, // #   #
	0x70, //  ###
	0x00, //
	0x00, //

	// @180 '4' (5 pixels wide)
	0x10, //    #
	0x30, //   ##
	0x50, //  # #
	0x90, // #  #
	0xf8, // #####
	0x10, //    #
	0x10, //    #
	0x00, //
	0x00, //

	// @189 '5' (5 pixels wide)
	0xf8, // #####
	0x80, // #
	0xf0, // ####
	0x08, //     #
	0x08, //     #
	0x88, // #   #
	0x70, //  ###
	0x00, //
	0x00, //

	// @198 '6' (5 pixels wide)
	0x30, //   ##
	0x40, //  #
	0x80, // #
	0xf0, // ####
	0x88, // #   #
	0x88, // #   #
	0x70, //  ###
	0x00, //
	0x00, //

	// @207 '7' (5 pixels wide)
	0xf8, // #####
	0x08, //     #
	0x10, //    #
	0x20, //   #
	0x40, //  #
	0x40, //  #
	0x40, //  #
	0x00, //
	0x00, //

	// @216 '8' (5 pixels wide)
	0x70, //  ###
	0x88, // #   #
	0x88, // #   #
	0x70, //  ###
	0x88, // #   #
	0x88, // #   #
	0x70, //  ###
	0x00, //
	0x00, //

	// @225 '9' (5 pixels wide)
	0x70, //  ###
	0x88, // #   #
	0x88, // #   #
	0x78, //  ####
	0x08, //     #
	0x10, //    #
	0x60, //  ##
	0x00, //
	0x00, //

	// @234 ':' (5 pixels wide)
	0x00, //
	0x00, //
	0x20, //   #
	0x00, //
	0x00, //
	0x20, //   #
	0x00, //
	0x00, //
	0x00, //

	// @243 ';' (5 pixels wide)
	0x00, //
	0x00, //
	0x20, //   #
	0x00, //
	0x00, //
	0x20, //   #
	0x40, //  #
	0x00, //
	0x00, //

	// @252 '<' (5 pixels wide)
	0x00, //
	0x10, //    #
	0x20, //   #
	0x40, //  #
	0x20, //   #
	0x10, //    #
	0x00, //
	0x00, //
	0x00, //

	// @261 '=' (5 pixels wide)
	0x00, //
	0x00, //
	0xf8, // #####
	0x00, //
	0xf8, // #####
	0x00, //
	0x00, //
	0x00, //
	0x00, //

	// @270 '>' (5 pixels wide)
	0x00, //
	0x80, // #
	0x40, //  #
	0x20, //   #
	0x40, //  #
	0x80, // #
	0x00, //
	0x00, //
	0x00, //

	// @279 '?' (5 pixels wide)
	0x70, //  ###
	0x88, // #   #
	0x08, //     #
	0x10, //    #
	0x20, //   #
	0x00, //
	0x20, //   #
	0x00, //
	0x00, //

	// @288 '@' (5 pixels wide)
	0x70, //  ###
	0x88, // #   #
	0x88, // #   #
	0xb8, // # ###
	0xb0, // # ##
	0x80, // #
	0x78, //  ####
	0x00, //
	0x00, //

	// @297 'A' (5 pixels wide)
	0x20, //   #
	0x50, //  # #
	0x88, // #   #
	0x88, // #   #
	0xf8, // #####
	0x88, // #   #
	0x88, // #   #
	0x00, //
	0x00, //

	// @306 'B' (5 pixels wide)
	0xf0, // ####
	0x88, // #   #
	0x88, // #   #
	0xf0, // ####
	0x88, // #   #
	0x88, // #   #
	0xf0, // ####
	0x00, //
	0x00, //

	// @315 'C' (5 pixels wide)
	0x70, //  ###
	0x88, // #   #
	0x80, // #
	0x80, // #
	0x80, // #
	0x88, // #   #
	0x70, //  ###
	0x00, //
	0x00, //

	// @324 'D' (5 pixels wide)
	0xe0, // ###
	0x90, // #  #
	0x88, // #   #
	0x88, // #   #
	0x88, // #   #
	0x90, // #  #
	0xe0, // ###
	0x00, //
	0x00, //

	// @333 'E' (5 pixels wide)
	0xf8, // #####
	0x80, // #
	0x80, // #
	0xf0, // ####
	0x80, // #
	0x80, // #
	0xf8, // #####
	0x00, //
	0x00, //

	// @342 'F' (5 pixels wide)
	0xf8, // #####
	0x80, // #
	0x80, // #
	0xf0, // ####
	0x80, // #
	0x80, // #
	0x80, // #
	0x00, //
	0x00, //

	// @351 'G' (5 pixels wide)
	0x70, //  ###
	0x88, // #   #
	0x80, // #
	0x98, // #  ##
	0x88, // #   #
	0x88, // #   #
	0x78, //  ####
	0x00, //
	0x00, //

	// @360 'H' (5 pixels wide)
	0x88, // #   #
	0x88, // #   #
	0x88, // #   #
	0xf8, // #####
	0x88, // #   #
	0x88, // #   #
	0x88, // #   #
	0x00, //
	0x00, //

	// @369 'I' (5 pixels wide)
	0x70, //  ###
	0x20, //   #
	0x20, //   #
	0x20, //   #
	0x20, //   #
	0x20, //   #
	0x70, //  ###
	0x00, //
	0x00, //

	// @378 'J' (5 pixels wide)
	0x38, //   ###
	0x10, //    #
	0x10, //    #
	0x10, //    #
	0x10, //    #
	0x90, // #  #
	0x60, //  ##
	0x00, //
	0x00, //

	// @387 'K' (5 pixels wide)
	0x88, // #   #
	0x90, // #  #
	0xa0, // # #
	0xc0, // ##
	0xa0, // # #
	0x90, // #  #
	0x88, // #   #
	0x00, //
	0x00, //

	// @396 'L' (5 pixels wide)
	0x80, // #
	0x80, // #
	0x80, // #
	0x80, // #
	0x80, // #
	0x80, // #
	0xf8, // #####
	0x00, //
	0x00, //

	// @405 'M' (5 pixels wide)
	0x88, // #   #
	0xd8, // ## ##
	0xa8, // # # #
	0xa8, // # # #
	0x88, // #   #
	0x88, // #   #
	0x88, // #   #
	0x00, //
	0x00, //

	// @414 'N' (5 pixels wide)
	0x88, // #   #
	0x88, // #   #
	0xc8, // ##  #
	0xa8, // # # #
	0x98, // #  ##
	0x88, // #   #
	0x88, // #   #
	0x00, //
	0x00, //

	// @423 'O' (5 pixels wide)
	0x70, //  ###
	0x88, // #   #
	0x88, // #   #
	0x88, // #   #
	0x88, // #   #
	0x88, // #   #
	0x70, //  ###
	0x00, //
	0x00, //

	// @432 'P' (5 pixels wide)
	0xf0, // ####
	0x88, // #   #
	0x88, // #   #
	0xf0, // ####
	0x80, // #
	0x80, // #
	0x80, // #
	0x00, //
	0x00, //

	// @441 'Q' (5 pixels wide)
	0x70, //  ###
	0x88, // #   #
	0x88, // #   #
	0x88, // #   #
	0xa8, // # # #
	0x90, // #  #
	0x68, //  ## #
	0x00, //
	0x00, //

	// @450 'R' (5 pixels wide)
	0xf0, // ####
	0x88, // #   #
	0x88, // #   #
	0xf0, // ####
	0xa0, // # #
	0x90, // #  #
	0x88, // #   #
	0x00, //
	0x00, //

	// @459 'S' (5 pixels wide)
	0x70, //  ###
	0x88, // #   #
	0x80, // #
	0x70, //  ###
	0x08, //     #
	0x88, // #   #
	0x70, //  ###
	0x00, //
	0x00, //

	// @468 'T' (5 pixels wide)
	0xf8, // #####
	0x20, //   #
	0x20, //   #
	0x20, //   #
	0x20, //   #
	0x20, //   #
	0x20, //   #
	0x00, //
	0x00, //

	// @477 'U' (5 pixels wide)
	0x88, // #   #
	0x88, // #   #
	0x88, // #   #
	0x88, // #   #
	0x88, // #   #
	0x88, // #   #
	0x70, //  ###
	0x00, //
	0x00, //

	// @486 'V' (5 pixels wide)
	0x88, // #   #
	0x88, // #   #
	0x88, // #   #
	0x88, // #   #
	0x88, // #   #
	0x50, //  # #
	0x20, //   #
	0x00, //
	0x00, //

	// @495 'W' (5 pixels wide)
	0x88, // #   #
	0x88, // #   #
	0x88, // #   #
	0x88, // #   #
	0xa8, // # # #
	0xa8, // # # #
	0x50, //  # #
	0x00, //
	0x00, //

	// @504 'X' (5 pixels wide)
	0x88, // #   #
	0x88, // #   #
	0x50, //  # #
	0x20, //   #
	0x50, //  # #
	0x88, // #   #
	0x88, // #   #
	0x00, //
	0x00, //

	// @513 'Y' (5 pixels wide)
	0x88, // #   #
	0x88, // #   #
	0x88, // #   #
	0x50, //  # #
	0x20, //   #
	0x20, //   #
	0x20, //   #
	0x00, //
	0x00, //

	// @522 'Z' (5 pixels wide)
	0xf8, // #####
	0x08, //     #
	0x10, //    #
	0x20, //   #
	0x40, //  #
	0x80, // #
	0xf8, // #####
	0x00, //
	0x00, //

	// @531 '[' (5 pixels wide)
	0x70, //  ###
	0x40, //  #
	0x40, //  #
	0x40, //  #
	0x40, //  #
	0x40, //  #
	0x70, //  ###
	0x00, //
	0x00, //

	// @540 '\' (5 pixels wide)
	0x40, //  #
	0x40, //  #
	0x20, //   #
	0x20, //   #
	0x20, //   #
	0x10, //    #
	0x10, //    #
	0x00, //
	0x00, //

	// @549 ']' (5 pixels wide)
	0x70, //  ###
	0x10, //    #
	0x10, //    #
	0x10, //    #
	0x10, //    #
	0x10, //    #
	0x70, //  ###
	0x00, //
	0x00, //

	// @558 '^' (5 pixels wide)
	0x20, //   #
	0x50, //  # #
	0x88, // #   #
	0x00, //
	0x00, //
	0x00, //
	0x00, //
	0x00, //
	0x00, //

	// @567 '_' (5 pixels wide)
	0x00, //
	0x00, //
	0x00, //
	0x00, //
	0x00, //
	0x00, //
	0xf8, // #####
	0x00, //
	0x00, //

	// @576 '`' (5 pixels wide)
	0x40, //  #
	0x20, //   #
	0x10, //    #
	0x00, //
	0x00, //
	0x00, //
	0x00, //
	0x00, //
	0x00, //

	// @585 'a' (5 pixels wide)
	0x00, //
	0x00, //
	0x70, //  ###
	0x08, //     #
	0x78, //  ####
	0x88, // #   #
	0x78, //  ####
	0x00, //
	0x00, //

	// @594 'b' (5 pixels wide)
	0x80, // #
	0x80, // #
	0xb0, // # ##
	0xc8, // ##  #
	0x88, // #   #
	0x88, // #   #
	0xf0, // ####
	0x00, //
	0x00, //

	// @603 'c' (5 pixels wide)
	0x00, //
	0x00, //
	0x70, //  ###
	0x80, // #
	0x80, // #
	0x88, // #   #
	0x70, //  ###
	0x00, //
	0x00, //

	// @612 'd' (5 pixels wide)
	0x08, //     #
	0x08, //     #
	0x68, //  ## #
	0x98, // #  ##
	0x88, // #   #
	0x88, // #   #
	0x78, //  ####
	0x00, //
	0x00, //

	// @621 'e' (5 pixels wide)
	0x00, //
	0x00, //
	0x70, //  ###
	0x88, // #   #
	0xf8, // #####
	0x80, // #
	0x70, //  ###
	0x00, //
	0x00, //

	// @630 'f' (5 pixels wide)
	0x18, //    ##
	0x20, //   #
	0x20, //   #
	0x70, //  ###
	0x20, //   #
	0x20, //   #
	0x20, //   #
	0x00, //
	0x00, //

	// @639 'g' (5 pixels wide)
	0x00, //
	0x00, //
	0x68, //  ## #
	0x98, // #  ##
	0x88, // #   #
	0x88, // #   #
	0x78, //  ####
	0x08, //     #
	0x70, //  ###

	// @648 'h' (5 pixels wide)
	0x80, // #
	0x80, // #
	0xb0, // # ##
	0xc8, // ##  #
	0x88, // #   #
	0x88, // #   #
	0x88, // #   #
	0x00, //
	0x00, //

	// @657 'i' (5 pixels wide)
	0x20, //   #
	0x00, //
	0x20, //   #
	0x20, //   #
	0x20, //   #
	0x20, //   #
	0x20, //   #
	0x00, //
	0x00, //

	// @666 'j' (5 pixels wide)
	0x00, //
	0x10, //    #
	0x00, //
	0x10, //    #
	0x10, //    #
	0x10, //    #
	0x10, //    #
	0x90, // #  #
	0x60, //  ##

	// @675 'k' (5 pixels wide)
	0x80, // #
	0x80, // #
	0x90, // #  #
	0xa0, // # #
	0xc0, // ##
	0xa0, // # #
	0x90, // #  #
	0x00, //
	0x00, //

	// @684 'l' (5 pixels wide)
	0x20, //   #
	0x20, //   #
	0x20, //   #
	0x20, //   #
	0x20, //   #
	0x20, //   #
	0x10, //    #
	0x00, //
	0x00, //

	// @693 'm' (5 pixels wide)
	0x00, //
	0x00, //
	0xd0, // ## #
	0xa8, // # # #
	0xa8, // # # #
	0x88, // #   #
	0x88, // #   #
	0x00, //
	0x00, //

	// @702 'n' (5 pixels wide)
	0x00, //
	0x00, //
	0xb0, // # ##
	0xc8, // ##  #
	0x88, // #   #
	0x88, // #   #
	0x88, // #   #
	0x00, //
	0x00, //

	// @711 'o' (5 pixels wide)
	0x00, //
	0x00, //
	0x70, //  ###
	0x88, // #   #
	0x88, // #   #
	0x88, // #   #
	0x70, //  ###
	0x00, //
	0x00, //

	// @720 'p' (5 pixels wide)
	0x00, //
	0x00, //
	0xb0, // # ##
	0xc8, // ##  #
	0x88, // #   #
	0x88, // #   #
	0xf0, // ####
	0x80, // #
	0x80, // #

	// @729 'q' (5 pixels wide)
	0x00, //
	0x00, //
	0x68, //  ## #
	0x98, // #  ##
	0x88, // #   #
	0x88, // #   #
	0x78, //  ####
	0x08, //     #
	0x08, //     #

	// @738 'r' (5 pixels wide)
	0x00, //
	0x00, //
	0xb0, // # ##
	0xc8, // ##  #
	0x80, // #
	0x80, // #
	0x80, // #
	0x00, //
	0x00, //

	// @747 's' (5 pixels wide)
	0x00, //
	0x00, //
	0x70, //  ###
	0x80, // #
	0x70, //  ###
	0x08, //     #
	0xf0, // ####
	0x00, //
	0x00, //

	// @756 't' (5 pixels wide)
	0x40, //  #
	0x40, //  #
	0xe0, // ###
	0x40, //  #
	0x40, //  #
	0x48, //  #  #
	0x30, //   ##
	0x00, //
	0x00, //

	// @765 'u' (5 pixels wide)
	0x00, //
	0x00, //
	0x88, // #   #
	0x88, // #   #
	0x88, // #   #
	0x98, // #  ##
	0x68, //  ## #
	0x00, //
	0x00, //

	// @774 'v' (5 pixels wide)
	0x00, //
	0x00, //
	0x88, // #   #
	0x88, // #   #
	0x88, // #   #
	0x50, //  # #
	0x20, //   #
	0x00, //
	0x00, //

	// @783 'w' (5 pixels wide)
	0x00, //
	0x00, //
	0x88, // #   #
	0x88, // #   #
	0xa8, // # # #
	0xa8, // # # #
	0x50, //  # #
	0x00, //
	0x00, //

	// @792 'x' (5 pixels wide)
	0x00, //
	0x00, //
	0x88, // #   #
	0x50, //  # #
	0x20, //   #
	0x50, //  # #
	0x88, // #   #
	0x00, //
	0x00, //

	// @801 'y' (5 pixels wide)
	0x00, //
	0x00, //
	0x88, // #   #
	0x88, // #   #
	0x88, // #   #
	0x98, // #  ##
	0x68, //  ## #
	0x08, //     #
	0x70, //  ###

	// @810 'z' (5 pixels wide)
	0x00, //
	0x00, //
	0xf8, // #####
	0x10, //    #
	0x20, //   #
	0x40, //  #
	0xf8, // #####
	0x00, //
	0x00, //

	// @819 '{' (5 pixels wide)
	0x10, //    #
	0x20, //   #
	0x20, //   #
	0x40, //  #
	0x20, //   #
	0x20, //   #
	0x10, //    #
	0x00, //
	0x00, //

	// @828 '|' (5 pixels wide)
	0x20, //   #
	0x20, //   #
	0x20, //   #
	0x20, //   #
	0x20, //   #
	0x20, //   #
	0x20, //   #
	0x00, //
	0x00, //

	// @837 '}' (5 pixels wide)
	0x40, //  #
	0x20, //   #
	0x20, //   #
	0x10, //    #
	0x20, //   #
	0x20, //   #
	0x40, //  #
	0x00, //
	0x00, //

	// @846 '~' (5 pixels wide)
	0x00, //
	0x00, //
	0x40, //  #
	0xa8, // # # #
	0x10, //    #
	0x00, //
	0x00, //
	0x00, //
	0x00, //
};
