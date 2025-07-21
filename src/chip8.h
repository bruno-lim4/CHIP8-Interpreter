#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>

#include <SDL2/SDL.h>

typedef struct chip8_ CHIP8;

CHIP8* setupInterpreter(char* file_path);
void processNextInstruction(CHIP8* chip8);
void updateTimers(CHIP8* chip8);
void freeInterpreter(CHIP8* chip8);

void printMemory(CHIP8* chip8);

#endif