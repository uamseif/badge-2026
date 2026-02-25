#include <stdio.h>
#include <ch32v00x.h>
#include "cbts_matrix.h"
#include "font.h"

void Delay_Init(void);
void Delay_Ms(uint32_t n);

#include "i2c_hal.h"
#include "gpio_hal.h"

#define I2C_SPEED_400KHZ 400000
#define I2C_MASTER_MY_ADDRESS 0x00

int textColor = RED;


CBTS_MATRIX display;

int fontIndex(char c) {
    if(c == ' ') return 0;
    if(c >= '0' && c <= '9') return 1 + (c - '0');
    if(c >= 'A' && c <= 'Z') return 11 + (c - 'A');
    if(c >= 'a' && c <= 'z') return 37 + (c - 'a');
    return 0;
}

void drawChar(char c,int x) {
    int idx = fontIndex(c);

    for(int col=0; col<5; col++)
    {
        uint8_t line = font5x7[idx][col];
        for(int row=0; row<7; row++)
        {
            if(line & (1<<row))
                CBTS_MATRIX_setLedWithColor(&display, x + col, row + 1, textColor, true);
        }
    }
}

void drawText(const char* text,int offset)
{
    int x = 16 - offset;

    while(*text)
    {
        drawChar(*text,x);
        x += 6;
        text++;
    }
}

int getTextWidth(const char* text) {
    int len = 0;

    while(*text++)
        len++;
    return len * 6;
}

int scroll = 0;

const char message[] = "Cibertracks 2026 - 29 Mayo";
int textWidth = 0;

void cycleAllColors() {
    CBTS_MATRIX_clear(&display);
    for (int color = RED; color < 7; color++) {

        for (int row = 0; row < 16; row++) {
            for (int column = 0; column < 8; column++) {
                CBTS_MATRIX_setLedWithColor(&display, row, column, color, true);
            }
            CBTS_MATRIX_show(&display);
        }
        HAL_delay_1ms(500);
    }
}
bool test = false;

void MyKeyHandler(uint16_t key, uint8_t state) {
    if(key == HAL_KEY_SW_1) {
        if(state == HAL_KEY_EVENT_DOWN) {
            //printf("KEY1 DOWN\n");
        }

        if(state == HAL_KEY_EVENT_UP) {
            //printf("KEY1 SHORT\n");
        }

        if(state == HAL_KEY_EVENT_LONG) {
            //printf("KEY1 LONG\n");
        }
    }
}


int main(void) {
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();

    HAL_Systick_Init();
    HAL_I2C_Init(I2C_SPEED_400KHZ, I2C_MASTER_MY_ADDRESS);
    HAL_KeyInit();
    HalKeyConfig(MyKeyHandler);

    CBTS_MATRIX_init(&display);
    CBTS_MATRIX_begin(&display);

    textWidth = getTextWidth(message) + 16;

    for (int i = 0; i < 2; i++) {
        cycleAllColors();
    }

    while (1)
    {

        uint8_t keys = HalKeyRead();
        HAL_KeyPoll();
        CBTS_MATRIX_clear(&display);
        drawText(message,scroll);

        if (test) {
            CBTS_MATRIX_setLed(&display, RED_DRIVER, 1, 7, true);
        }
        CBTS_MATRIX_setLed(&display, RED_DRIVER, 0, 7, (keys >> 0) & 0x01);
        CBTS_MATRIX_setLed(&display, RED_DRIVER, 1, 7, (keys >> 1) & 0x01);
        CBTS_MATRIX_setLed(&display, RED_DRIVER, 2, 7, (keys >> 2) & 0x01);
        CBTS_MATRIX_setLed(&display, RED_DRIVER, 3, 7, (keys >> 3) & 0x01);
        CBTS_MATRIX_setLed(&display, RED_DRIVER, 4, 7, (keys >> 4) & 0x01);


        CBTS_MATRIX_show(&display);
        scroll++;
        if(scroll > textWidth) {
            scroll = 0;
            textColor+=1;
            if (textColor > 6) {
                textColor = RED;
            }
        }
        HAL_delay_1ms(50);
    }
}
