#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "chip8.h"
#include "display.h"

#define X   0x0f00
#define Y   0x00f0
#define N   0x000f
#define NN  0x00ff
#define NNN 0x0fff

#define MEMORY_SIZE 4096

/*
 THIS IS RELATED TO THE AMBIGUOUS INSTRUCTION 8XY6/8XYE
 0: THE OLD WAY
 1: THE MODERN WAY
*/
#define SHIFT_MODE 1

struct chip8_ {
    // 4kb memory
    uint8_t memory[4096];

    // stack (64 bytes)
    uint16_t stack[32];

    // display with monocromatic color (0 or 1)
    DISPLAY* display;
    
    // special registers
    uint16_t pc, idx;
    uint8_t sp; // points to a position in the stack array
    uint8_t delay_timer, sound_timer;

    // general-purpose registers (v[15] is the flag register)
    uint8_t v[16];
};

void processNextInstruction(CHIP8* chip8) {
    // get the instrucion pointed by PC
    uint16_t inst = (chip8->memory[chip8->pc] << 8)|(chip8->memory[chip8->pc+1]);
    chip8->pc += 2; // update PC

    uint16_t optype = 0xf000  & inst;
    optype >>= 12;

    uint8_t x = (inst & X) >> 8;
    uint8_t y = (inst & Y) >> 4;
    uint8_t n = (inst & N);
    uint8_t nn = (inst & NN);
    uint16_t nnn = (inst & NNN);

    switch (optype) {
        case 0x0:
            switch (nnn) {
                case 0x0EE: 
                    // 00EE - pops stack and sets pc to it
                    break;
                case 0x0E0:
                    // 00E0 - clear the display (sets all the pixels to 0)
                    cleanDisplay(chip8->display);
                    updateDisplay(chip8->display);
                    break;
                default:
                    break;
            }

            break;
        
        case 0x1:
            // 1NNN - sets PC to NNN
            chip8->pc = nnn;
            break;

        case 0x2:
            // 2NNN - calls the subroutine at NNN, pushing the current PC to the stack
            break;

        case 0x3:
            // 3XNN - skips one instruction if register v[X] is equal to NN
            break;

        case 0x4:
            // 4XNN - skips one instruction if register v[X] is NOT equal to NN
            break;

        case 0x5:
            // 5XY0 - skips one instruction if register v[X] is equal to v[Y]
            break;

        case 0x6:
            // 6XNN - sets the register v[X] to NN 
            chip8->v[x] = nn;
            break;

        case 0x7:
            // 7XNN - adds the value NN to v[X] (carry flag is not affected)
            chip8->v[x] += nn;
            break;

        case 0x8:
            switch (n) {
                case 0x0:
                    // 8XY0 - sets v[X] to the value of v[Y]
                    break;
                
                case 0x1:
                    // 8XY1 - sets v[X] = v[X] | v[Y]
                    break;
                
                case 0x2:
                    // 8XY2 - sets v[X] = v[X] & v[Y]
                    break;
                
                case 0x3:
                    // 8XY3 - sets v[X] = v[X] ^ v[Y]
                    break;

                case 0x4:
                    // 8XY4 - sets v[X] = v[X] + v[Y] (if overflow -> flag register is set to 1)
                    break;

                case 0x5:
                    // 8XY5 - sets v[X] = v[X] - v[Y] (if v[Y] > v[X], flag is 0, otherwise flag is 1)
                    break;

                case 0x6:
                    // 8XY6 - if !SHIFT_MODE (v[X] = v[Y]) -> v[x] >>= 1 and sets flag to the bit that was shifted out
                    break;

                case 0x7:
                    // 8XY7 - sets v[X] = v[Y] - v[X] (if v[X] > v[Y], flag is 0, otherwise flag is 1)
                    break;

                case 0xE:
                    // 8XYE - if !SHIFT_MODE (v[X] = v[Y]) -> v[x] <<= 1 and sets flag to the bit that was shifted out
                    break;

                default:
                    break;
            }
            break;

        case 0x9:
            // 9XY0 - skips one instruction if register v[X] is NOT equal to v[Y]
            break;

        case 0xA:
            // ANNN - sets idx register to NNN
            chip8->idx = nnn;
            break;

        case 0xB:
            // BNNN - (ambiguous, implementing the most common way) - jumps to address NNN plus v[0]
            break;

        case 0xC:
            // CXNN - generates a random number, ANDs with NN, and puts the result in v[X]
            break;

        case 0xD:
            // DXYN - draw to the screen (complex)
            int x_coord = chip8->v[x] & (DISPLAY_WIDTH-1);
            int y_coord = chip8->v[y] & (DISPLAY_HEIGHT-1);

            chip8->v[15] = 0;

            for(int i = 0; i < n; i++) {
                uint8_t sprite = chip8->memory[chip8->idx+i];

                for(int k = 7; k >= 0; k--) {
                    if (sprite&(1<<k)) {
                        // TURN OFF pixel color
                        if (getPixelColor(chip8->display, x_coord, y_coord)) {
                            changePixelColor(chip8->display, x_coord, y_coord, 0);
                            chip8->v[15] = 1;
                        } else { // TURN ON
                            changePixelColor(chip8->display, x_coord, y_coord, 1);
                        }
                    }

                    x_coord++;
                    if (x_coord >= DISPLAY_WIDTH) break;
                }

                // return to the beginning of the line
                x_coord = chip8->v[x] & (DISPLAY_WIDTH-1);

                y_coord++;
                if (y_coord >= DISPLAY_HEIGHT) break;
            }
            
            updateDisplay(chip8->display);

            break;
        
        case 0xE:
            switch(nn) {
                case 0x9E:
                    // EX9E - skips one instruction if the key corresponding to the value in v[X] is pressed
                    break;

                case 0xA1:
                    // EXA1 - skips one instruction if the key corresponding to the value in v[X] is NOT pressed
                    break;

                default:
                    break;
            }
            break;
            
        case 0xF:
            switch(nn) {
                case 0x07:
                    // FX07 - sets v[X] to the current value of the delay timer
                    break;

                case 0x15:
                    // FX15 - sets the delay timer to the value in v[X]
                    break;

                case 0x18:
                    // FX18 - sets the sound timer to the value in v[X]
                    break;

                case 0x1E:
                    // FX1E - sets idx = v[X] + idx (flag is set to 1 if idx overflows (> 0x0FFF) - this is ambiguous)
                    break;

                case 0x0A:
                    // FX0A - blocking instruction until a key is pressed - sets v[X] to its hex value
                    break;

                case 0x29:
                    // FX29 - sets idx to the address of the hexadecimal character in v[X]
                    break;
                
                case 0x33:
                    // FX33 - takes the number in v[X] (decimal) and places its digits in idx, idx+1, ...
                    break;
                
                case 0x55:
                    // FX55 - stores [v0, v1, ..., vx] in idx, idx+1, ..., idx+x (DONT UPDATE IDX - MODERN WAY)
                    break;
                
                case 0x65:
                    // FX65 - takes the values stored in idx, idx+1, ... , idx+x and stores in v0, v1, ..., vx (DONT UPDATE IDX - MODERN WAY)
                    break;

                default:
                    break;
            }
            break;  
        
        default:
            break;
    }
}





void setDefaultFont(CHIP8* chip8);

CHIP8* setupInterpreter(char* file_path) {
    FILE* f = fopen(file_path, "rb");
    if (f == NULL) {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        exit(1);  
    }

    CHIP8* chip8 = (CHIP8*) malloc(sizeof(CHIP8));
    
    // create the display
    chip8->display = createDisplay();

    // write a font for hex values into the beginning of the chip8 memory
    setDefaultFont(chip8);

    // set Program Counter to 0x200
    chip8->pc = 0x200;

    // inicialize timers
    chip8->delay_timer = chip8->sound_timer = 0;

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

void updateTimers(CHIP8* chip8) {
    if (chip8->delay_timer > 0) {
        chip8->delay_timer--;
    }

    if (chip8->sound_timer > 0) {
        chip8->sound_timer--;
    }
}

void freeInterpreter(CHIP8* chip8) {
    freeDisplay(chip8->display);
    free(chip8);
    return;
}

/*
    DEGUB
*/

void printMemory(CHIP8* chip8) {
    for(int i = 0; i < MEMORY_SIZE; i++) {
        printf("%d: %02X\n", i, chip8->memory[i]);
    }
}
