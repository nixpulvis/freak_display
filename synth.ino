#include <Adafruit_NeoPixel.h>

#define NUMLEDS 28 * 6

int analogPin = 0;
int strobePin = 9;
int resetPin = 8; // reset is attached to digital pin 3
int spectrumValue[7]; // to hold a2d values

Adafruit_NeoPixel leds = Adafruit_NeoPixel(NUMLEDS, 11, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(9600);

  // Setup MSEQ7.
  pinMode(analogPin, INPUT);
  pinMode(strobePin, OUTPUT);
  pinMode(resetPin, OUTPUT);
  analogReference(DEFAULT);
  digitalWrite(resetPin, LOW);
  digitalWrite(strobePin, HIGH);

  // Setup Lights.
  leds.begin();
  leds.setBrightness(255);
}

void loop() {
  // Read MSEQ7.
  digitalWrite(resetPin, HIGH);
  digitalWrite(resetPin, LOW);
  for (int i = 0; i < 7; i++) {
    digitalWrite(strobePin, LOW);
    delayMicroseconds(30);
    spectrumValue[i] = analogRead(analogPin);
    digitalWrite(strobePin, HIGH);
  }

  // Set lights.
  //
  // --------------------------->
  // <---------------------------
  // --------------------------->
  // <---------------------------
  // --------------------------->
  // <--------------------------0
  for (int band = 0; band < 7; band++) {
    int intensity = audio_to_luminance(spectrumValue[6 - band]);
    leds.setPixelColor(band + 10, leds.Color(intensity, 0, 0));
  }
  leds.show();
}

int audio_to_luminance(int audio_sample) {
  if (audio_sample <= 75) {
    return 0;
  } else {
    return map(audio_sample, 0, 1024, 0, 255);
  }
}


