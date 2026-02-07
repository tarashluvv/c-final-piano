// ==================================================================================
// FILE: piano_terminal.cpp
// PROJECT: Console-Based Virtual Piano
// AUTHOR: [Your Name]
// SYSTEM: Windows Only (uses windows.h for sound and conio.h for input)
// ==================================================================================
// Compile: g++ piano_terminal.cpp -o piano_terminal
// Run: piano_terminal.exe

#include <iostream>      // Include standard input/output stream library (for std::cout)
#include <vector>        // Include vector container library (dynamic arrays)
#include <string>        // Include string library (text manipulation)
#include <map>           // Include map container library (key-value pairs)
#include <chrono>        // Include time library (for high-precision timestamps)
#include <thread>        // Include thread library (for sleep functions)
#include <cmath>         // Include math library (required for std::pow function)
#include <windows.h>     // Include Windows API header (required for Beep() function)
#include <conio.h>       // Include Console I/O header (required for _getch() function)

// ==================================================================================
// CONFIGURATION & CONSTANTS
// ==================================================================================
const int BASE_DURATION = 200; // Define a constant integer for note duration (200 milliseconds)

// Music Note Structure definition
struct Note {
    std::string name;       // String to store the display name of the note (e.g., "C")
    double frequency;       // Double precision float for the note's frequency in Hertz
    long long timestamp;    // Long long integer for the time offset (ms) from start of recording
};

// Recording Structure definition
struct Recording {
    std::string name;       // String to store the name of the recording
    std::vector<Note> notes; // Vector (dynamic list) to store the sequence of Note objects
};

// ==================================================================================
// PIANO CLASS
// ==================================================================================
class ConsolePiano {        // Define the main class for the Piano logic
private:                    // Private access modifier (variables only accessible inside this class)
    std::map<char, std::pair<std::string, double>> keyMap; // Map linking a keyboard 'char' to a pair: Note Name (string) and Frequency (double)
    std::vector<Note> currentRecording; // Vector to store notes currently being recorded
    bool isRecording;       // Boolean flag to track if we are currently recording (true/false)
    long long recordingStartTime; // Variable to store the exact system time when recording started
    int octave;             // Integer to store the current octave shift (default is 4)

public:                     // Public access modifier (functions accessible from main)
    ConsolePiano() : isRecording(false), octave(4), recordingStartTime(0) { // Constructor: Initializes variables (recording off, octave 4)
        // Initialize Key Mappings (Keyboard key -> {Note Name, Frequency})
        // Mapping 'z' key to C note (261.63 Hz)
        keyMap['z'] = {"C",  261.63};
        // Mapping 's' key to C# note
        keyMap['s'] = {"C#", 277.18};
        // Mapping 'x' key to D note
        keyMap['x'] = {"D",  293.66};
        // Mapping 'd' key to D# note
        keyMap['d'] = {"D#", 311.13};
        // Mapping 'c' key to E note
        keyMap['c'] = {"E",  329.63};
        // Mapping 'v' key to F note
        keyMap['v'] = {"F",  349.23};
        // Mapping 'g' key to F# note
        keyMap['g'] = {"F#", 369.99};
        // Mapping 'b' key to G note
        keyMap['b'] = {"G",  392.00};
        // Mapping 'h' key to G# note
        keyMap['h'] = {"G#", 415.30};
        // Mapping 'n' key to A note (Standard Tuning Pitch)
        keyMap['n'] = {"A",  440.00};
        // Mapping 'j' key to A# note
        keyMap['j'] = {"A#", 466.16};
        // Mapping 'm' key to B note
        keyMap['m'] = {"B",  493.88};
    }

    // Function to draw the User Interface in the console
    void drawInterface() {
        system("cls"); // Execute system command to clear the console screen
        std::cout << "==================================================\n"; // Print separator line
        std::cout << "   C++ CONSOLE PIANO (ENGINEERING PROJECT)   \n";      // Print Title
        std::cout << "==================================================\n"; // Print separator line
        std::cout << " Controls:                                        \n"; // Print header for controls
        std::cout << "  [Keys z-m]: Play Notes                          \n"; // Instruction for playing
        std::cout << "  [R]: Start/Stop Recording                       \n"; // Instruction for recording
        std::cout << "  [P]: Play Last Recording                        \n"; // Instruction for playback
        std::cout << "  [+/-]: Change Octave (Current: " << octave << ")\n"; // Print current octave dynamically
        std::cout << "  [Q]: Quit                                       \n"; // Instruction for quitting
        std::cout << "==================================================\n"; // Print separator line
        
        // Draw Visual Keyboard representation
        std::cout << "\n";                                                // Print newline
        std::cout << "   | |S| |D| | |G| |H| |J| | |\n";                   // Print black keys visual
        std::cout << "   | | | | | | | | | | | | | |\n";                   // Print key spacing
        std::cout << "   |_| |_| |_| |_| |_| |_| |_|\n";                   // Print key bottoms
        std::cout << "    Z   X   C   V   B   N   M \n";                   // Print corresponding keyboard letters
        std::cout << "\n";                                                // Print newline
        
        // Check if recording is active to show status
        if (isRecording) {
            std::cout << "  [🔴 RECORDING IN PROGRESS...] \n";             // Print red recording indicator
        } else if (!currentRecording.empty()) {                           // Else if we have saved notes
            std::cout << "  [💾 Recording Saved: " << currentRecording.size() << " notes] \n"; // Show count of saved notes
        }
    }

