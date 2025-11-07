#include "input_check.h"

bool check_specSymbols_input(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

void number_filteredInput(std::string& buf) {
    buf.clear();

    while (true) {
        int c = _getch();

        if (c == BACKSPACE) {
            if (!buf.empty()) {
                buf.pop_back();
                std::cout << "\b \b";
            }
            continue;
        }

        if (c == ENTER) {
            if (!buf.empty()) {
                std::cout << std::endl;
                return;
            }
            continue;
        }

        bool isValid = false;

        isValid = check_specSymbols_input((char)c);
        if (isValid && !buf.empty()) {
            isValid = false;
        }

        if (isValid) {
            buf += (char)c;
            std::cout << (char)c;
        }
    }
}