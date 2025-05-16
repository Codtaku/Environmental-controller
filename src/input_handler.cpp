#include "input_handler.h"
#include "eeprom_manager.h" // Needed for saving settings when exiting settings screen
#include "display.h"        // Needed for clearing line cache on screen change
#include "wifi_manager.h"   // Needed to call connectWiFiBlynk on reconnect option

// Rotary Encoder State
volatile int lastA = HIGH;
volatile long encoderPos = 0;
volatile bool encoderUpdated = false;                // Flag set by ISR, cleared by handler
volatile unsigned long lastEncoderInterruptTime = 0; // For debouncing ISR
TaskHandle_t encoderTaskHandle = NULL;

// Button State
unsigned long buttonHoldStart = 0;
bool longPressTriggered = false;
bool lastButtonState = HIGH;
void encoderTask(void *parameter)
{
    Serial.println("Encoder Task started on Core 1.");
    pinMode(ENCODER_PIN_A, INPUT_PULLUP); // Initialize pins within the task if not done in setupInput
    pinMode(ENCODER_PIN_B, INPUT_PULLUP);
    lastA = digitalRead(ENCODER_PIN_A); // Read initial state

    long lastProcessedPos = 0; // Local tracking within the task

    for (;;)
    { // Task loop runs forever
        // --- Direct Pin Reading (Alternative to ISR) ---
        int currentA = digitalRead(ENCODER_PIN_A);
        int currentB = digitalRead(ENCODER_PIN_B);

        if (lastA == LOW && currentA == HIGH)
        { // Rising edge on A
            // Need atomic access or disable interrupts if encoderPos write isn't atomic on ESP32?
            // For simple increments/decrements, should be okay, but be mindful.
            noInterrupts(); // Briefly disable interrupts for safe update
            if (currentB == LOW)
            {
                encoderPos++; // Clockwise
            }
            else
            {
                encoderPos--; // Counter-clockwise
            }
            interrupts(); // Re-enable interrupts
        }
        lastA = currentA;
        // --- End Direct Pin Reading ---

        // --- Process Position Change ---
        // Read volatile variable safely
        noInterrupts();
        long currentPos = encoderPos;
        interrupts();

        if (currentPos != lastProcessedPos)
        {
            long delta = currentPos - lastProcessedPos;
            lastProcessedPos = currentPos; // Update tracking

            // Get current screen mode and settings state (read volatile vars)
            noInterrupts();
            int currentScreenMode = screenMode;
            int currentSettingsState = settingsState;
            int currentSelectedField = selectedField; // Need this too
            interrupts();

            // Update interaction time (accessed by main loop too)
            // Use atomic operation or mutex if available, otherwise volatile might suffice here
            lastInteraction = millis();

            if (currentScreenMode == 2 && currentSettingsState == 1)
            { // Editing mode
                // Safely modify shared variables (mi, ma, mod, etc.)
                // Use volatile variables directly if simple writes are okay,
                // or use mutexes for more complex updates if needed.
                noInterrupts(); // Protect shared variable access
                switch (currentSelectedField)
                {
                case 1:
                    mi += delta;
                    if (mi > ma)
                        mi = ma;
                    break;
                case 2:
                    ma += delta;
                    if (ma < mi)
                        ma = mi;
                    break;
                case 3:
                    mod += delta;
                    mod = (mod % 5 + 5) % 5;
                    break;
                case 4:
                    switchingDelay += delta;
                    if (switchingDelay < 1)
                        switchingDelay = 1;
                    if (switchingDelay > 300)
                        switchingDelay = 300;
                    break;
                case 5:
                    humLimit += delta;
                    if (humLimit < 0)
                        humLimit = 0;
                    if (humLimit > 100)
                        humLimit = 100;
                    break;
                }
                interrupts(); // Release protection
            }
            else if (currentScreenMode == 2 && currentSettingsState == 0)
            { // Navigation mode
                // Modify selectedField (also shared, needs protection)
                noInterrupts();
                selectedField += delta;
                if (selectedField < 0)
                    selectedField = 0;
                if (selectedField >= SETTINGS_TOTAL_ITEMS)
                    selectedField = SETTINGS_TOTAL_ITEMS - 1;
                interrupts();
                // Beep feedback (safe to call)
                beepEncoder();
            }
            // Serial.print("Encoder Task: New Pos="); Serial.println(currentPos); // Debug
        } // End if position changed

        // Let other tasks on Core 1 run (important!)
        vTaskDelay(2 / portTICK_PERIOD_MS); // Check encoder pins every ~2ms
    } // End task loop
}
void setupInput()
{
    //   pinMode(ENCODER_PIN_A, INPUT_PULLUP);
    //   pinMode(ENCODER_PIN_B, INPUT_PULLUP);
    pinMode(ENCODER_BUTTON, INPUT_PULLUP);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW); // Ensure buzzer is off initially

    // Read initial state for ISR
    lastA = digitalRead(ENCODER_PIN_A);

    // Attach Interrupts (ISR runs on change) - Ensure ISR is fast!
    //   attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), readEncoderISR, CHANGE);
    //   attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), readEncoderISR, CHANGE);

    encoderPos = 0;             // Reset position on setup
    lastInteraction = millis(); // Initialize interaction time
    lastButtonState = digitalRead(ENCODER_BUTTON);
    lastInteraction = millis();
    //   Serial.println("Input (Encoder/Button) Initialized with ISR.");
}

