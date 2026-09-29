#include "Chip8.h"
#include <fstream>
#include <iostream>

const uint8_t Chip8::fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

//Constructor to initialize default values/states
Chip8::Chip8() {
    for (int i = 0; i < 4096; i++) {
        memory[i] = 0;
    }

    for (int i = 0; i < 16; i++) {
        registers[i] = 0;
    }

    index_register = 0;
    /* Program counter starts at 0x200 (hexadecimal) or 512 bytes in decimal.
    The first 512 bytes of memory should be reserved for the CHIP-8 interpreter itself.
    */
    program_counter = 0x200;

    for (int i = 0; i < 16; i++) {
        stack[i] = 0;
        keypad[i] = 0;
    }

    stack_pointer = 0;
    delay_timer = 0;
    sound_timer = 0;

    for (int i = 0; i < 64 * 32; i++) {
        display[i] = 0;
    }

    for (int i = 0; i < 80; ++i) {
        //The Chip8:: reference is used to access the private static member that is declared in the Chip8 class in the header file.
        this->memory[i] = Chip8::fontset[i];
    }
}

// Core Function responsible for loading a ROM file into the emulator's memory.
bool Chip8::LoadROM(const char* filename) {
    
    // The second argument "rb" stands for "read binary"; the mode in which the file is opened.
    FILE* file = fopen(filename, "rb");

    // Checks to see if the file can be successfully opened. Trying to read an inaccessible file could lead to the emulator crashing.
    if (file == nullptr) {
        std::cerr << "Failed to open ROM: " << filename << std::endl;
        return false;
    }


    //fseek function is used to move the file pointer. To determine the size of a ROM file, we must move to the end of the file.
    // "file" is a pointer to the file that we want to read;
    // "0" refers to the number of bytes to offset from the starting point; since we are jumping to the end of the file, we do need to offset any bytes.
    // The third argument, "SEEK_END", is our starting point, so the end of the file.
    fseek(file, 0, SEEK_END);
    //ftell function returns the current file position; we moved to the end of the file with the fseek function.
    long file_size = ftell(file);
    // Rewinds the file pointer back to the beginning for reading the ROM data into memory.
    rewind(file);

    // The total memory size of our CHIP-8 emulator is 4096 bytes, but the first 512 bytes are reserved for the CHIP-8 interpreter itself.
    if (file_size > (4096 - 512)) {
        std::cerr << "ROM size exceeds available memory." << std::endl;
        fclose(file);
        return false;
    }

    //Reads the ROM data from the file and stores it in the emulator's memory.
    //fread function breakdown:
    // Destination: "memory + 512" is the destination buffer where the ROM data will be stored
    // Size of Each Element: "sizeof(uint8_t)" is the size of each element to be read; since we are reading bytes, this is 1 byte.
    // Number of Elements: "file_size" is the total number of bytes to read from the file.
    // Source: "file" is the pointer to the file from which we are reading.
    size_t bytes_read = fread(memory + 512, sizeof(uint8_t), file_size, file);
    fclose(file);
    //Good practice to check if the entire ROM was read successfully.
    if (bytes_read != file_size) {
        std::cerr << "Failed to read the entire ROM." << std::endl;
        return false;
    }

    return true;
    
}

