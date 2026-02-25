#include <Arduino.h>
#include <Wire.h>
#include <SBK_HT16K33.h>
#include "font.h"

#define RED_DRIVER              0
#define GREEN_DRIVER            1
#define BLUE_DRIVER             2

#define DRIVER_COUNT            3

#define ROWS                    16
#define COLUMNS                 8


enum LedColor {
    RED=0,
    GREEN,
    BLUE,
    YELLOW,
    CYAN,
    MAGENTA,
    WHITE
};
int textColor = RED;
const uint8_t addresses[] = {0x71, 0x74, 0x72};
const uint8_t devices[] = {RED_DRIVER, GREEN_DRIVER, BLUE_DRIVER};

SBK_HT16K33 ht(DRIVER_COUNT);


void setup() {
    delay(100);
    Wire.begin();

    for (int device = 0; device < DRIVER_COUNT; device++) {
        ht.setAddress(device, addresses[device]);
        ht.setDriverRows(device, 16);
    }

    ht.begin();

    //ht.setLed(0, 2, 3, true);
    //ht.show();

    uint8_t mask = 0b00000001;
    ht.setInvertedMask(mask);

    ht.setBrightness(6);


    ht.clear();
}

void cycleAll() {
    for (int device = 0; device < DRIVER_COUNT; device++) {
        for (int column = 0; column < COLUMNS; column++) {
            for (int row = 0; row < ROWS; row++) {
                ht.setLed(device, row, column, true);
                ht.show();
                delay(20);
            }
        }
        ht.clear();
    }
}

void setLedWithColor(int row, int column, LedColor color, bool state) {
    row = 15 - row;
    column = 7 - column;

    if (color == RED || color == YELLOW || color == MAGENTA || color == WHITE) {
        ht.setLed(RED_DRIVER, row, column, state);
    } else {
        ht.setLed(RED_DRIVER, row, column, false);
    }

    if (color == GREEN || color == YELLOW || color == CYAN || color == WHITE) {
        ht.setLed(GREEN_DRIVER, row, column, state);
    } else {
        ht.setLed(GREEN_DRIVER, row, column, false);
    }

    if (color == BLUE || color == MAGENTA || color == CYAN || color == WHITE) {
        ht.setLed(BLUE_DRIVER, row, column, state);
    } else {
        ht.setLed(BLUE_DRIVER, row, column, false);
    }
}

void cycleAllColors() {
    ht.clear();
    for (int color = RED; color < 7; color++) {

        for (int row = 0; row < 16; row++) {
            for (int column = 0; column < 8; column++) {
                setLedWithColor(row, column, static_cast<LedColor>(color), true);
            }
            ht.show();
        }
        delay(1000);
    }


}

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
                setLedWithColor(x + col, row + 1, static_cast<LedColor>(textColor), true);

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

/*void drawLetter(int8_t x) {
    for(int col=0; col<5; col++)
    {
        for(int row=0; row<7; row++)
        {
            bool pixel = A[col] & (1 << row);
            setLedWithColor(x + col, row, YELLOW, pixel);

        }

    }
    ht.show();
}*/

int getTextWidth(const char* text) {
    int len = 0;
    while(*text++)
        len++;
    return len * 6;
}

int scroll = 0;

const char message[] = "Esto es una prueba";
int textWidth = getTextWidth(message) + 16;


void loop() {
    //cycleAll();
    //cycleAllColors();

    ht.clear();
    drawText(message,scroll);
    ht.show();
    scroll++;
    if(scroll > textWidth) {
        scroll = 0;
        textColor+=1;
        if (textColor > 6) {
            textColor = RED;
        }
    }

    delay(30);
}