// Interrupt Service Routine - KEEP THIS EXTREMELY SHORT AND FAST!
// Avoid Serial prints, complex logic, or delays inside ISRs.
void IRAM_ATTR readEncoderISR()
{
    // Basic Debounce: Ignore interrupts occurring too close together
    unsigned long interruptTime = millis();
    if (interruptTime - lastEncoderInterruptTime < 5)
    { // Adjust debounce time (ms) if needed
        return;
    }
    lastEncoderInterruptTime = interruptTime;

    int a = digitalRead(ENCODER_PIN_A);
    int b = digitalRead(ENCODER_PIN_B);

    // Detect RISING edge on Pin A for simpler logic (or FALLING depending on encoder)
    if (lastA == LOW && a == HIGH)
    { // Check for a RISING edge on A
        if (b == LOW)
        { // If B is LOW during A's rise, it's clockwise
            encoderPos++;
        }
        else
        { // If B is HIGH during A's rise, it's counter-clockwise
            encoderPos--;
        }
        encoderUpdated = true; // Set flag for main loop to process
        // DO NOT update lastInteraction here - ISR should be minimal. Update in handleEncoderRotation.
    }
    lastA = a; // Update last state for next interrupt

    // Alternative: Full quadrature decoding (handles all transitions) - More complex
    // int sig1 = digitalRead(ENCODER_PIN_A);
    // int sig2 = digitalRead(ENCODER_PIN_B);
    // int8_t movement = 0;
    // // ... (lookup table or logic for quadrature) ...
    // if (movement != 0) {
    //    encoderPos += movement;
    //    encoderUpdated = true;
    // }
}

// Processes the encoder position change flagged by the ISR
// Called from the main loop when encoderUpdated is true
void handleEncoderRotation()
{
    if (encoderUpdated)
    {
        // Get the latest position atomically (disable interrupts briefly if needed, though unlikely necessary for long)
        // noInterrupts(); // Consider if race conditions are possible with other ISRs modifying encoderPos
        long currentPos = encoderPos;
        // interrupts();

        static long lastProcessedPos = 0; // Keep track of the last position handled

        long delta = currentPos - lastProcessedPos;

        if (delta != 0)
        {                               // If there was a change since last processing
            lastInteraction = millis(); // Update interaction time here

            if (screenMode == 2 && settingsState == 1)
            { // Editing mode in settings
                // beepEncoder(); // Beep on each tick while editing
                switch (selectedField)
                {
                case 1: // Edit Min Temp
                    mi += delta;
                    if (mi > ma)
                        mi = ma; // Clamp min <= max
                    // Add lower bound? if (mi < 0) mi = 0;
                    break;
                case 2: // Edit Max Temp
                    ma += delta;
                    if (ma < mi)
                        ma = mi; // Clamp max >= min
                                 // Add upper bound? if (ma > 50) ma = 50;
                    break;
                case 3: // Edit Mode
                    mod += delta;
                    // Keep mode within 0-4 range using modulo
                    mod = (mod % 5 + 5) % 5; // Handles negative results correctly
                    break;
                case 4: // Edit Switching Delay
                    switchingDelay += delta;
                    if (switchingDelay < 1)
                        switchingDelay = 1; // Min delay 1 sec
                    if (switchingDelay > 300)
                        switchingDelay = 300; // Max delay 300 secs (5 min)
                    break;
                case 5: // Edit Humidity Limit
                    humLimit += delta;
                    if (humLimit < 0)
                        humLimit = 0; // Min limit 0%
                    if (humLimit > 100)
                        humLimit = 100; // Max limit 100%
                    break;
                    // Add cases for other editable settings if any
                }
            }
            else if (screenMode == 2 && settingsState == 0)
            {                  // Navigation mode in settings
                beepEncoder(); // Beep on each tick while navigating menu
                selectedField += delta;
                // Clamp selection within bounds
                if (selectedField < 0)
                    selectedField = 0;
                if (selectedField >= SETTINGS_TOTAL_ITEMS)
                    selectedField = SETTINGS_TOTAL_ITEMS - 1;
            }
            // Add handling for encoder use in screenMode 1 if needed
        }

        lastProcessedPos = currentPos; // Update the last processed position
        encoderUpdated = false;        // Clear the flag
    }
}

