// Header guard to prevent the compiler from trying to define the CHIP8 class multiple times
// Only really needed if the header file is included in multiple source files
#ifndef CHIP8_H
#define CHIP8_H

#include <cstdint>

class Chip8 {
    public:

        // The constructor that initializes the emulator's state; defaults values for memory, registers, the program counter, etc.
        Chip8();
        // Destructor; defaulted since we are not using dynamic memory allocation
        ~Chip8() = default;

        bool LoadROM(const char* filename);
        void EmulateCycle();
        void UpdateTimers();

        // Getter functions to access private members from outside the class
        const uint8_t* GetDisplay() const {return display; }
        const uint8_t* GetMemory() const {return memory; }

        // Setter and getter functions to access the keypad state from outside the class
        void SetKeyState(uint8_t key, bool isPressed) {
            if (key < 16) {
                keypad[key] = isPressed ? 1 : 0;
            }
        }

        uint8_t GetKeyState(uint8_t key) const {
            return (key < 16) ? keypad[key] : 0;
        }

        bool quirk_memory_increment = false;

    
    // Only the Chip8 class itself needs access to the fontset.
    private:
        
        /*
        uint8_t is a fixed-width integer type
        u: unsigned - it can only represent non-negative values (0 and positive integers)
        int: integer
        8: number of bits; 8 bits = 1 byte; can represent values from 0 to 255
        _t: type - indicates that this is a type definition
        Total storage size of memory is 4096 bytes (4KB), which is sufficient for most Chip-8 programs.
        */
        uint8_t memory[4096];
        // Registers are used to store temporary data during the execution of a program.
        uint8_t registers[16];
        // Used to point to locations in memory where data is stored or where instructions are located.
        uint16_t index_register;
        // a 16-bit number that stores the memory address of the instruction the CPU is currently executing
        uint16_t program_counter;
        /* Stack is used to store the memory addresses of where the program should return after a subroutine call.
        When a subroutine is called, the current program counter is pushed onto the stack and then popped off the stack once the instruction is complete.
        */
        uint16_t stack[16];
        // Keeps track of where we are in the stack.
        uint8_t stack_pointer;
        // Used for game timing. Rate of 60Hz, 60 times per second
        // Without these timers, the games will run as fast as the user's CPU;
        // CHIP-8 interpreters were designed to run on hardware that was much slower than modern PCs.
        uint8_t delay_timer;
        // Used for sound effects
        uint8_t sound_timer;
        // Creates a 64 pixels wide by 32 pixels high display in the console.
        uint8_t display[64 * 32];
        // Listens for key presses. Creates a bridge between the emulator and the user input. The Chip-8 has a 16-key hexadecimal keypad (0-9 and A-F).
        uint8_t keypad[16];
    
        //Every copy of the Chip-8 emulator shares the same fontset, making the fontset a static member of the class saves memory.
        // static - "read-only" data; no need for duplication.
        // const  - cannot be modified after initialization.
        static const uint8_t fontset[80];

};

#endif