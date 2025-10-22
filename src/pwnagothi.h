#include "Arduino.h"
#include <vector>
#pragma once

void addToWhitelist(const String &valueToAdd);
std::vector<String> parseWhitelist();
void pwnagothiLoop();
bool pwnagothiBegin();
void removeItemFromWhitelist(String valueToRemove);