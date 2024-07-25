#include "morse.h"

// Morse code representation for each letter, digit, space, and punctuation
const char *alphabet[] = {
    ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", // a-j
    "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", // k-t
    "..-", "...-", ".--", "-..-", "-.--", "--..", // u-z
    " ", // space, this will be handled separately
    ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----.", "-----", // 0-9
    ".", "-" // . -
};

const char* getMorseCode(char c) {
    if (c >= 'a' && c <= 'z') {
        return alphabet[c - 'a'];
    } else if (c >= 'A' && c <= 'Z') {
        return alphabet[c - 'A'];
    } else if (c >= '0' && c <= '9') {
        return alphabet[c - '0' + 26];
    } else if (c == '.') {
        return alphabet[37];
    } else if (c == '-') {
        return alphabet[38];
    } else {
        return ""; // Ignore unsupported characters
    }
}

// Helper function to clear a buffer
void clearBuffer(char *buffer, int size) {
    for (int i = 0; i < size; i++) {
        buffer[i] = '\0';
    }
}

// Helper function to concatenate strings
void concatString(char *dest, const char *src) {
    while (*dest) {
        dest++;
    }
    while (*src) {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
}

// Function to encode a string into Morse code and return the output string
char* string2morse(const char *str) {
    static char output[1024]; // Static to hold the value across function calls
    clearBuffer(output, sizeof(output)); // Clear the output buffer

    while (*str) {
        if (*str == ' ') {
            // Add three spaces for each space in the original sentence
            concatString(output, "   ");
        } else {
            const char *morse = getMorseCode(*str);
            concatString(output, morse);
            concatString(output, " "); // Add space between Morse code characters
        }
        str++;
    }

    return output;
}
