#ifndef DISPLAY_H
#define DISPLAY_H

#include "globals.h"

void setupDisplay();
void printLine(int line, const String &text); // Helper to print only changed lines
void clearLineCache();                        // Reset the line cache
void lcdScreen1();                            // Renders the main info screen
void lcdSettingsScreen();                     // Renders the settings menu/edit screen
void updateDisplay();                         // Decides which screen to render

#endif // DISPLAY_H