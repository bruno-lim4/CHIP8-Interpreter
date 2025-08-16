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

## Key Mapping

The CHIP-8 uses a 16-key hexadecimal keypad.  
This interpreter maps the CHIP-8 keys to your keyboard as follows:

| CHIP-8 Key | Keyboard Key |
|:----------:|:------------:|
| 1          | 1            |
| 2          | 2            |
| 3          | 3            |
| C          | 4            |
| 4          | Q            |
| 5          | W            |
| 6          | E            |
| D          | R            |
| 7          | A            |
| 8          | S            |
| 9          | D            |
| E          | F            |
| A          | Z            |
| 0          | X            |
| B          | C            |
| F          | V            |

## Project Structure

- `src/` — Source code
- `games/` — CHIP-8 ROMs for testing