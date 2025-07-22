#include "chip8.h"
#include "display.h"
#include <stdio.h>
#include <SDL2/SDL.h>
#include <time.h>

#define INSTRUCIONS_PER_SECOND 100

uint32_t callback(uint32_t interval, void *param) {
    printf("FUI CHAMADO\n");
    updateTimers((CHIP8*) param);
    fflush(stdout);
    return interval;
}

int main() {
    CHIP8* interpreter = setupInterpreter("games/Airplane.ch8");

    srand(time(NULL));
    SDL_AddTimer(1000/60, callback, interpreter);

    double loop_delay = 1000/INSTRUCIONS_PER_SECOND;

    SDL_Event event;
    int running = 1;
    while (running) {
        uint32_t start_time = SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // process instruction and updates screen if necessary
        processNextInstruction(interpreter);

        uint32_t t = SDL_GetTicks() - start_time;
        if (t < loop_delay) {
            SDL_Delay(loop_delay - t);
        }
    }

    freeInterpreter(interpreter);
    SDL_Quit();
}