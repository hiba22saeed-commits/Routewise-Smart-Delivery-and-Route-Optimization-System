// ============================================================
//  Utils.h  -  Small helper functions used by the whole project
//
//  * Safe keyboard input: wrong input never crashes the program.
//    Everything is read with getline(), so no Enter key is left behind
//    in the input buffer (a common beginner bug when cin >> and
//    getline() are mixed).
//  * Simple text helpers: trim, lower case, split a line.
//
//  "inline" simply means: this function may live inside a header file.
// ============================================================
#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <string>
#include <cstdlib>      // exit(), atoi(), atof()
using namespace std;
// ##########################################################
// #  MEMBER 1  :  splitText, trim, colour codes, isBlankChar, toLower, sameText
// ##########################################################
const string RESET   = "\033[0m";
const string CYAN    = "\033[1;36m";
const string YELLOW  = "\033[1;33m";
const string GREEN   = "\033[1;32m";
const string RED     = "\033[1;31m";
const string MAGENTA = "\033[1;35m";
// ---------------------------------------------------------------
//  Text helpers
// ---------------------------------------------------------------

inline bool isBlankChar(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

// Removes spaces from the start and from the end of a text.
inline string trim(const string& text) {
    int first = 0;
    int last = (int)text.length() - 1;
    while (first <= last && isBlankChar(text[first])) first++;
    while (last >= first && isBlankChar(text[last])) last--;
    return text.substr(first, last - first + 1);
}

// Changes capital letters to small letters ("City Center" -> "city center").
inline string toLower(string text) {
    for (int i = 0; i < (int)text.length(); i++) {
        if (text[i] >= 'A' && text[i] <= 'Z') {
            text[i] = (char)(text[i] + ('a' - 'A'));
        }
    }
    return text;
}

// true when two texts are equal if we ignore capital / small letters.
inline bool sameText(const string& a, const string& b) {
    return toLower(a) == toLower(b);
}

// Cuts a line into parts at every 'separator' character.
// Example: "101|Ali|0300" with '|'  ->  parts[0]="101", parts[1]="Ali", parts[2]="0300"
// Returns how many parts the line really has (it can be more than maxParts).
inline int splitText(const string& line, char separator, string parts[], int maxParts) {
    int count = 0;
    string current = "";
    for (int i = 0; i < (int)line.length(); i++) {
        if (line[i] == separator) {
            if (count < maxParts) parts[count] = trim(current);
            count++;
            current = "";
        } else {
            current += line[i];
        }
    }
    if (count < maxParts) parts[count] = trim(current);
    count++;
    return count;
}

// ##########################################################
// #  MEMBER 2  :  readLine, isWholeNumber, isDecimalNumber, printLine, readInt
// ##########################################################
// Prints a line made of 'length' copies of 'symbol' (used for table borders).
inline void printLine(char symbol, int length) {
    cout << CYAN << string(length, symbol) << RESET << endl;
}

// ---------------------------------------------------------------
//  Number checks (used before converting text to a number)
// ---------------------------------------------------------------

// true for texts like 25, -3, +7   (at most 9 digits, so it always fits in an int)
inline bool isWholeNumber(const string& text) {
    int start = 0;
    if (text.length() > 0 && (text[0] == '-' || text[0] == '+')) start = 1;
    int digits = (int)text.length() - start;
    if (digits < 1 || digits > 9) return false;
    for (int i = start; i < (int)text.length(); i++) {
        if (text[i] < '0' || text[i] > '9') return false;
    }
    return true;
}

// true for texts like 2, 2.5, 0.75   (no minus sign)
inline bool isDecimalNumber(const string& text) {
    if (text.length() == 0 || text.length() > 9) return false;
    int dots = 0;
    int digits = 0;
    for (int i = 0; i < (int)text.length(); i++) {
        if (text[i] == '.') dots++;
        else if (text[i] >= '0' && text[i] <= '9') digits++;
        else return false;
    }
    return dots <= 1 && digits >= 1;
}

// ---------------------------------------------------------------
//  Keyboard input
// ---------------------------------------------------------------

// Shows the prompt and reads one full line. Spaces at both ends are removed.
// If the input ends (Ctrl+Z / Ctrl+D, or the end of a test file) the program closes politely.
inline string readLine(const string& prompt) {
    string text;
    cout << MAGENTA << prompt << RESET;
    if (!getline(cin, text)) {
      cout << YELLOW << "\n\nInput finished. Closing RouteWise.\n" << RESET;
        exit(0);
    }
    return trim(text);
}

// Reads a whole number. Asks again until the user types a valid one.
inline int readInt(const string& prompt) {
    while (true) {
        string text = readLine(prompt);
        if (isWholeNumber(text)) return atoi(text.c_str());
        cout << RED<<"  Invalid input. Please type a whole number.\n"<<RESET;
    }
}

// ##########################################################
// #  MEMBER 3  :  readPositiveDouble, readIntInRange, readPositiveInt, readNonEmpty, removeBars, pauseForEnter
// ##########################################################
// Reads a whole number between low and high (both included).
inline int readIntInRange(const string& prompt, int low, int high) {
    while (true) {
        int value = readInt(prompt);
        if (value >= low && value <= high) return value;
        cout << RED<<"  Please type a number from " << low << " to " << high << ".\n"<<RESET;
    }
}

// Reads a number that is greater than 0 (used for IDs).
inline int readPositiveInt(const string& prompt) {
    while (true) {
        int value = readInt(prompt);
        if (value > 0) return value;
        cout << RED <<"  The number must be greater than 0.\n"<<RESET;
    }
}

// Reads a decimal number greater than 0 (used for weight).
inline double readPositiveDouble(const string& prompt) {
    while (true) {
        string text = readLine(prompt);
        if (isDecimalNumber(text)) {
            double value = atof(text.c_str());
            if (value > 0) return value;
        }
        cout <<RED<< "  Invalid input. Please type a number greater than 0 (for example 2.5).\n"<<RESET;
    }
}

// Reads a text that is not empty (name, phone, address ...).
inline string readNonEmpty(const string& prompt) {
    while (true) {
        string text = readLine(prompt);
        if (text.length() > 0) return text;
        cout <<RED << "  This field cannot be empty.\n"<<RESET;
    }
}

// The data files use '|' to separate fields, so a typed '|' is replaced by '/'.
inline string removeBars(string text) {
    for (int i = 0; i < (int)text.length(); i++) {
        if (text[i] == '|') text[i] = '/';
    }
    return text;
}

// Waits until the user presses Enter (so the output can be read before the menu appears again).
inline void pauseForEnter() {
    readLine("\nPress Enter to return to the menu...");
}

#endif
