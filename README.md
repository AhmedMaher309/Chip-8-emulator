# Chip-8 Emulator
## Overview
This repository contains a Chip8 emulator, a program that simulates the behavior of a Chip8 virtual machine. The Chip8 is an interpreted programming language developed in the mid-1970s for early microcomputers. It was designed to simplify the development of games and applications for these systems.

## ![Review](https://github.com/AhmedMaher309/Chip-8-emulator/blob/main/assets/tetris.gif)

## ![Architecture](https://github.com/AhmedMaher309/Chip-8-emulator/blob/main/assets/Screenshot%20from%202024-02-10%2019-56-53.png)


## Features
- The emulator accurately emulates the Chip8 virtual machine, including its CPU instructions, memory layout, and display rendering.
- It supports loading and executing Chip8 ROMs, which are programs written for the Chip8 platform.
- The emulator provides a graphical display of the Chip8 screen and allows user input via a keyboard.


## How to run 
#### Find games [here](https://github.com/dmatlack/chip8/tree/master/roms/games)

    git clone https://github.com/AhmedMaher309/Chip-8-emulator.git
    create a build directory to run the cmake in it and type <cmake ..>
    add a rom file for the game you want to play
    type:  ./chip8 <dimenstion> <delay> game.ch8
    example: ./chip8 15 3 tetris.ch8
    

### Keyboard mapping for the emulator
    Keypad in Chip-8       Keyboard
    +-+-+-+-+             +-+-+-+-+
    |1|2|3|C|             |1|2|3|4|
    +-+-+-+-+             +-+-+-+-+
    |4|5|6|D|             |Q|W|E|R|
    +-+-+-+-+ ========>   +-+-+-+-+
    |7|8|9|E|             |A|S|D|F|
    +-+-+-+-+             +-+-+-+-+
    |A|0|B|F|             |Z|X|C|V|
    +-+-+-+-+             +-+-+-+-+

## Implementation Overview
1- Each function represents an instruction with a matching opcode

2- You can find a cheat sheet for instructions [here](https://github.com/AhmedMaher309/Chip-8-emulator/blob/main/assets/chip8ref.pdf)

3- GLAD library files have been included and are accessed directly from the local directory

    



