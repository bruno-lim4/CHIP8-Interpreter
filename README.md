# CHIP-8 Interpreter

This project is a CHIP-8 interpreter written in C, using SDL2 for graphics.

## Requirements

- GCC
- SDL2 development libraries

To install SDL2 on Linux (Ubuntu/Debian), run:
```bash
sudo apt-get install libsdl2-dev
```

## How to Build

Open a terminal in the project folder and run:
```bash
make
```

This will create the executable `chip8`.

To clean up build files:
```bash
make clean
```

## How to Run

Run the interpreter with the path to a CHIP-8 ROM:
```bash
./chip8 games/PONG
```

## Project Structure

- `src/` — Source code
- `games/` — CHIP-8 ROMs for testing