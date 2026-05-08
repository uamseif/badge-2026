#include "cbts_matrix.h"
#include <stdlib.h>
#include "i2c_hal.h"

/* Helpers */
static uint8_t maxColumns(void)
{
    return _defaultColBufferSize;
}

static inline uint8_t colIndex(uint8_t devIdx, uint8_t colIdx)
{
    return devIdx * _defaultColBufferSize + colIdx;
}

/* Init / Deinit */
void CBTS_MATRIX_init(CBTS_MATRIX *dev) {
    if (!dev) return;

    dev->buffer = NULL;
    dev->invertedMask = 0;

    dev->i2c_addr[RED_DRIVER] = CBTS_MATRIX_RED_DRIVER_ADDRESS;
    dev->i2c_addr[GREEN_DRIVER] = CBTS_MATRIX_GREEN_DRIVER_ADDRESS;
    dev->i2c_addr[BLUE_DRIVER] = CBTS_MATRIX_BLUE_DRIVER_ADDRESS;

    //dev->invertedMask = 0b00000001; //Temp fix
}

void CBTS_MATRIX_deinit(CBTS_MATRIX *dev)
{
    if (!dev) return;

    if (dev->buffer)
    {
        free(dev->buffer);
        dev->buffer = NULL;
    }
}

static inline uint8_t CBTS_MATRIX_addr(const CBTS_MATRIX *dev, uint8_t idx)
{
    return dev->i2c_addr[idx] << 1;   // HAL requiere shift
}

/* Begin */
void CBTS_MATRIX_begin(CBTS_MATRIX *dev)
{
    dev->buffer = calloc(maxColumns() * CBTS_MATRIX_DEVS, sizeof(uint16_t));
    if (!dev->buffer)
        return;

    for (uint8_t i = 0; i < CBTS_MATRIX_DEVS; i++)
    {
        uint8_t addr = CBTS_MATRIX_addr(dev, i);
        uint8_t cmd;

        /* Oscillator ON */
        cmd = 0x21;
        HAL_I2C_Transmit(CBTS_MATRIX_I2C, addr, &cmd, 1, CBTS_MATRIX_TIMEOUT);

        /* Display ON, Blink OFF */
        cmd = 0x81;
        HAL_I2C_Transmit(CBTS_MATRIX_I2C, addr, &cmd, 1, CBTS_MATRIX_TIMEOUT);

        CBTS_MATRIX_setBrightnessSingle(dev, i, 8);
        CBTS_MATRIX_clearSingle(dev, i);
        CBTS_MATRIX_showSingle(dev, i);
    }
}

/* Clear */
void CBTS_MATRIX_clearSingle(CBTS_MATRIX *dev, uint8_t devIdx)
{
    if (!dev || !dev->buffer || devIdx >= CBTS_MATRIX_DEVS)
        return;

    for (uint8_t i = 0; i < maxColumns(); i++)
        dev->buffer[colIndex(devIdx, i)] = 0;
}

void CBTS_MATRIX_clear(CBTS_MATRIX *dev)
{
    if (!dev) return;

    for (uint8_t d = 0; d < CBTS_MATRIX_DEVS; d++)
        CBTS_MATRIX_clearSingle(dev, d);
}

/* Brightness */
void CBTS_MATRIX_setBrightnessSingle(CBTS_MATRIX *dev, uint8_t devIdx, uint8_t brightness)
{
    if (devIdx >= CBTS_MATRIX_DEVS)
        return;

    brightness &= 0x0F;

    uint8_t cmd = 0xE0 | brightness;

    HAL_I2C_Transmit(
            CBTS_MATRIX_I2C,
            CBTS_MATRIX_addr(dev, devIdx),
            &cmd,
            1,
            CBTS_MATRIX_TIMEOUT);
}

void CBTS_MATRIX_setBrightness(CBTS_MATRIX *dev, uint8_t brightness)
{
    if (!dev) return;

    for (uint8_t d = 0; d < CBTS_MATRIX_DEVS; d++)
        CBTS_MATRIX_setBrightnessSingle(dev, d, brightness);
}

