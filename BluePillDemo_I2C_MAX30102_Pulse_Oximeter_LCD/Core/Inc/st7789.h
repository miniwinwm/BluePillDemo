#ifndef INC_ST7789_H_
#define INC_ST7789_H_

#include <stdint.h>
#include <stdbool.h>

#define ST7789_RST_PORT           	GPIOA
#define ST7789_RST_PIN              GPIO_PIN_9
#define ST7789_DC_PORT              GPIOA
#define ST7789_DC_PIN               GPIO_PIN_8
#define ST7789_PRESCALER            16UL
#define ST7789_SC_MHZ              	8UL
#define ST7789_LCD_WIDTH            240U
#define ST7789_LCD_HEIGHT           240U

typedef uint16_t colour_t;

// colour bit layout rrrr rggg gggb bbbb
#define BLACK 						((colour_t)0x0000)
#define YELLOW 						((colour_t)0xffe0)
#define RED 						((colour_t)0xf800)
#define GREEN 						((colour_t)0x07e0)
#define BLUE 						((colour_t)0x001f)
#define WHITE 						((colour_t)0xffff)
#define PINK 						((colour_t)0xf80e)
#define PURPLE 						((colour_t)0xf83f)
#define	GREY15						((colour_t)0x1082)
#define	GREY14						((colour_t)0x2104)
#define	GREY13						((colour_t)0x3186)
#define	GREY12						((colour_t)0x4208)
#define	GREY11						((colour_t)0x528a)
#define	GREY10						((colour_t)0x630c)
#define	GREY9						((colour_t)0x738e)
#define	GREY8						((colour_t)0x8410)
#define	GREY7						((colour_t)0x9492)
#define	GREY6						((colour_t)0xa514)
#define	GREY5						((colour_t)0xb596)
#define	GREY4						((colour_t)0xc618)
#define	GREY3						((colour_t)0xd69a)
#define	GREY2						((colour_t)0xe71c)
#define	GREY1						((colour_t)0xf79e)
#define ORANGE 						((colour_t)0xfb80)
#define CYAN						((colour_t)0x07ff)
#define DARK_CYAN					((colour_t)0x0492)
#define LIGHT_ORANGE				((colour_t)0xfe20)
#define BRICK_RED					((colour_t)0xb104)

// System Function Command Table 1
#define ST7789CMD_NOP               0x00U
#define ST7789CMD_SWRESET           0x01U
#define ST7789CMD_RDDID             0x04U
#define ST7789CMD_RDDST             0x09U
#define ST7789CMD_RDDPM             0x0aU
#define ST7789CMD_RDDMADCTL         0x0bU
#define ST7789CMD_RDDCOLMOD         0x0cU
#define ST7789CMD_RDDIM             0x0dU
#define ST7789CMD_RDDSM             0x0eU
#define ST7789CMD_RDDSDR            0x0fU
#define ST7789CMD_SLPIN             0x10U
#define ST7789CMD_SLPOUT            0x11U
#define ST7789CMD_PTLON             0x12U
#define ST7789CMD_NORON             0x13U
#define ST7789CMD_INVOFF            0x20U
#define ST7789CMD_INVON             0x21U
#define ST7789CMD_GAMSET            0x26U
#define ST7789CMD_DISPOFF           0x28U
#define ST7789CMD_DISPON            0x29U
#define ST7789CMD_CASET             0x2aU
#define ST7789CMD_RASET             0x2bU
#define ST7789CMD_RAMWR             0x2cU
#define ST7789CMD_RAMRD             0x2eU
#define ST7789CMD_PTLAR             0x30U
#define ST7789CMD_VSCRDEF           0x33U
#define ST7789CMD_TEOFF             0x34U
#define ST7789CMD_TEON              0x35U
#define ST7789CMD_MADCTL            0x36U
#define ST7789CMD_VSCRSADD          0x37U
#define ST7789CMD_IDMOFF            0x38U
#define ST7789CMD_IDMON             0x39U
#define ST7789CMD_COLMOD            0x3aU
#define ST7789CMD_RAMWRC            0x3cU
#define ST7789CMD_RAMRDC            0x3eU
#define ST7789CMD_TESCAN            0x44U
#define ST7789CMD_RDTESCAN          0x45U
#define ST7789CMD_WRDISBV           0x51U
#define ST7789CMD_RDDISBV           0x52U
#define ST7789CMD_WRCTRLD           0x53U
#define ST7789CMD_RDCTRLD           0x54U
#define ST7789CMD_WRCACE            0x55U
#define ST7789CMD_RDCABC            0x56U
#define ST7789CMD_WRCABCMB          0x5eU
#define ST7789CMD_RDCABCMB          0x5fU
#define ST7789CMD_RDABCSDR          0x68U
#define ST7789CMD_RDID1             0xdaU
#define ST7789CMD_RDID2             0xdbU
#define ST7789CMD_RDID3             0xdcU
#define ST7789CMD_RAMCTRL           0xb0U
#define ST7789CMD_RGBCTRL           0xb1U
#define ST7789CMD_PORCTRL           0xb2U
#define ST7789CMD_FRCTRL1           0xb3U
#define ST7789CMD_GCTRL             0xb7U
#define ST7789CMD_DGMEN             0xbaU
#define ST7789CMD_VCOMS             0xbbU
#define ST7789CMD_LCMCTRL           0xc0U
#define ST7789CMD_IDSET             0xc1U
#define ST7789CMD_VDVVRHEN          0xc2U
#define ST7789CMD_VRHS              0xc3U
#define ST7789CMD_VDVSET            0xc4U
#define ST7789CMD_VCMOFSET          0xc5U
#define ST7789CMD_FRCTR2            0xc6U
#define ST7789CMD_CABCCTRL          0xc7U
#define ST7789CMD_REGSEL1           0xc8U
#define ST7789CMD_REGSEL2           0xcaU
#define ST7789CMD_PWMFRSEL          0xccU
#define ST7789CMD_PWCTRL1           0xd0U
#define ST7789CMD_VAPVANEN          0xd2U
#define ST7789CMD_CMD2EN            0xdfU
#define ST7789CMD_PVGAMCTRL         0xe0U
#define ST7789CMD_NVGAMCTRL         0xe1U
#define ST7789CMD_DGMLUTR           0xe2U
#define ST7789CMD_DGMLUTB           0xe3U
#define ST7789CMD_GATECTRL          0xe4U
#define ST7789CMD_PWCTRL2           0xe8U
#define ST7789CMD_EQCTRL            0xe9U
#define ST7789CMD_PROMCTRL          0xecU
#define ST7789CMD_PROMEN            0xfaU
#define ST7789CMD_NVMSET            0xfcU
#define ST7789CMD_PROMACT           0xfeU

void ST7789Reset(void);
void ST7789Init(void);
void ST7789Pixel(uint16_t x, uint16_t y, colour_t colour);
void ST7789FilledRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, colour_t colour);
void ST7789DrawMonoBitmap(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *imageData, colour_t fgColour, colour_t bgColour);
void ST7789DrawColourBitmap(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *imageData);

#endif
