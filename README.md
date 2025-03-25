# gbcxx

A simple Nintendo Game Boy emulator (currently only for Linux), written in C++.

## Screenshots
| Tetris | The Legend of Zelda: Link's Awakening | Kirby's Dream Land 2 |
|---|---|---|
| ![Tetris](https://github.com/user-attachments/assets/88607530-b1b7-46fd-bac3-5641f7ccea61) | ![The Legend of Zelda: Link's Awakening]( https://github.com/user-attachments/assets/7c48d0cc-8b74-4b4e-a082-1116b0498473 ) | ![Kirby's Dream Land 2]( https://github.com/user-attachments/assets/540cfcc1-e239-4284-962f-5244e6947b11 ) |

## Running
Currently, no prebuilt binaries are provided. If you wish to build the emulator yourself, scroll down for instructions.

Usage: `gbcxx <path-to-rom>`

Some things are still unimplemented, so you can safely ignore the error logs or suppress them with the `--quiet` flag.

## Controls
- **D-Pad:** <kbd>Up</kbd> <kbd>Down</kbd> <kbd>Left</kbd> <kbd>Right</kbd>
- **Start:** <kbd>Return</kbd>
- **Select:** <kbd>Backspace</kbd>
- **A:** <kbd>X</kbd>
- **B:** <kbd>Z</kbd>


## Building
Building requires a C++23 compatible Clang or GNU compiler, CMake >= 3.21 and Ninja.

Install build dependencies:
```bash
sudo apt install cmake ninja-build
```

Clone the repository:
```bash
git clone --recurse-submodules git@github.com:elias-ka/gbcxx.git
```

Configure project:
```bash
cmake --preset release
```

Build:
```bash
cd build/gbcxx-release && ninja
```
