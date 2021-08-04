// #include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "light_ws2812.h"

#define RESET_PIN PB1
#define STROBE_PIN PB3
#define ANALOG_PIN PD2

#define BANDS 7

struct cRGB leds[28];

void read_msgeq7(int spectrum[7]);
void adc_init(void);
uint16_t adc_read(uint8_t ch);
int main(void) {
  // #ifdef __AVR_ATtiny10__
  // CCP=0xD8;		// configuration change protection, write signature
  // CLKPSR=0;		// set cpu clock prescaler =1 (8Mhz) (attiny 4/5/9/10)
  // #endif

  DDRB |= (1 << RESET_PIN);
  DDRB |= (1 << STROBE_PIN);
  DDRB &= ~(1 << 4);
  // PORTB &= ~(1 << RESET_PIN);
  // PORTB |= (1 << STROBE_PIN);
  adc_init();

  for (;;) {
    int spectrum[7];
    read_msgeq7(spectrum);
    for (int i = 0; i < BANDS; i++) {
      for (int j = 0; j < 4; j++) {
	leds[(4*i)+j].r = spectrum[i];
	leds[(4*i)+j].g = 0;
	leds[(4*i)+j].b = 0;
      }
    }
    ws2812_setleds(leds, 7*4);
  }
}

// TODO: Smooth out the bars so only the two in the middle are the brightest,
// this more closely matches the shape of the frequency response on the
// datasheet.
void update_display(int spectrums[6][7]) {
  int intensity;
  int loudest_band;


  for (int s = 0; s < DISPLAY_DEPTH; s++) {
    loudest_band = max_index(spectrums[s]);

    for (int b = 0; b < BANDS; b++) {
      if (s % 2 == 0) {
        intensity = map(spectrums[s][(BANDS - 1) - b], 0, 1024, 0, 255);
      } else {
        intensity = map(spectrums[s][b], 0, 1024, 0, 255);
      }
      for (int i = 0; i < DISPLAY_WIDTH / BANDS; i++) {
        int display_index = (s * 28) + (b * 4) + i;
        uint32_t color;
        color = intensity_color(intensity, loudest_band);
        display.setPixelColor(display_index, color);
      }
    }
  }
  display.show();
}

// This function updates it's input array with the latest values.
//
// The array's structure is defined by the MSGEQ7, and has the following
// values: [63Hz, 160Hz, 400Hz, 1kHz, 2.5kHz, 6.25kHz, 16kHz].
void read_msgeq7(int spectrum[7]) {
  // TODO: Dynamically set this value somehow?
  int spectrumOffset[7] = { 60, 74, 68, 60, 62, 60, 60 };

  // digitalWrite(RESET_PIN, HIGH);
  PORTB |= (1 << RESET_PIN);
  // digitalWrite(RESET_PIN, LOW);
  PORTB &= ~(1 << RESET_PIN);

  for (int i = 0; i < BANDS; i++) {
    // digitalWrite(STROBE_PIN, LOW);
    PORTB &= ~(1 << STROBE_PIN);
    _delay_us(30);
    // spectrum[i] = analogRead(ANALOG_PIN) - spectrumOffset[i];
    spectrum[i] = adc_read(ANALOG_PIN) - spectrumOffset[i];
    if (spectrum[i] < 0) {
      spectrum[i] = 0;
    }
    // digitalWrite(STROBE_PIN, HIGH);
    PORTB |= (1 << STROBE_PIN);
  }
}

void adc_init(void)
{
  // // AREF = AVcc
  // ADMUX = (1<<REFS0)|

  // ADC Enable and prescaler of 128
  // 16000000/128 = 125000
  ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

uint16_t adc_read(uint8_t ch)
{
  // select the corresponding channel 0~7
  // ANDing with ’7′ will always keep the value
  // of ‘ch’ between 0 and 7
  ch &= 0b00000111;  // AND operation with 7
  ADMUX = (ADMUX & 0xF8)|ch; // clears the bottom 3 bits before ORing

  // start single convertion
  // write ’1′ to ADSC
  ADCSRA |= (1<<ADSC);

  // wait for conversion to complete
  // ADSC becomes ’0′ again
  // till then, run loop continuously
  while(ADCSRA & (1<<ADSC));

  return (ADC);
}
