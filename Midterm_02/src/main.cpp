#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

char mpaN = 1;  // Modifiable value for the moving point average N value (initialized to 1)
int mpaArr[9] = {0};
int mpaSum;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(10, 8, NEO_RGB);

int calculateNpointMovingAVG(int* mpaArr);
void UpdateUI(char mpaN);

/*   SETUP   */

void TIMER_setup() {
  cli();

  // Using timer 1
  // Direction is out for timer 1 (B6)
  pinMode(30, OUTPUT);

  TCCR1A = 0b00100011;
  /*         00 - OCR1A disconnected since im using for the TOP
               10 - OCR1B clears on a match
                 00 - OCR1C not used
                   11 - Bits 1:0 of WGM (1111) - Fast PWM up to OCR1A - mode 15
  */
  TCCR1B = 0b01011010;
  /*         0 - ICNC1 not used
              1 - ICES set to rising edge
               0 - not used
                11 - Bits 3:2 of WGM (1111) - Fast PWM up to OCR1A - mode 15
                  010 - prescale to 8, clock = 8Mhz/8 = 1Mhz, each tick = 0.8us
  */
  TCCR1C = 0b00000000;
  /*         000 - FOC1A not used
                00000 - not used  
  */

  TIMSK1 = 0b00100000;
  /*         00 - NA
               1 - ICIE1 enabled
                00 - NA
                  1 - ICIE1B is enabled (interupt executed when TCNT1 value matches OCRnB)
                   00 - NA
  */

  // Set TOP = 625
  // Now it takes 625*8us to reach 5ms
  OCR1A = 625;
  // Set OCR1B to be 0 (100% duty cycle)
  OCR1B = 0;

  sei();
}

void ADC_setup() {
  cli();

  // Using ADC1
  // Direction is in for ADC1 (F1)
  pinMode(40, INPUT);

  ADMUX = 0b01000001;
  /*        01 - Use Vcc as ref
              0 for adlar to right justify
               00001 - ADC1
  */
  ADCSRA = 0b10101010;
  /*         1 - enable ADC
              0 - hold off while I configure
               1 - Auto trigger enabled
                0 - Interrupt flag
                 1 - ADC interupt enable
                  010 - clock prescale to 4   
  */
  ADCSRB = 0b00000000;
  /*         0 - Not high speed mode
              0 - NA
               0 - Bit 5 of ADC selection (ADC1)
                0000 -   
  */

  sei();
}


void setup() {
  cli();

  // Enable global interupts
  SREG |= (1 << SREG_I);

  TIMER_setup();
  ADC_setup();

  Serial.begin(9600);

  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'


  sei();
}
/*   SETUP   */


/*   INTERUPTS   */
char adcData;
int hRmovingAvg;
ISR(ADC_vect) {
  // calculate the N point moving avg
  hRmovingAvg = calculateNpointMovingAVG(mpaArr);
  // Calculate the data from the ADC after conversion
  adcData = map(ADCW, 0, 1023, 0, 330);
  // Store the new found data in an array
  mpaArr[0] = adcData;
  // Reset the summation value for the average
  mpaSum = adcData;
  // Print the found average
  Serial.print(">HR: ");
  Serial.println(hRmovingAvg);

  sei();
}

// Timer1 interupt is executed when TCNT1 value matches OCR1B 
// (right after timer reaches max, i.e. 5ms)
ISR(TIMER1_COMPB_vect) {
  cli();

  // Start converting on the ADC
  ADCSRA |= (1 << ADSC);

  sei();
}
/*   INTERUPTS   */


void loop() {
  cli();

  // Check for input from user
  if (Serial.available()) {
    mpaN = Serial.read();   // Change the N value
  }

  delay(100);

  sei();
}


int calculateNpointMovingAVG(int* mpaArr) {
  cli();

  // If the mpa value is only 1, then there is no calculation to do 
  // so just return the current value
  if (mpaN == 1) return mpaSum;

  // Otherwise, move each value to the index to the right of it
  for (int idx = mpaN-2; idx >= 0; idx--) {
    mpaSum += mpaArr[idx];
    mpaArr[idx+1] = mpaArr[idx];
  }

  UpdateUI(mpaN);  // Update NeoPixels to reflect N value
  return mpaSum;

  sei();
}


void UpdateUI(char mpaN) {
  cli();

  // First clear all the NeoPixels
  strip.show();
  // Then turn on the amount of NeoPixels corresponding to mpaN
  for (int i = 0; i <= mpaN; i++) {
    strip.setPixelColor(i, strip.Color(0, 150, 0));
  }

  sei();
}