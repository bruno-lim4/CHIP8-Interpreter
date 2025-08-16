#include "chip8.h"
#include "display.h"
#include <stdio.h>
#include <SDL2/SDL.h>
#include <time.h>

#define CPU_HZ 700.0
#define DISPLAY_HZ 60.0
#define TIMERS_HZ 60.0

int main(int argc, char* argv[]) {
    srand(time(NULL));

    const double cpu_interval_ms = 1000.0/CPU_HZ;
    const double display_interval_ms = 1000.0/DISPLAY_HZ;
    const double timers_interval_ms = 1000.0/TIMERS_HZ;

    double cpu_accumulator, display_accumulator, timers_accumulator;
    cpu_accumulator = display_accumulator = timers_accumulator = 0.0;

    CHIP8* interpreter = setupInterpreter(argv[1]);

    SDL_Event event;
    int running = 1;

    uint32_t last_time = SDL_GetTicks();

    while (running) {
        // check for QUIT EVENT or KEY_DOWN
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYUP && waitingForKey(interpreter))
                handleKeyPressed(interpreter, &event);
        }

        uint32_t current_time = SDL_GetTicks();

        // calculates elapsed time for better sinc
        double elapsed_time = current_time - last_time;
        last_time = current_time;

        // update accumulators
        cpu_accumulator += elapsed_time;
        display_accumulator += elapsed_time;
        timers_accumulator += elapsed_time;

        // CPU
        while(cpu_accumulator >= cpu_interval_ms) {
            if (!waitingForKey(interpreter))
                processNextInstruction(interpreter);
            cpu_accumulator -= cpu_interval_ms;
        }

        // DISPLAY
        while(display_accumulator >= display_interval_ms) {
            if (needsToDraw(interpreter))
                drawDisplay(interpreter);
            display_accumulator -= display_interval_ms;
        }

        // TIMERS
        while(timers_accumulator >= timers_interval_ms) {
            updateTimers(interpreter);
            timers_accumulator -= timers_interval_ms;
        }

        SDL_Delay(1);
    }

    freeInterpreter(interpreter);
    SDL_Quit();
}