#include "input_check.h"

bool check_specSymbols_input(char c) {
    return (c == '+') || (c == '-') || (c == '*') || (c == '/');
}
bool check_numbers_input(char c, const std::string& current_buf) {
    if (c >= '0' && c <= '9') return true;
    if (c == '.' && !current_buf.empty() && current_buf.find('.') == std::string::npos) return true;
    if (c == '-' && current_buf.empty()) return true;
    return false;
}

void number_filteredInput(std::string& buf, bool check_numbers) {
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

        if (check_numbers) {
            isValid = check_numbers_input((char)c, buf);
        }
        else {
            isValid = check_specSymbols_input((char)c);
            if (isValid && !buf.empty()) {
                isValid = false;
            }
        }

        if (isValid) {
            buf += (char)c;
            std::cout << (char)c;
        }
    }
}