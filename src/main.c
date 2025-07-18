#include "chip8.h"
#include <stdio.h>

int main() {
    CHIP8* myChip8 = setupInterpreter("games/IBM_logo.ch8");
    printMemory(myChip8);
}