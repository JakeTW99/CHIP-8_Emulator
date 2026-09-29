// SDL3 = Simple DirectMedia Layer, Version 3
// open-source, cross-platform development library used in multimedia programming
#include <SDL3/SDL.h>
#include "Chip8.h"
#include <iostream>
// Used to measure real-world time between frames
#include <chrono>
// Used to control the timing of the emulation loop
#include <thread>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 320;

// This function is used to make sure that the fontset characters are correctly stored and loaded
void FontSetCheck(const uint8_t* memory, int address) {
    
    // Each character is 5 bytes tall
    for (int row = 0; row < 5; ++row) {
        uint8_t byte = memory[address + row];

        // Each byte is 8 bits wide; 8x5 pixels per character
        for (int col = 0; col < 8; ++col) {
            //Checks to see if the current bit in the byte is "1" or "0".
            if ((byte << col) & 0x80) {
                std::cout << "*";
            } else {
                std::cout << " ";
            }
        }
        // Moves to the next line after printing each row of the character
        std::cout << std::endl;
    }
}

void RenderDisplay(SDL_Renderer* renderer, SDL_Texture* texture, const uint8_t* display) {
    
    // Moves cursor to the top-left corner of the console window
    //std::cout << "\x1B[H";

    uint32_t pixels[64 * 32];

    //Debugging: Displays the frame buffer in the console window
    //std::cout << "\n--- FRAME START ---\n";
    //for (int i = 0; i < 66; ++i) std::cout << "#";
    //std::cout << "\n";
    
    for (int y = 0; y < 32; ++y) {

        //std::cout << "#";

        for (int x = 0; x < 64; ++x) {
            int index = x + (y * 64);
            // 0xFFFFFFFF = white pixel; 0x000000FF = black pixel
            pixels[index] = (display[index] == 1) ? 0xFFFFFFFF : 0x000000FF;
        }
        //std::cout << "#" <<std::endl;
    }

    //for (int i = 0; i < 66; ++i) std::cout << "#";
    //std::cout << "\n--- FRAME END ---\n";

    // Moves pixels array from CPU memory (RAM) to GPU memory (VRAM)
    SDL_UpdateTexture(texture, NULL, pixels, 64 * sizeof(uint32_t));
    // Clears the screen
    SDL_RenderClear(renderer);
    // Draws texture onto canvas
    // 1st NULL = Source Rectangle; Uses the full 64x32 texture area
    SDL_RenderTexture(renderer, texture, NULL, NULL);
    // 2nd NULL = Destination Rectangle; Stretches the 64x32 texture to fill 640x320 window
    SDL_RenderPresent(renderer);
}

// argc = argument count; argv = Argument Vector; determines the amount of command-line arguments are being passed to
// the program and then an array of strings are created to represent each individual argument.
int main(int argc, char* argv[]) {
    
    // if the argc is less than "2", it usually means the user launched the executable without a ROM file path
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <path-to-rom>" << std::endl;
        return 1;
    }

    //If SDL's graphic initialization fails, this message will display.
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL Initialization Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("CHIP-8 Emulator", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        64, 32
    );
    // Removes blurriness to create sharp pixel edges
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    
    // Creates an instance of the Chip8 class.
    Chip8 myChip8;

    //Executes function and then tests it
    if (!myChip8.LoadROM(argv[1])) {
        std::cerr << "Failed to load ROM" << std::endl;
        return 1;
    }
    
    bool game_is_running = true;
    SDL_Event event;

    // 11 instructions targets the standard CHIP-8 CPU execution speed of ~700 Hz
    // 700Hz / 60 FPS = ~11.66 instructions per frame
    const int INSTRUCTIONS_PER_FRAME = 11;
    // Equal to 60 FPS
    const std::chrono::microseconds FRAME_TARGET_TIME(16666);

    while (game_is_running) {
        //'auto' keyword instructs compiler to automatically deduce a variable's data type at compile time
        auto frame_start = std::chrono::high_resolution_clock::now();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                game_is_running = false;
            } else if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {

                if (event.key.key == SDLK_ESCAPE) {
                    game_is_running = false;
                }
                // SDL_Event -> .key (SDL_KeyboardEvent) -> .key(SDL_Keycode Value: SDLK_ESCAPE)
                bool isPressed = (event.type == SDL_EVENT_KEY_DOWN);

                switch (event.key.key) {
                    case SDLK_1: myChip8.SetKeyState(0x1, isPressed); break;
                    case SDLK_2: myChip8.SetKeyState(0x2, isPressed); break;
                    case SDLK_3: myChip8.SetKeyState(0x3, isPressed); break;
                    case SDLK_4: myChip8.SetKeyState(0xC, isPressed); break;
                    case SDLK_Q: myChip8.SetKeyState(0x4, isPressed); break;
                    case SDLK_W: myChip8.SetKeyState(0x5, isPressed); break;
                    case SDLK_E: myChip8.SetKeyState(0x6, isPressed); break;
                    case SDLK_R: myChip8.SetKeyState(0xD, isPressed); break;
                    case SDLK_A: myChip8.SetKeyState(0x7, isPressed); break;
                    case SDLK_S: myChip8.SetKeyState(0x8, isPressed); break;
                    case SDLK_D: myChip8.SetKeyState(0x9, isPressed); break;
                    case SDLK_F: myChip8.SetKeyState(0xE, isPressed); break;
                    case SDLK_Z: myChip8.SetKeyState(0xA, isPressed); break;
                    case SDLK_X: myChip8.SetKeyState(0x0, isPressed); break;
                    case SDLK_C: myChip8.SetKeyState(0xB, isPressed); break;
                    case SDLK_V: myChip8.SetKeyState(0xF, isPressed); break;

                }
            }
        }

        for (int i = 0; i < INSTRUCTIONS_PER_FRAME; ++i) {
            myChip8.EmulateCycle();
        }

        myChip8.UpdateTimers();

        RenderDisplay(renderer, texture, myChip8.GetDisplay());

        // Caps Frame Rate to 60 fps
        auto frame_time = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - frame_start
        );
        if (frame_time < FRAME_TARGET_TIME) {
            std::this_thread::sleep_for(FRAME_TARGET_TIME - frame_time);
        }

    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