// Checks the state of the encoder button for short press and long press
// Called continuously from the main loop
void checkEncoderButton()
{
    bool currentButtonState = digitalRead(ENCODER_BUTTON);

    // Detect button press (transition from HIGH to LOW)
    if (lastButtonState == HIGH && currentButtonState == LOW)
    {
        buttonHoldStart = millis(); // Record press time
        longPressTriggered = false; // Reset long press flag
        // Don't beep immediately, wait for release for short press action
        Serial.println("Button Pressed.");
    }

    // Check for long press while button is held LOW
    if (currentButtonState == LOW && !longPressTriggered)
    {
        if (millis() - buttonHoldStart >= 5000)
        { // 5 seconds hold time
            longPressTriggered = true;
            Serial.println("Long Press Detected - Restarting...");
            beepEncoder(); // Beep for feedback
            delay(100);    // Short delay for beep
            beepEncoder();

            // Display restarting message (optional, needs display access)
            screen.clear();
            clearLineCache();
            screen.setCursor(0, 1); // Center roughly
            screen.print("Restarting...");
            delay(1500); // Show message briefly

            ESP.restart(); // Restart the ESP32
        }
    }

    // Detect button release (transition from LOW to HIGH)
    if (lastButtonState == LOW && currentButtonState == HIGH)
    {
        Serial.println("Button Released.");
        if (!longPressTriggered)
        {
            beepEncoder();              // Beep for short press feedback
            noInterrupts();             // Protect shared variable writes
            lastInteraction = millis(); // Update interaction time

            if (screenMode == 1)
            {
                screenMode = 2;
                settingsState = 0;
                selectedField = 0;
                settingsMenuOffset = 0; // Assuming this is only used by display code in main loop
            }
            else if (screenMode == 2)
            {
                if (settingsState == 0)
                {
                    if (selectedField == 0)
                    { // Back selected
                        screenMode = 1;
                        // Save outside interrupt disabled section if it takes time
                    }
                    else if (selectedField == 6)
                    {                   // Reconnect
                        screenMode = 1; // Switch screen first
                                        // Trigger reconnect flag/mechanism to be handled in main loop?
                    }
                    else
                    { // Enter edit mode
                        settingsState = 1;
                    }
                }
                else if (settingsState == 1)
                { // Exiting edit mode
                    settingsState = 0;
                    // Save outside interrupt disabled section if it takes time
                }
            }
            interrupts(); // Re-enable interrupts

            // Perform actions needing libraries AFTER re-enabling interrupts
            if (screenMode == 1 && settingsState == 0)
            { // Just exited settings
                saveSettingsEEPROM();
            }
            else if (screenMode == 1 && selectedField == 6)
            { // Just selected Reconnect
                Serial.println("Action: Attempting Reconnect...");
                // connectionStatus = CONNECTING; // Update status (volatile)
                // Need to handle actual reconnect call in main loop based on a flag maybe?
                setupWiFiAndServer(); // Calling this directly might block button handling briefly
            }
            else if (screenMode == 2 && settingsState == 0 && settingsState == 1)
            { // Just exited edit mode
                saveSettingsEEPROM();
            }

            clearLineCache(); // Force redraw
        }
        // Reset hold timer variables after release (regardless of short/long press)
        buttonHoldStart = 0;
        // longPressTriggered is reset on next press
    }

    // Update last state for next iteration
    lastButtonState = currentButtonState;
}

void beepEncoder()
{
    digitalWrite(BUZZER_PIN, HIGH);
    delay(10); // Short beep duration
    digitalWrite(BUZZER_PIN, LOW);
}

/*
// --- Alternative: Task-based encoder reading (runs on Core 1) ---
// If using this, remove ISR attachment in setupInput() and call handleEncoderRotation() from here.
void encoderTask(void * parameter) {
  Serial.println("Encoder Task started on Core 1.");
  long lastReportedPos = 0;
  for(;;) {
    int a = digitalRead(ENCODER_PIN_A);
    int b = digitalRead(ENCODER_PIN_B);

     // Basic rotation detection (same logic as ISR example)
     if (lastA == LOW && a == HIGH) { // Rising edge on A
        if (b == LOW) {
            encoderPos++;
        } else {
            encoderPos--;
        }
        encoderUpdated = true; // Set flag
     }
     lastA = a;

     // If using full quadrature, implement logic here

     // If position changed, process it (call handler directly or just update flag)
     if (encoderUpdated) {
        // Call the same handler logic as used with ISR
        handleEncoderRotation(); // This will clear encoderUpdated flag
     }


    // Small delay to prevent hogging the core
    vTaskDelay(5 / portTICK_PERIOD_MS); // Check every 5ms
  }
}
*/