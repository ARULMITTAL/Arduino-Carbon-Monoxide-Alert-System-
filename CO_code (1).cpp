#include <TM1637Display.h>

#define CO_PIN A0
#define TM_CLK 2
#define TM_DIO 3

// RGB LED pins
#define RED 9
#define GREEN 10
#define BLUE 11

// timer
unsigned long start_time = 0;
bool timer_started = false;
unsigned long time_elapsed;

#define ALARM_PPM 200

TM1637Display display(TM_CLK, TM_DIO);

unsigned long lastRead = 0;
unsigned long lastPrint = 0;

long ppm = 0;

void setup() {
  Serial.begin(115200);

  // Initialize display
  display.setBrightness(7);
  display.showNumberDec(0, false);

  // Configure RGB LED pins
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);


  // Keep RGB LED OFF initially
  analogWrite(RED, 0);
  analogWrite(GREEN, 0);
  analogWrite(BLUE, 0);
}


void loop()
{
  unsigned long now = millis();

  // Read sensor every 200 ms
  if (now - lastRead >= 200) {
    lastRead = now;

    long sum = 0;


    for (byte i = 0; i < 20; i++) {
      sum += analogRead(CO_PIN);
    }

    // Calculate average sensor reading
    int raw = sum / 20;

    // Simplified
    ppm = ((raw - 20L) * 1000L) / 573L;

    if (ppm < 0) {
      ppm = 0;
    }

    // timer
    if (ppm >= ALARM_PPM){
      if (!timer_started){
        start_time = millis();
        time_elapsed = millis() - start_time;
        timer_started = true;
      }
      else if (timer_started){
        time_elapsed = millis() - start_time;
      }

      int mins = (time_elapsed / 60) / 1000;
      int secs = time_elapsed / 1000;
      int time = (mins * 100) + secs;

      display.showNumberDecEx(time, 0b01000000, true);
    }
    else{
      // Display estimated CO level
      int value = (ppm > 9999) ? 9999 : (int)ppm;
      display.showNumberDec(value, false);
    }


    // Set RGB LED colour based on estimated CO level
    if (ppm < ALARM_PPM) {
      // Normal: LED OFF
      timer_started = false;
      analogWrite(RED, 0);
      analogWrite(GREEN, 0);
      analogWrite(BLUE, 0);
    }

    if (timer_started && time_elapsed >= 6000){
      if (ppm >= 200 && ppm < 400) {
      // Yellow: 200–399 ppm
      analogWrite(RED, 255);
      analogWrite(GREEN, 255);
      analogWrite(BLUE, 0);
      }
      else {
        // Red: 400 ppm and above
        analogWrite(RED, 255);
        analogWrite(GREEN, 0);
        analogWrite(BLUE, 0);
    }
    }



    // Print readings every 2 seconds
    if (now - lastPrint >= 2000) {
      lastPrint = now;

      Serial.print("Raw = ");
      Serial.print(raw);
      Serial.print(" | Estimated CO = ");
      Serial.print(ppm);
      Serial.println(" ppm");
    }
  }
}

