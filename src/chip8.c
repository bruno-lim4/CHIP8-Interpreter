#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include "chip8.h"

#define MEMORY_SIZE 4096
#define DISPLAY_WEIGHT 64
#define DISPLAY_HEIGHT 32

struct chip8_ {
    // 4kb memory
    uint8_t memory[4096];

    // stack (64 bytes)
    uint16_t stack[32];

    // display with monocromatic color (0 or 1)
    bool display[DISPLAY_HEIGHT][DISPLAY_WEIGHT];
    
    // special registers
    uint16_t pc, idx;
    uint8_t sp; // points to a position in the stack array
    uint8_t delay_timer, sound_timer;

    // general-purpose registers (v[15] is the flag register)
    uint8_t v[16];
};

void setDefaultFont(CHIP8* chip8);

void executeNextInstruction(CHIP8* chip8) {
    // fetch

}

CHIP8* setupInterpreter(char* file_path) {
    FILE* f = fopen(file_path, "rb");
    if (f == NULL) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        exit(1);  // encerra com código de erro 1
    }

    CHIP8* chip8 = (CHIP8*) malloc(sizeof(CHIP8));
    
    // write a font for hex values into the beginning of the chip8 memory
    setDefaultFont(chip8);

    // write the game file into chip8 memory
    for(int i = 0x200; i < MEMORY_SIZE; i++) {
        uint8_t data;
        if (fread(&data, sizeof(uint8_t), 1, f) == 0) {
            // there's no more data in the file
            fclose(f);
            return chip8;
        }
        chip8->memory[i] = data;
    }

    fclose(f);
    return chip8;
}

void setDefaultFont(CHIP8* chip8) {
    uint8_t font[80] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    for(int i = 0; i < 80; i++) {
        chip8->memory[i] = font[i];
    }
}

/*
    DEGUB
*/

void printMemory(CHIP8* chip8) {
    for(int i = 0; i < MEMORY_SIZE; i++) {
        printf("%d: %02X\n", i, chip8->memory[i]);
    }
}