    // Function to calculate Frequency based on Octave shift
    // Input: baseFreq (standard frequency at octave 4)
    double getFrequency(double baseFreq) {
        // Return calculated frequency: base * 2^(octave - 4)
        // Uses std::pow from <cmath> library
        return baseFreq * std::pow(2.0, octave - 4);
    }

    // Function to play a single note based on key input
    void playTone(char key) {
        // Check if the pressed key exists in our map
        if (keyMap.find(key) != keyMap.end()) {
            std::string noteName = keyMap[key].first; // Get the note name (e.g., "C")
            double baseFreq = keyMap[key].second;     // Get the base frequency
            double finalFreq = getFrequency(baseFreq); // Calculate actual frequency for current octave

            // Visual feedback: Print playing note info
            // \r returns cursor to start of line to overwrite text (animation effect)
            std::cout << " -> Playing: " << noteName << octave << " (" << finalFreq << "Hz)\r";

            // Check if we are currently recording
            if (isRecording) {
                // Get current system time
                auto now = std::chrono::system_clock::now().time_since_epoch();
                // Convert time to milliseconds
                long long timeNow = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
                
                Note n;                         // Create a new Note object
                n.name = noteName;              // Set note name
                n.frequency = finalFreq;        // Set note frequency
                n.timestamp = timeNow - recordingStartTime; // Calculate relative time since recording started
                currentRecording.push_back(n);  // Add note to the recording vector
            }

            // Generate Sound using Windows API
            // Beep blocks execution for the duration (BASE_DURATION)
            Beep(static_cast<DWORD>(finalFreq), BASE_DURATION);
        }
    }

    // Function to toggle recording state on/off
    void toggleRecording() {
        if (!isRecording) { // If not currently recording
            isRecording = true; // Set flag to true
            currentRecording.clear(); // Clear previous recording data
            // Get start time for the new recording session
            auto now = std::chrono::system_clock::now().time_since_epoch();
            recordingStartTime = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
            drawInterface(); // Redraw UI to show "Recording" status
        } else { // If already recording
            isRecording = false; // Set flag to false
            drawInterface(); // Redraw UI to show "Saved" status
        }
    }

    // Function to play back the saved recording
    void playRecording() {
        if (currentRecording.empty()) { // Check if vector is empty
            std::cout << "\nNo recording found!\n"; // Print error
            Sleep(1000); // Pause for 1 second
            drawInterface(); // Redraw interface
            return; // Exit function
        }

        std::cout << "\n\n▶ Playing Recording...\n"; // Print status
        long long lastTime = 0; // Variable to track time of the previous note

        // Loop through every note in the recording vector
        for (const auto& note : currentRecording) {
            // Calculate delay: timestamp of current note minus timestamp of previous note
            long long delay = note.timestamp - lastTime;
            
            // If there is a delay needed, sleep the thread
            if (delay > 0) Sleep(static_cast<DWORD>(delay));

            std::cout << "🎵 " << note.name << " "; // Print note name
            // Play the beep sound
            Beep(static_cast<DWORD>(note.frequency), BASE_DURATION);
            
            // Update lastTime to current note's timestamp
            lastTime = note.timestamp; 
        }
        
        std::cout << "\nDone!\n"; // Print finished message
        Sleep(1000); // Pause for 1 second
        drawInterface(); // Return to main screen
    }

    // Function to change the octave
    void changeOctave(int delta) {
        octave += delta; // Add delta (+1 or -1) to current octave
        if (octave < 1) octave = 1; // Clamp minimum octave to 1
        if (octave > 8) octave = 8; // Clamp maximum octave to 8
        drawInterface(); // Redraw UI to update octave number
    }

    // Main loop function to run the application
    void run() {
        drawInterface(); // Draw initial interface
        char key; // Variable to store key press
        while (true) { // Infinite loop
            // _getch() captures a character directly from console without waiting for Enter
            key = _getch(); 
            
            // Command Handling Logic
            if (key == 'q' || key == 'Q') break; // If 'q' pressed, break loop (quit)
            else if (key == 'r' || key == 'R') toggleRecording(); // If 'r' pressed, toggle recording
            else if (key == 'p' || key == 'P') playRecording(); // If 'p' pressed, play recording
            else if (key == '+') changeOctave(1); // If '+' pressed, increase octave
            else if (key == '-') changeOctave(-1); // If '-' pressed, decrease octave
            else {
                // If not a command key, try to play it as a musical note
                playTone(tolower(key)); // Convert to lowercase and pass to playTone
            }
        }
    }
};

// ==================================================================================
// MAIN EXECUTION ENTRY POINT
// ==================================================================================
int main() {
    // System command to set the window title of the console
    system("title C++ Virtual Piano Project");
    
    ConsolePiano piano; // Instantiate the ConsolePiano object
    piano.run();        // Call the run method to start the program loop
    
    return 0; // Return 0 to indicate successful execution
}