#pragma once
// Waveshare 1.8-inch Knob reference board, official Arduino demo.
// These are S3 pins. Never flash the ESP32 audio companion with this target.
#define ESP32_I2C_DEV_NUM I2C_NUM_0
#define ESP32_I2C_SDA_PIN GPIO_NUM_11
#define ESP32_I2C_SCL_PIN GPIO_NUM_12
#define BOOT_BUTTON_PIN GPIO_NUM_0
#define PWR_BUTTON_PIN GPIO_NUM_NC
#define EPD_WIDTH 200
#define EPD_HEIGHT 200
#define EPD_BUSY_PIN GPIO_NUM_NC
#define TAIJI_WIDTH 360
#define TAIJI_HEIGHT 360
#define TAIJI_LCD_CS 14
#define TAIJI_LCD_CLK 13
#define TAIJI_LCD_D0 15
#define TAIJI_LCD_D1 16
#define TAIJI_LCD_D2 17
#define TAIJI_LCD_D3 18
#define TAIJI_LCD_RESET 21
#define TAIJI_BACKLIGHT 47
#define TAIJI_ENCODER_A 8
#define TAIJI_ENCODER_B 7
#define TAIJI_TOUCH_ADDRESS 0x15
#define TAIJI_AUDIO_OWNER 0
#define TAIJI_MIC_CLOCK GPIO_NUM_45
#define TAIJI_MIC_DATA GPIO_NUM_46
#define TAIJI_DAC_BCLK GPIO_NUM_39
#define TAIJI_DAC_WS GPIO_NUM_40
#define TAIJI_DAC_DATA GPIO_NUM_41
