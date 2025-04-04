#include <Arduino.h>


void setupTimerPWM() {
  // USING TIMER 1

  // Direction is out for port B pin 6
  // Using this pin as the output of the timer
  DDRB |= (0 << 6);

  TCCR1A = 0b00100011;
  /*         00 - OCR1A disconnected since it is used as the TOP
               10 - OCR1B clears on a match
                 00 - OCR1C not used
                   11 - Bits 1:0 of WGM (1111) - Fast PWM up to OCR1A - mode 15
  */
  TCCR1B = 0b00011001;
  /*         0 - ICNC1 not used
              0- ICES trigger not used
               0 - not used
                11 - Bits 3:2 of WGM (1111) - Fast PWM up to OCR1A - mode 15
                  001 - no prescaling, clock = 8Mhz, each tick = 0.125us
  */
  TCCR1C = 0b00000000;
  /*         000 - FOC1A not used
                00000 - not used  
  */

  // Set TOP
  // 8000 * 0.125us = 1000us = (1Khz)
  OCR1A = 8000;
  // Set OCR1B to be 50% duty cycle (half of OCR1A)
  OCR1B = OCR1A/2;
}


void setPWMFreq(uint16_t freqInHz) {
  if ((freqInHz < 1000) | (freqInHz > 4000)) return;
  char multiplier = freqInHz/1000;
  OCR1A = multiplier * 8000;
  OCR1B = OCR1A/2;
}


void setupADC() {
  // USING ADC 9

  // Input for ADC 9 is D6 and direction is in
  DDRD &= ~(1 << 6);
  
  ADMUX = 0b01100001;
  /*        01 - Reference voltage for the sensor (from the schematic its 3.3V)
              1 - For adlar to left adjust
               00001 - ADC 9 (bits 4:0)
  */
  ADCSRA = 0b10100010;
  /*         1 - ADC enable bit
              0 - Bit for starting the conversion (set to 0 for now until we are ready to convert)
               0 - We want to be able to auto trigger the ADC
                0 - Not using interupts
                 0 - Not using interupts
                  010 - Prescale to 4
  */
  ADCSRB = 0b00100000;
  /*         0 - Not HS mode
              0 - NA
               1 - ADC 9 (bit 5)
                0 - NA
                 0000 - No auto trigger (free running mode)
  */

  // set Data Input for ADC 9 to be off
  DIDR2 &= ~(0 << 1);

  // setting the ADC to start conversion
  ADCSRA |= (1 << ADSC);
}


uint16_t translateADCtoFreq(uint16_t ADCval) {
  // map the ADC registers values to the freq range of 1k - 4k
  return map(ADCval, 0, 65535, 1000, 4000);
} 


void setupLEDs() {
  // USING PORTS B10, B9, D6 FOR LED'S
  // Data direction is out
  DDRB |= (1 << 6); // LED 1
  DDRB |= (1 << 5); // LED 2
  DDRD |= (1 << 7); // LED 3

  // All led's initially off
  PORTB &= (0 << 6);
  PORTB &= (0 << 5);
  PORTD &= (0 << 7);
}


void updateLEDs(uint8_t Percent) {
  if (Percent < 33) {                            // 0 - 33%
    PORTB |= (1 << 6);  // LED 1 is on
    PORTB &= (0 << 5);  // LED 2 is off
    PORTD &= (0 << 7);  // LED 3 is off
  }
  else if ((Percent >= 33) & (Percent < 66)) {   // 33 - 65%
    PORTB |= (1 << 6);  // LED 1 is on
    PORTB |= (1 << 5);  // LED 2 is on
    PORTD &= (0 << 7);  // LED 3 is off
  }
  else if ((Percent >= 66) & (Percent < 100)) {   // 66 - 100%
    PORTB |= (1 << 6);  // LED 1 is on
    PORTB |= (1 << 5);  // LED 2 is on
    PORTD |= (1 << 7);  // LED 3 is on
  }
}


void setup() {
  setupLEDs();
  setupTimerPWM();
  setupADC();
}


void loop() {
  // Calculate FreqPercent from the value of ADCW
  uint8_t FreqPercent;
  FreqPercent = map(ADCW, 0, 65535, 0, 100);

  setPWMFreq(translateADCtoFreq(ADCW)); //Read ADC and set the PWM Freq
  updateLEDs(FreqPercent);
  delay(100); //update 10 times a sec.
}
