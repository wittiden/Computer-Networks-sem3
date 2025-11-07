#pragma once
#include <iostream>
#include <string>
#include <conio.h>

constexpr const int ENTER = 13;
constexpr const int BACKSPACE = 8;

bool check_specSymbols_input(char c);
void number_filteredInput(std::string& buf);