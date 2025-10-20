#pragma once
#include <iostream>
#include <string>
#include <conio.h>

constexpr const int ENTER = 13;
constexpr const int BACKSPACE = 8;

bool check_specSymbols_input(char c);
bool check_numbers_input(char c, const std::string& current_buf);
void number_filteredInput(std::string& buf, bool check_numbers = false, bool check_englishLetters = false);
bool check_englishLetters_input(char c);