// This #include statement was automatically added by the Particle IDE.
#include <IRremote.h>

// Translation of IR signals to human readable
int num0 = 16738455;
int num1 = 16724175;
int num2 = 16718055;
int num3 = 16743045;
int num4 = 16716015;
int num5 = 16726215;
int num6 = 16734885;
int num7 = 16728765;
int num8 = 16730805;
int num9 = 16732845;
int eq = 16748655;
int ch = 16736925;

// Putting these signals into an array for easier access
int validInputs[10] = {num0, num1, num2, num3, num4, num5, num6, num7, num8, num9};

// Pin declarations
int RECV_PIN = D0;
int redLED = D2;
int yellowLED = D3;
int blueLED = D4;

IRrecv irrecv(RECV_PIN);

decode_results results;

// Flag for reading values
bool isReading = false;

// Flag for password setting
bool settingPassword = false;

// Default PIN code
int password[4] = {num1, num2, num3, num4};

// Currently read code
int inputCode[4] = {num0, num0, num0, num0};
int currentEntry = 0;

// Timer to prevent reading mode from being enabled longer than required
Timer gracePeriod(5000, disable_reading_mode);

// Helper function for reading from arrays, checks if a value exists in a given array
int find_index(int a[], int num_elements, int value)
{
   for (int i = 0; i < num_elements; i++)
   {
	 if (a[i] == value)
	 {
	    return(value);
	 }
   }
   return(-1);  // if it was not found, return -1 as a default
}

// Just makes setting LEDs quicker
void toggle_LED(int pin) {
    if (digitalRead(pin) == HIGH)
    {
        digitalWrite(pin, LOW);
    }
    else
    {
        digitalWrite(pin, HIGH);
    }
}

Timer flashYellow(500, flash_yellow_LED);

void flash_yellow_LED() {
    toggle_LED(yellowLED);
}

Timer flashRed(500, flash_red_LED, true);

void flash_red_LED() {
    toggle_LED(redLED);
    delay(500);
    toggle_LED(redLED);   
}

// Switches reading mode on or off, displaying indicator with yellow LED
void toggle_reading_mode() {
    toggle_LED(yellowLED);
    
    if (isReading == true) {
        isReading = false;
    }
    else
    {
        isReading = true;
    }
    
    // Just as a safety, re-initialise the values so that we get a fresh read
    inputCode[0] = num0;
    inputCode[1] = num0;
    inputCode[2] = num0;
    inputCode[3] = num0;
    
    currentEntry = 0;
}

// Explicitly disables reading mode
void disable_reading_mode() {
    digitalWrite(yellowLED, LOW);
    
    isReading = false;
    
    // Just as a safety, re-initialise the values so that we get a fresh read
    inputCode[0] = num0;
    inputCode[1] = num0;
    inputCode[2] = num0;
    inputCode[3] = num0;
    
    currentEntry = 0;
}

// Compares the recent inputs to the password
void read_inputs() {
    Serial.println("Code: " + String(inputCode[0]) + " " + String(inputCode[1]) + " " + String(inputCode[2]) + " " + String(inputCode[3]));
    Serial.println("Password: " + String(password[0]) + " " + String(password[1]) + " " + String(password[2]) + " " + String(password[3]));
    
    // If all four digits are identical and in sequence, open the lock (toggle blue and red LEDs)
    if (inputCode[0] == password[0] && inputCode[1] == password[1] && inputCode[2] == password[2] && inputCode[3] == password[3]) {
        toggle_LED(blueLED);
        toggle_LED(redLED);
        toggle_LED(yellowLED);
        isReading = false;
        
        // Simulating "unlocked for 5 seconds" before reset
        delay(5000);
        toggle_LED(blueLED);
        toggle_LED(redLED);
    }
    else {
        flashRed.start();
    }
}

void set_password() {
    Serial.println("Code: " + String(inputCode[0]) + " " + String(inputCode[1]) + " " + String(inputCode[2]) + " " + String(inputCode[3]));
    Serial.println("Password: " + String(password[0]) + " " + String(password[1]) + " " + String(password[2]) + " " + String(password[3]));
    
    flashYellow.stop();
    digitalWrite(yellowLED, LOW);
    
    settingPassword = false;
    isReading = false;
}

void setup() {
    Serial.begin(9600);
    irrecv.enableIRIn();
  
    pinMode(redLED, OUTPUT);
    pinMode(yellowLED, OUTPUT);
    pinMode(blueLED, OUTPUT);
    
    digitalWrite(redLED, HIGH);
}

void loop() {
    // Check if an IR code is received
    if (irrecv.decode(&results)) {
        Serial.println(results.value);
        
        // If it's the EQ button, toggle reading mode
        if (results.value == eq) {
            currentEntry = 0;
            
            // Grace period of 5 seconds between key presses, if the timer expires, reading mode disables itself
            if (isReading == false) {
                gracePeriod.start();   
            }
            else
            {
                gracePeriod.reset();
                gracePeriod.stop();
            }
            
            toggle_reading_mode();
            Serial.println(isReading);
        }
        
        // If it's the CH button, toggle password change mode
        if (results.value == ch) {
            currentEntry = 0;
            isReading = true;
            settingPassword = true;
            flashYellow.start();
        }
        
        // If we're in password change mode, update the password
        if (isReading && settingPassword) {
            if (currentEntry < 4) {
                if (find_index(validInputs, 10, results.value) != -1) {
                    password[currentEntry] = results.value;
                    currentEntry++;
                }
                else {
                    set_password();
                }
                
            }
        }
        // If we're in reading mode, add current button press to the queue
        if (isReading && !settingPassword) {
            gracePeriod.reset();
            if (currentEntry < 4)
            {
                if (find_index(validInputs, 10, results.value) != -1) {
                    inputCode[currentEntry] = results.value;
                    currentEntry++;
                }  
            }
            else {
                read_inputs();
            }
        }
        irrecv.resume();
    }
}
