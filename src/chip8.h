#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>

typedef struct chip8_ CHIP8;

CHIP8* setupInterpreter(char* file_path);
void freeInterpreter(CHIP8* chip8);

void printMemory(CHIP8* chip8);

#endif