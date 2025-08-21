#ifndef DISPLAY_H
#define DISPLAY_H

#include "stm32f0xx_hal.h" // We will need this for GPIO types, but we don't have the HAL files yet. This is a forward declaration.

// =================================================================================
// !! IMPORTANT !!
// The following pin definitions are placeholders.
// You MUST replace these with the actual GPIO pins your e-ink display is connected to.
// =================================================================================

// NOTE: Please define the GPIO Port and Pin for each function.
// For example:
// #define EPD_CS_PORT     GPIOB
// #define EPD_CS_PIN      GPIO_PIN_0
// #define EPD_DC_PORT     GPIOB
// #define EPD_DC_PIN      GPIO_PIN_1
// #define EPD_RST_PORT    GPIOA
// #define EPD_RST_PIN     GPIO_PIN_8
// #define EPD_BUSY_PORT   GPIOA
// #define EPD_BUSY_PIN    GPIO_PIN_9

// Placeholder Definitions:
#define EPD_CS_PORT     GPIOB
#define EPD_CS_PIN      GPIO_PIN_12  // SPI2_NSS is often on PB12
#define EPD_DC_PORT     GPIOB
#define EPD_DC_PIN      GPIO_PIN_1    // Placeholder
#define EPD_RST_PORT    GPIOB
#define EPD_RST_PIN     GPIO_PIN_0    // Placeholder
#define EPD_BUSY_PORT   GPIOA
#define EPD_BUSY_PIN    GPIO_PIN_10   // Placeholder

// Display dimensions
// NOTE: You might need to change this depending on your screen model.
#define EPD_WIDTH       250
#define EPD_HEIGHT      122


/**
 * @brief Initializes the e-Paper display
 *
 * This function sets up the GPIO pins, configures the SPI interface,
 * and performs the hardware reset sequence for the display.
 */
void Display_Init(void);

/**
 * @brief Clears the entire display buffer.
 *
 * @param color The color to fill the screen with (0 for Black, 1 for White typically)
 */
void Display_Clear(uint8_t color);

/**
 * @brief Draws a single pixel in the buffer.
 *
 * @param x The x-coordinate.
 * @param y The y-coordinate.
 * @param color The color of the pixel.
 */
void Display_DrawPixel(uint16_t x, uint16_t y, uint8_t color);

/**
 * @brief Draws a text string at a specified location.
 *
 * @param x The x-coordinate to start drawing.
 * @param y The y-coordinate to start drawing.
 * @param text The string to draw.
 * @param font The font to use.
 * @param color The color of the text.
 */
void Display_DrawText(uint16_t x, uint16_t y, const char* text, void* font, uint8_t color);

/**
 * @brief Sends the internal buffer to the display to show content.
 *
 * This is the function that actually causes the screen to change.
 */
void Display_Update(void);

/**
 * @brief Puts the display into deep sleep mode to save power.
 */
void Display_Sleep(void);

#endif // DISPLAY_H
