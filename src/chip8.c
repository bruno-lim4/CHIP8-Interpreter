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
 THIS IS RELATED TO SOME AMBIGUOUS INSTRUCTIONS (QUIRKS)
 0: THE OLD WAY
 1: THE MODERN WAY
*/
#define SHIFT_MODE 0
#define FLAG_RESET 0
#define IDX_MODE 0
#define JUMP_MODE 0

const int commands[16] = {SDL_SCANCODE_X, SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_Z,SDL_SCANCODE_C, SDL_SCANCODE_4, SDL_SCANCODE_R, SDL_SCANCODE_F, SDL_SCANCODE_V};

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
 
    bool waiting_key, needs_to_draw; 
    uint8_t key_register;
};

void drawDisplay(CHIP8* chip8) {
    updateDisplay(chip8->display);
    chip8->needs_to_draw = false;
}

bool needsToDraw(CHIP8* chip8) {
    return chip8->needs_to_draw;
}

bool waitingForKey(CHIP8* chip8) {
    return chip8->waiting_key;
}

void handleKeyPressed(CHIP8* chip8, SDL_Event* event) {
    if ((*event).type == SDL_KEYDOWN) {
        SDL_Scancode sc = (*event).key.keysym.scancode;
        //printf("Scancode: %d (%s)\n", sc, SDL_GetScancodeName(sc));
        for(int i = 0; i < 16; i++) {
            if (commands[i] == sc) {
                chip8->v[chip8->key_register] = i;
                break;
            }
        }
    }

    chip8->waiting_key = false;
}

