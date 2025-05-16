#include "display.h"

LiquidCrystal_I2C screen(LCD_ADDRESS, LCD_LENGTH, LCD_HEIGHT);

// Cache for LCD lines to minimize writes and flicker
char prevLines[LCD_HEIGHT][LCD_LENGTH + 1];

// Total number of items in the main settings menu
const int SETTINGS_TOTAL_ITEMS = 7;
// How many lines are visible in the settings menu
const int VISIBLE_SETTINGS_LINES = 4;

void setupDisplay()
{
  screen.begin(LCD_ADDRESS, LCD_LENGTH, LCD_HEIGHT); // Use defines from config.h
  screen.backlight();
  clearLineCache(); // Initialize the cache
  screen.clear();
  screen.setCursor(0, 0);
  screen.print("Initializing...");
  Serial.println("LCD Initialized.");
}

void clearLineCache()
{
  for (int i = 0; i < LCD_HEIGHT; i++)
  {
    prevLines[i][0] = '\0'; // Set first char to null terminator
  }
}

void printLine(int line, const String &text)
{
  if (line < 0 || line >= LCD_HEIGHT)
    return; // Basic bounds check

  String padded = text;
  // Pad with spaces to ensure the previous content is overwritten
  while (padded.length() < LCD_LENGTH)
  {
    padded += " ";
  }
  // Trim if too long (should ideally not happen with proper formatting)
  if (padded.length() > LCD_LENGTH)
  {
    padded = padded.substring(0, LCD_LENGTH);
  }

  // Only update LCD if the content has changed
  if (padded != String(prevLines[line]))
  {
    screen.setCursor(0, line);
    screen.print(padded);
    strcpy(prevLines[line], padded.c_str()); // Update cache
  }
}

void lcdScreen1()
{
  // Line 0: Temperature and Humidity (Unchanged)
  String line0 = "T:" + String(t, 1) + "C H:" + String(h, 1) + "%";

  // Line 1: Min/Max Temp Thresholds (Unchanged)
  String line1 = "Min:" + String(mi) + " Max:" + String(ma);

  // Line 2: Connection Status / Mode
  String line2;
  switch (connectionStatus)
  {
  case CONNECTING:
    line2 = "Connecting...";
    break;
  case CONNECTED:
    line2 = "Online ";
    break;
  case OFFLINE:
    line2 = "Offline ";
    break;
  default:
    line2 = "Status? ";
    break;
  }
  // Append Mode
  String modeStr;
  int m = abs(mod) % 5;
  if (m == 0)
    modeStr = "AUTO";
  else if (m == 1)
    modeStr = "OFF";
  else if (m == 2)
    modeStr = "ON";
  else if (m == 3)
    modeStr = "HALF";
  else
    modeStr = "TIMER";
  line2 += " M:" + modeStr;

  // Line 3: IP Address
  String line3 = "IP: " + currentIPAddress; // Uses the global volatile variable

  // Print the lines
  printLine(0, line0); // Line 0
  printLine(1, line1); // Line 1
  printLine(2, line2); // Line 2 (Connection/Mode)
  printLine(3, line3); // Line 3 (IP Address)
}

void lcdSettingsScreen()
{
  String lineText;

  if (settingsState == 0)
  { // Navigation mode
    // Adjust the view offset based on selection
    if (selectedField < settingsMenuOffset)
    {
      settingsMenuOffset = selectedField;
    }
    else if (selectedField >= settingsMenuOffset + VISIBLE_SETTINGS_LINES)
    {
      settingsMenuOffset = selectedField - VISIBLE_SETTINGS_LINES + 1;
    }

    // Render the visible menu items
    for (int i = 0; i < VISIBLE_SETTINGS_LINES; i++)
    {
      int itemIndex = settingsMenuOffset + i;
      if (itemIndex >= SETTINGS_TOTAL_ITEMS)
      {
        printLine(i, ""); // Clear lines beyond the last item
        continue;
      }

      lineText = (itemIndex == selectedField) ? ">" : " "; // Indicate selection

      // Populate text based on item index
      switch (itemIndex)
      {
      case 0:
        lineText += "Back";
        break;
      case 1:
        lineText += "Min Temp: " + String(mi);
        break;
      case 2:
        lineText += "Max Temp: " + String(ma);
        break;
      case 3:
      {
        lineText += "Mode: ";
        int m = abs(mod) % 5;
        if (m == 0)
          lineText += "AUTO";
        else if (m == 1)
          lineText += "OFF";
        else if (m == 2)
          lineText += "ON";
        else if (m == 3)
          lineText += "HALF";
        else
          lineText += "TIMER";
        break;
      }
      case 4:
        lineText += "Delay(s): " + String(switchingDelay);
        break;
      case 5:
        lineText += "Hum Limit: " + String(humLimit) + "%";
        break;
      case 6:
        lineText += "Reconnect WiFi";
        break;
        // Add more settings here if needed, increasing SETTINGS_TOTAL_ITEMS
      }
      printLine(i, lineText);
    }
  }
  else if (settingsState == 1)
  { // Editing mode
    // Clear screen except for the edit prompt and value
    for (int i = 0; i < LCD_HEIGHT; ++i)
      printLine(i, ""); // Clear all lines first

    String prompt = "Edit ";
    String valueStr = "";
    bool showSaveHint = true;

    switch (selectedField)
    {
    case 1:
      prompt += "Min Temp:";
      valueStr = String(mi);
      break;
    case 2:
      prompt += "Max Temp:";
      valueStr = String(ma);
      break;
    case 3:
    {
      prompt += "Mode:";
      int m = abs(mod) % 5;
      if (m == 0)
        valueStr = "AUTO";
      else if (m == 1)
        valueStr = "OFF";
      else if (m == 2)
        valueStr = "ON";
      else if (m == 3)
        valueStr = "HALF";
      else
        valueStr = "TIMER";
      break;
    }
    case 4:
      prompt += "Delay(s):";
      valueStr = String(switchingDelay);
      break;
    case 5:
      prompt += "Hum Limit(%):";
      valueStr = String(humLimit);
      break;
    default:
      // Should not happen if navigation is correct
      prompt = "Error";
      valueStr = "Invalid Field";
      showSaveHint = false;
      settingsState = 0; // Go back to navigation
      break;
    }

    printLine(0, prompt);
    printLine(1, valueStr);
    if (showSaveHint)
    {
      printLine(3, "Turn to change");
      // Or display range hints if applicable
      // printLine(2, "Range: 1-50");
      // printLine(3, "Press to Save");
    }
  }
}

void updateDisplay()
{
  // Check for screen timeout first
  if ((screenMode == 2) && (millis() - lastInteraction > SCREEN2_TIMEOUT))
  {
    if (settingsState == 1)
    { // If editing, save before timeout
      // saveSettingsEEPROM(); // Save is now handled on button press exit
    }
    screenMode = 1;    // Return to main screen
    settingsState = 0; // Reset settings state
    clearLineCache();  // Force redraw of main screen
    Serial.println("Settings screen timed out.");
  }

  // Render the current screen
  if (screenMode == 1)
  {
    lcdScreen1();
  }
  else if (screenMode == 2)
  {
    lcdSettingsScreen();
  }
}