void CBTS_MATRIX_setLedWithColor(CBTS_MATRIX *dev, int row, int column, enum LedColor color, bool state) {
    //row = 15 - row;
    //column = 7 - column;

    if (color == RED || color == YELLOW || color == MAGENTA || color == WHITE) {
        CBTS_MATRIX_setLed(dev, RED_DRIVER, row, column, state);
    } else {
        CBTS_MATRIX_setLed(dev, RED_DRIVER, row, column, false);    }

    if (color == GREEN || color == YELLOW || color == CYAN || color == WHITE) {
        CBTS_MATRIX_setLed(dev, GREEN_DRIVER, row, column, state);
    } else {
        CBTS_MATRIX_setLed(dev, GREEN_DRIVER, row, column, false);
    }

    if (color == BLUE || color == MAGENTA || color == CYAN || color == WHITE) {
        CBTS_MATRIX_setLed(dev, BLUE_DRIVER, row, column, state);
    } else {
        CBTS_MATRIX_setLed(dev, BLUE_DRIVER, row, column, false);
    }
}

/* LEDs */
void CBTS_MATRIX_setLed(CBTS_MATRIX *dev, uint8_t devIdx, uint8_t coldIdx, uint8_t rowIdx, bool state)
{
    if (!dev || !dev->buffer ||
        devIdx >= CBTS_MATRIX_DEVS ||
        coldIdx >= _defaultRowBufferSize ||
        rowIdx >= maxColumns())
        return;

    if ((dev->invertedMask >> devIdx) & 0x01)
        rowIdx = maxColumns() - 1 - rowIdx;

    uint8_t index = colIndex(devIdx, rowIdx);

    if (state)
        dev->buffer[index] |= (1 << coldIdx);
    else
        dev->buffer[index] &= ~(1 << coldIdx);
}

bool CBTS_MATRIX_getLed(const CBTS_MATRIX *dev, uint8_t devIdx, uint8_t coldIdx, uint8_t rowIdx)
{
    if (!dev || !dev->buffer ||
        devIdx >= CBTS_MATRIX_DEVS ||
        coldIdx >= _defaultRowBufferSize ||
        rowIdx >= maxColumns())
        return false;

    if ((dev->invertedMask >> devIdx) & 0x01)
        rowIdx = maxColumns() - 1 - rowIdx;

    uint8_t index = colIndex(devIdx, rowIdx);
    return (dev->buffer[index] >> coldIdx) & 0x01;
}

static inline uint8_t CBTS_MATRIX_colIndex(uint8_t devIdx, uint8_t colIdx)
{
    return devIdx * _defaultColBufferSize + colIdx;
}

static void CBTS_MATRIX_write(CBTS_MATRIX *dev, uint8_t devIdx)
{
    if (!dev->buffer || devIdx >= CBTS_MATRIX_DEVS)
        return;

    uint8_t data[16];

    for (uint8_t col = 0; col < maxColumns(); col++)
    {
        uint16_t val = dev->buffer[CBTS_MATRIX_colIndex(devIdx, col)];

        data[col * 2]     = val & 0xFF;
        data[(col * 2) + 1] = (val >> 8) & 0xFF;
    }

    HAL_I2C_Mem_Write(
            CBTS_MATRIX_I2C,
            CBTS_MATRIX_addr(dev, devIdx),
            0x00,           // RAM base
            data,
            16,
            CBTS_MATRIX_TIMEOUT);
}

void CBTS_MATRIX_showSingle(CBTS_MATRIX *dev, uint8_t devIdx)
{
    CBTS_MATRIX_write(dev, devIdx);
}

void CBTS_MATRIX_show(CBTS_MATRIX *dev)
{
    if (!dev) return;

    for (uint8_t d = 0; d < CBTS_MATRIX_DEVS; d++)
        CBTS_MATRIX_showSingle(dev, d);
}

void CBTS_MATRIX_setInvertedMask(CBTS_MATRIX *dev, uint8_t invertedMask) {
    dev->invertedMask = invertedMask;
}