void processNextInstruction(CHIP8* chip8) {
    // get the instrucion pointed by PC
    uint16_t inst = (chip8->memory[chip8->pc] << 8)|(chip8->memory[chip8->pc+1]);

    // program has ended
    if (inst == 0) {
        SDL_Event quit_event;
        quit_event.type = SDL_QUIT;
        SDL_PushEvent(&quit_event);
        return;
    }

    //printf("pc: %d; inst: %04X\n", chip8->pc, inst);
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
                    chip8->pc = chip8->stack[chip8->sp];
                    chip8->sp++;
                    break;
                case 0x0E0:
                    // 00E0 - clear the display (sets all the pixels to 0)
                    cleanDisplay(chip8->display);
                    chip8->needs_to_draw = true;
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
            chip8->sp--;
            chip8->stack[chip8->sp] = chip8->pc;
            chip8->pc = nnn;
            break;

        case 0x3:
            // 3XNN - skips one instruction if register v[X] is equal to NN
            if (chip8->v[x] == nn) chip8->pc += 2;
            break;

        case 0x4:
            // 4XNN - skips one instruction if register v[X] is NOT equal to NN
            if (chip8->v[x] != nn) chip8->pc += 2;
            break;

        case 0x5:
            // 5XY0 - skips one instruction if register v[X] is equal to v[Y]
            if (chip8->v[x] == chip8->v[y]) chip8->pc += 2;
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
                    chip8->v[x] = chip8->v[y];
                    break;
                
                case 0x1:
                    // 8XY1 - sets v[X] = v[X] | v[Y]
                    chip8->v[x] |= chip8->v[y];
                    if (FLAG_RESET == 0) chip8->v[15] = 0;
                    break;
                
                case 0x2:
                    // 8XY2 - sets v[X] = v[X] & v[Y]
                    chip8->v[x] &= chip8->v[y];
                    if (FLAG_RESET == 0) chip8->v[15] = 0;
                    break;
                
                case 0x3:
                    // 8XY3 - sets v[X] = v[X] ^ v[Y]
                    chip8->v[x] ^= chip8->v[y];
                    if (FLAG_RESET == 0) chip8->v[15] = 0;
                    break;

                case 0x4:
                    // 8XY4 - sets v[X] = v[X] + v[Y] (if overflow -> flag register is set to 1)
                    uint16_t res = chip8->v[x] + chip8->v[y];
                    chip8->v[x] += chip8->v[y];
                    if (res > 0xff) chip8->v[15] = 1;
                    else chip8->v[15] = 0;

                    break;

                case 0x5:
                    // 8XY5 - sets v[X] = v[X] - v[Y] (if v[Y] > v[X], flag is 0, otherwise flag is 1)
                    if (chip8->v[y] > chip8->v[x]) {
                        chip8->v[x] -= chip8->v[y]; 
                        chip8->v[15] = 0;
                    } else {
                        chip8->v[x] -= chip8->v[y];
                        chip8->v[15] = 1;
                    }
                    
                    break;

                case 0x6:
                    // 8XY6 - if !SHIFT_MODE (v[X] = v[Y]) -> v[x] >>= 1 and sets flag to the bit that was shifted out
                    if (!SHIFT_MODE) chip8->v[x] = chip8->v[y];
                    chip8->v[15] = 1&chip8->v[x];
                    if (x == 15) break;
                    chip8->v[x] >>= 1;
                    break;

                case 0x7:
                    // 8XY7 - sets v[X] = v[Y] - v[X] (if v[X] > v[Y], flag is 0, otherwise flag is 1)
                    if (chip8->v[x] > chip8->v[y]) {
                        chip8->v[x] = chip8->v[y] - chip8->v[x];
                        chip8->v[15] = 0;
                    } else {
                        chip8->v[x] = chip8->v[y] - chip8->v[x];
                        chip8->v[15] = 1;
                    }
                    break;

                case 0xE:
                    // 8XYE - if !SHIFT_MODE (v[X] = v[Y]) -> v[x] <<= 1 and sets flag to the bit that was shifted out
                    if (!SHIFT_MODE) chip8->v[x] = chip8->v[y];
                    chip8->v[15] = ((1<<7)&chip8->v[x]) >> 7;
                    if (x == 15) break;
                    chip8->v[x] <<= 1;
                    break;

                default:
                    break;
            }
            break;

        case 0x9:
            // 9XY0 - skips one instruction if register v[X] is NOT equal to v[Y]
            if (chip8->v[x] != chip8->v[y]) chip8->pc += 2;
            break;

        case 0xA:
            // ANNN - sets idx register to NNN
            chip8->idx = nnn;
            break;

        case 0xB:
            // BNNN - (ambiguous, implementing the most common way) - jumps to address NNN plus v[0]
            chip8->pc = nnn + chip8->v[0];
            if (JUMP_MODE == 1) chip8->pc += chip8->v[x]; 
            break;

        case 0xC:
            // CXNN - generates a random number, ANDs with NN, and puts the result in v[X]
            chip8->v[x] = nn & rand();
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
            
            chip8->needs_to_draw = true;

            break;
        
        case 0xE:
            const uint8_t *keystate = SDL_GetKeyboardState(NULL);
            switch(nn) {
                case 0x9E:
                    // EX9E - skips one instruction if the key corresponding to the value in v[X] is pressed
                    if (keystate[commands[chip8->v[x]]]) chip8->pc += 2;
                    break;

                case 0xA1:
                    // EXA1 - skips one instruction if the key corresponding to the value in v[X] is NOT pressed
                    if (!keystate[commands[chip8->v[x]]]) chip8->pc += 2;
                    break;

                default:
                    break;
            }
            break;
            
        case 0xF:
            switch(nn) {
                case 0x07:
                    // FX07 - sets v[X] to the current value of the delay timer
                    chip8->v[x] = chip8->delay_timer;
                    break;

                case 0x15:
                    // FX15 - sets the delay timer to the value in v[X]
                    chip8->delay_timer = chip8->v[x];
                    break;

                case 0x18:
                    // FX18 - sets the sound timer to the value in v[X]
                    chip8->sound_timer = chip8->v[x];
                    break;

                case 0x1E:
                    // FX1E - sets idx = v[X] + idx (flag is set to 1 if idx overflows (> 0x0FFF) - this is ambiguous)
                    chip8->idx += chip8->v[x];
                    if (chip8->idx > 0x0FFF) chip8->v[15] = 1;
                    break;

                case 0x0A:
                    // FX0A - blocking instruction until a key is pressed - sets v[X] to its hex value

                    chip8->waiting_key = true;
                    chip8->key_register = x;

                    SDL_Event event;
                    while(true) {
                        if (SDL_WaitEvent(&event)) {
                            if (event.type == SDL_QUIT) {
                                SDL_PushEvent(&event);
                                break;
                            } else if (event.type == SDL_KEYDOWN) {
                                SDL_Scancode sc = event.key.keysym.scancode;
                                //printf("Scancode: %d (%s)\n", sc, SDL_GetScancodeName(sc));
                                for(int i = 0; i < 16; i++) {
                                    if (commands[i] == sc) {
                                        chip8->v[x] = i;
                                        break;
                                    }
                                }
                                break;
                            }
                        }
                    }
                    break;

                case 0x29:
                    // FX29 - sets idx to the address of the hexadecimal character in v[X]
                    uint8_t hex_value = chip8->v[x];
                    chip8->idx = hex_value*5;
                    break;
                
                case 0x33:
                    // FX33 - takes the number in v[X] (decimal) and places its digits in idx, idx+1, ...
                    uint8_t decimal_value = chip8->v[x];
                    uint8_t digits[3] = {0, 0, 0}; int k = 0;

                    while(decimal_value > 0) {
                        uint8_t d = decimal_value%10;
                        decimal_value /= 10;
                        digits[k] = d;
                        k += 1; 
                    }

                    int aux = 0;
                    for(int i = 2; i >= 0; i--) {
                        chip8->memory[chip8->idx+aux] = digits[i];
                        aux++;
                    }

                    break;
                
                case 0x55:
                    // FX55 - stores [v0, v1, ..., vx] in idx, idx+1, ..., idx+x (DONT UPDATE IDX - MODERN WAY)
                    for(int i = 0; i <= x; i++) {
                        chip8->memory[chip8->idx+i] = chip8->v[i];
                    }
                    if (IDX_MODE == 0) chip8->idx += x+1;
                    break;
                
                case 0x65:
                    // FX65 - takes the values stored in idx, idx+1, ... , idx+x and stores in v0, v1, ..., vx (DONT UPDATE IDX - MODERN WAY)
                    for(int i = 0; i <= x; i++) {
                        chip8->v[i] = chip8->memory[chip8->idx+i];
                    }
                    if (IDX_MODE == 0) chip8->idx += x+1;
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

    // set stack pointer to 32
    chip8->sp = 32;

    // inicialize timers
    chip8->delay_timer = chip8->sound_timer = 0;


    chip8->waiting_key = chip8->needs_to_draw = false;

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
