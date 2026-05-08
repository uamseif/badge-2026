#ifndef CBTS_MATRIX_H
#define CBTS_MATRIX_H

#include <stdint.h>
#include <stdbool.h>

#define CBTS_MATRIX_DEVS 3

#define CBTS_MATRIX_RED_DRIVER_ADDRESS  0x71
#define CBTS_MATRIX_GREEN_DRIVER_ADDRESS  0x72
#define CBTS_MATRIX_BLUE_DRIVER_ADDRESS  0x74

#define _defaultRowBufferSize 16
#define _defaultColBufferSize 8

#define CBTS_MATRIX_I2C        I2C1
#define CBTS_MATRIX_TIMEOUT    100

#define RED_DRIVER              0
#define GREEN_DRIVER            1
#define BLUE_DRIVER             2


typedef struct
{
    uint8_t i2c_addr[CBTS_MATRIX_DEVS];
    uint16_t *buffer;
    uint8_t invertedMask;
} CBTS_MATRIX;

enum LedColor {
    RED=0,
    GREEN,
    BLUE,
    YELLOW,
    CYAN,
    MAGENTA,
    WHITE
};


/* API */
void CBTS_MATRIX_init(CBTS_MATRIX *dev);
void CBTS_MATRIX_deinit(CBTS_MATRIX *dev);

void CBTS_MATRIX_begin(CBTS_MATRIX *dev);

void CBTS_MATRIX_clearSingle(CBTS_MATRIX *dev, uint8_t devIdx);
void CBTS_MATRIX_clear(CBTS_MATRIX *dev);

void CBTS_MATRIX_setBrightnessSingle(CBTS_MATRIX *dev, uint8_t devIdx, uint8_t brightness);
void CBTS_MATRIX_setBrightness(CBTS_MATRIX *dev, uint8_t brightness);

void CBTS_MATRIX_setLed(CBTS_MATRIX *dev, uint8_t devIdx, uint8_t rowIdx, uint8_t colIdx, bool state);
bool CBTS_MATRIX_getLed(const CBTS_MATRIX *dev, uint8_t devIdx, uint8_t rowIdx, uint8_t colIdx);

void CBTS_MATRIX_showSingle(CBTS_MATRIX *dev, uint8_t devIdx);
void CBTS_MATRIX_show(CBTS_MATRIX *dev);

void CBTS_MATRIX_setLedWithColor(CBTS_MATRIX *dev, int row, int column, enum LedColor color, bool state);
#endif