void Chip8::EmulateCycle() {
    // Fetches the next instruction/opcode from memory.
    // Opcodes in CHIP-8 are 2 bytes long (16 bits), so we must combine two consecutive bytes.
    // The first byte is shifted left by 8 bits, then the bitwise OR operator (|) is used to combine it with the second byte.
    // Addition is not ideal because it could lead to overflow issues; bitwise operations are designed for these purposes.
    uint16_t opcode = (memory[program_counter] << 8) | memory[program_counter + 1];

    // Debugging: Prints the current program counter and opcode in hexadecimal format to the console.
    //std::cout << "PC: " << std::hex << program_counter << " | Opcode: " << opcode << std::endl;

    //Checks to see if the program counter has been incremented by a jump instruction
    bool pc_incremented_by_jump = false;

    // Massive switch statement to decode and execute opcodes
    
    /*Placeholder Cheat Sheet for Opcode Implementation:
        X - destination register
        Y - source register
        N - 4-bit constant; nibble
        NN - 8-bit constant; byte
        NNN - 12-bit constant; address
    */
    
    
    // The opcode is "masked" with 0xF000 to isolate the first hexadecimal digit; the first four bits of the opcode determine the instruction type.
    /// "F" in hexadecimal is "1111" in binary, so the bitwise AND operation with 0xF000 will zero out all bits except for the first four bits of the opcode.
    switch (opcode & 0xF000) {
        //Opcode starts with 0x0.
        case 0x0000:
            
            switch (opcode & 0x00FF) {
                case 0x00E0: //Clear the display
                    for (int i = 0; i < 64 * 32; i++) {
                        display[i] = 0;
                    }
                    break;
                case 0x00EE: //Return from a subroutine
                if (stack_pointer > 0) {
                    stack_pointer--;
                    program_counter = stack[stack_pointer];
                    pc_incremented_by_jump = true;
                } else {
                    std::cerr << "Stack underflow: No subroutine to return from." << std::endl;
                    return;
                }

                break;

            default:
                //std::hex prints the opcode in hexadecimal format.
                std::cerr << "Unknown opcode: " << std::hex << opcode << std::endl;
            }
            break;

        //1NNN - Any opcode that starts with 0x1 is a jump instruction; NNN represents/are held for the
        // three remaining hexadecimal digits of the opcode, which specify the address to jump to.
        case 0x1000: { // 1NNN - Jump to address NNN
            program_counter = opcode & 0x0FFF;
            pc_incremented_by_jump = true;
            break;
        }

        case 0x2000: { // 2NNN - Call subroutine at address NNN and store the current program counter on the stack.
            if (stack_pointer < 16) {
                stack[stack_pointer] = program_counter + 2;
                stack_pointer++;
                program_counter = opcode & 0x0FFF;
                pc_incremented_by_jump = true;
            } else {
                std::cerr << "Stack overflow: Maximum stack depth exceeded." << std::endl;
                return;
            }
            break;
        }

        case 0x3000: { // 3XNN - Skip the next instruction if register VX equals NN
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t nn = (opcode & 0x00FF);
            if(registers[x] == nn) {
                program_counter += 2;
            }
            break;
        }

        case 0x4000: { // 4XNN - Skip the next instruction if register VX does not equal NN
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t nn = (opcode & 0x00FF);
            if(registers[x] != nn) {
                program_counter += 2;
            }
            break;
        }

        case 0x5000: { // 5XY0 - Skip the next instruction if destination register VX equals source register VY
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t y = (opcode & 0x00F0) >> 4;
            if(registers[x] == registers[y]) {
                program_counter += 2;
            }
            break;
        }

        case 0x6000: { // 6XNN - Set register VX to NN
            //V refers to the 16 registers; X specifies which register we are working with
            //NN; the last two hexadecimal digits of the opcode, represent the value to be loaded into the register.
            uint8_t x = (opcode & 0x0F00) >> 8; // Shifting right by 8 bits to get a value between 0 and 15, which corresponds to the register index.
            uint8_t nn = (opcode & 0x00FF);
            registers[x] = nn;
            break;
        }

        case 0x7000: { // 7XNN - Add NN to register VX (without carry flag)
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t nn =(opcode & 0x00FF);
            registers[x] += nn;
            break;
        }

        // The 0x8000 opcodes are used for arithmetic and logical operations between registers.

        // Bitwise Operations Cheat Sheet:
        // AND (&): Sets each bit to 1 if both bits are 1. (Filter/Mask)
        // OR (|): Sets each bit to 1 if at least one of the bits is 1. (Combining)
        // XOR (^): Sets each bit to 1 if the bits are different. (Toggling)
        // Shifts(>> and <<): Move bits to the left or right

        case 0x8000: {
            // Get the destination (VX) and source (VY)registers and store them in their respective variables.
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t y = (opcode & 0x00F0) >> 4;
            switch (opcode & 0x000F) {
                // 8XY0 - Set VX to the value of VY
                case 0x0000:
                    registers[x] = registers[y];
                break;

                // 8XY1 - Set VX to VX OR VY (Combining)
                case 0x0001:
                    registers[x] |= registers[y];
                break;

                // 8XY2 - Set VX to VX AND VY (Filtering/Masking)
                case 0x0002:
                    registers[x] &= registers[y];
                break;

                // 8XY3 - Set VX to VX XOR VY (Toggling)
                case 0x0003:
                    registers[x] ^= registers[y];
                break;

                // 8XY4 - Add VY to VX. VF acts as a carry flag if the sum exceeds 255 (8 bits)
                // That is also why the sum is stored in a 16-bit variable; to check for overflow
                case 0x0004: {
                    uint16_t sum = registers[x] + registers[y];
                    uint8_t carry = (sum > 0xFF) ? 1 : 0;

                    registers[x] = static_cast<uint8_t>(sum & 0xFF);
                    registers[0xF] = carry;
                    break;
                }

                // 8XY5 - Subtract VY from VX
                // VF is set to 0 if there is a borrow and 1 if there is not
                case 0x0005: {
                    
                    uint8_t noBorrow = (registers[x] >= registers[y]) ? 1 : 0;
                    uint16_t diff = registers[x] - registers[y];

                    registers[x] = diff;
                    registers[0xF] = noBorrow;
                    break;
                }

                // 8XY6 - Shift VX right by one
                // VF is set to the least significant bit of VX before the shift
                // The bit stored in VF acts as a kind of "paper trail" (e.g. collision detection)
                case 0x0006: {
                    uint8_t lsb = (registers[y] & 0x01);
                    registers[x] = registers[y] >> 1;
                    registers[0xF] = lsb;
                    break;
                }
                
                // 8XY7 - Set VX to VY minus VX
                case 0x0007: {
                    uint8_t noBorrow = (registers[y] >= registers[x]) ? 1 : 0;
                    uint16_t diff = registers[y] - registers[x];

                    registers[x] = diff;
                    registers[0xF] = noBorrow;
                    break;
                }

                // 8XYE - Shift VX left by one
                // VF is set to the most significant bit of VX before the shift
                // We shift it right by 7 bits so that VF will be either 0 or 1
                case 0x000E: {
                    uint8_t msb = (registers[y] & 0x80) >> 7;
                    registers[x] = registers[y] << 1;
                    registers[0xF] = msb;
                    break;
                }
            }

            break;

            
                
        }

        // 9XY0 - Skip the next instruction if register VX does not equal register VY
        case 0x9000: {
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t y = (opcode & 0x00F0) >> 4;

            if (registers[x] != registers[y]) {
                program_counter += 2;
            }

            break;
        }

        case 0xA000: { // ANNN - Set index register to address NNN
            index_register = opcode & 0x0FFF;
            break;
        }

        // BNNN - Jump to address NNN plus V0
        case 0xB000: {
            uint16_t nnn = opcode & 0x0FFF;
            program_counter = nnn + registers[0];
            pc_incremented_by_jump = true;
            break;
        }

        // CXNN - Set VX to a random number and mask it with NN
        case 0xC000: {
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t nn = opcode & 0x00FF;
            // The modulo operator (%) limits the range to 0-255; the masking with NN allows
            // for more control over specific ranges
            registers[x] = (rand() % 256) & nn;
            break;
        }
        
        case 0xD000: { // DXYN - Draw a sprite at coordinate (VX, VY) with width 8 pixels and height N pixels.
            
            //std::cout << "CPU is drawing a sprite at coordinate!" << std::endl;
            
            uint8_t x = (opcode & 0x0F00) >> 8; // VX holds the X coordinate for drawing the sprite
            uint8_t y = (opcode & 0x00F0) >> 4; // VY holds the Y coordinate for drawing the sprite
            uint8_t n = opcode & 0x000F; // N specifies the height of the sprite in pixels.

            uint8_t vx = registers[x] % 64;
            uint8_t vy = registers[y] % 32; // Variable used to store the value of the source register % 32

            registers[0xF] = 0; // Reset the collision flag before drawing the sprite;
                                // Is set to 1 if a collision is detected during the drawing process
            
            for (uint8_t row = 0; row < n; ++row) { // Each row of the sprite is represented by a byte (8 bits) in memory

                uint8_t current_y = vy + row;

                // If the current row exceeds the display height, stop drawing this sprite
                if (current_y >= 32) {
                    break;
                }

                uint8_t sprite_byte = memory[index_register + row];

                for (uint8_t col = 0; col < 8; ++col) {
                    
                    uint8_t current_x = vx + col;

                    // If the current column exceeds the display width, stop drawing this sprite
                    if (current_x >= 64) {
                        break;
                    }
                    
                    //Checks to see if the current bit in the sprite byte is "1"
                    uint8_t sprite_pixel = (sprite_byte >> (7 - col)) & 0x01;

                    if (sprite_pixel == 1) {
                        
                        uint16_t display_index = (current_y * 64) + current_x;
                        
                        // Checks to see if the pixel is already set to "1" on the display screen
                        // If it is, collision occurs and the collision flag is set to 1
                        if (display[display_index] == 1) registers[0xF] = 1;
                        //The XOR operator (^) is commonly used in graphics programming for toggling pixels;
                        // The simple rule of XOR is that if the bits are different, the result is "1"; if they are the same, the result is "0".
                        // If the pixel is "0", it will be set to "1"; if it is already "1", it will be toggled back to "0".
                        display[display_index] ^= 1;
                        
                    }
                }
            }
            break;
        }

        case 0xE000: {
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t key = registers[x] & 0x0F;

            switch (opcode & 0x00FF) {
                // EX9E - Skip the next instruction if the key corresponding to the value of VX
                // is currently pressed (1 in the keypad array)
                case 0x009E:
                    if (keypad[key] != 0) {
                        program_counter += 2;
                    }
                break;

                // EXA1 - Skip the next instruction if the key corresponding to value of VX
                // is not currently pressed (0 in the keypad array)
                case 0x00A1:
                    if (keypad[key] == 0) {
                        program_counter += 2;
                    }
                break;

            }
            break;
        }

        case 0xF000: {
            // Stores the value of the destination register (VX) in a variable "x"
            uint8_t x = (opcode & 0x0F00) >> 8;
            switch (opcode & 0x00FF) {
                // FX07 - Set VX to the value of the delay timer
                case 0x0007: 
                    registers[x] = delay_timer;
                break;

                // FX15 - Set the delay timer to the value of VX
                case 0x0015:
                    delay_timer = registers[x];
                break;

                // FX18 - Set the sound timer to the value of VX
                case 0x0018:
                    sound_timer = registers[x];
                break;

                // FX1E - Adds value of VX (registers[x]) to index register.
                case 0x001E:
                    index_register += registers[x];
                break;

                // FX29 - Set index register to the memory location of the sprite for the character in VX.
                case 0x0029:
                    // Each character in the fontset is 5 bytes long
                    // Multiplying by 5 gives us the starting memory address of the sprite
                    index_register = registers[x] * 5;
                break;

                // FX33 - Binary-Coded Decimal (BCD) Conversion;
                // Breaks down the value of VX into individual digits and stores them in memory
                // starting at the address in the index register
                case 0x0033: {
                    uint8_t value = registers[x];
                    // Hundreds digit
                    memory[index_register] = value /100;
                    //Tens digit
                    memory[index_register + 1] = (value / 10) % 10;
                    //Ones digit
                    memory [index_register + 2] = value % 10;
                    break;
                }

                // FX55 - Store registers V0 through VX in memory starting at the address in the index register.
                case 0x0055:
                    for (int i = 0; i <= x; ++i) {
                        memory[index_register + i] = registers[i];
                    }

                    break;

                // FX65 - Fill registers V0 through VX with values from memory starting at the address in the index register.
                case 0x0065: {
                    for (int i = 0; i <= x; ++i) {
                        registers[i] = memory[index_register + i];
                    }

                    break;
                }

                // FX0A - Wait for a key press and store the value of the key in VX
                case 0x000A: {
                    bool key_pressed = false;
                    // Emulator pauses execution until a key is pressed
                    // Iterates through the keypad array to check for any key press
                    for (int i = 0; i < 16; ++i) {
                        if (keypad[i] != 0) {
                            registers[x] = i;
                            key_pressed = true;
                            break;
                        }
                    }

                    // If no key is pressed, instruction is repeated
                    if (!key_pressed) {
                        program_counter -= 2;
                    }

                    break;
                }
            }

            break;
        }

        default:
            std::cerr << "Unknown opcode: " << std::hex <<opcode << std::endl;
        break;    
    }

    if (!pc_incremented_by_jump) {
        program_counter += 2;
    }
}

// Progress: 28 out of 35 opcodes implemented;

void Chip8::UpdateTimers() {
    if (delay_timer > 0) {
        delay_timer--;
    }

    if (sound_timer > 0) {
        sound_timer--;
    }
}