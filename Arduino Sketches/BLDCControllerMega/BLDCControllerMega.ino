
/* Sensorless brushless DC (BLDC) motor control with Arduino UNO (Arduino DIY ESC).
   This is a free software with NO WARRANTY.
   https://simple-circuit.com/
*/
#include <Servo.h>

#define SPEED_UP          A0
#define SPEED_DOWN        A1
#define PWM_MAX_DUTY      255
#define PWM_MIN_DUTY      20
#define PWM_START_DUTY    150

byte bldc_step = 0, motor_speed;
unsigned int i;
unsigned int a;

Servo servo1;
Servo servo2;

void setup() {
  Serial.begin(115200);

  PORTE = 0x00;           // Pins 0-7
  PORTG = 0x00;
  PORTH = 0x00;
  PORTB = 0x00;

  DDRE |= 0x28;           // Configure pins 3, 4 and 5 as outputs
  DDRG |= 0x20;

  DDRH |= 0x40;           // Configure pins 9, 10 and 11 as outputs
  DDRB |= 0x30;

  // Timer1 module setting: set clock source to clkI/O / 1 (no prescaling)
  TCCR1A = 0;
  TCCR1B = 0x01;
  // Timer2 module setting: set clock source to clkI/O / 1 (no prescaling)
  TCCR2A = 0;
  TCCR2B = 0x01;
  // Analog comparator setting
  ACSR   = 0x10;           // Disable and clear (flag bit) analog comparator interrupt

  servo1.attach(13);
  servo1.write(0);

}
// Analog comparator ISR
ISR (ANALOG_COMP_vect) {
  // BEMF debounce
    for(i = 0; i < 150; i++) {
      if(bldc_step & 1){
        if(!(ACSR & 0x20)) i -= 1;
      }
      else {
        if((ACSR & 0x20))  i -= 1;
      }
    }
    //Serial.print("isr\n");
    bldc_move();
    bldc_step++;
    bldc_step %= 6;
  
}
void bldc_move() {       // BLDC motor commutation function
  switch (bldc_step) {
    case 0:
      AH_BL();
      BEMF_C_RISING();
      break;
    case 1:
      AH_CL();
      BEMF_B_FALLING();
      break;
    case 2:
      BH_CL();
      BEMF_A_RISING();
      break;
    case 3:
      BH_AL();
      BEMF_C_FALLING();
      break;
    case 4:
      CH_AL();
      BEMF_B_RISING();
      break;
    case 5:
      CH_BL();
      BEMF_A_FALLING();
      break;
  }
}

void loop() {
  SET_PWM_DUTY(PWM_START_DUTY);    // Setup starting PWM with duty cycle = PWM_START_DUTY
  i = 1000;
  delay(1000);
  // Motor start
  while(i > 20) {
    delayMicroseconds(i*10);
    bldc_move();
    bldc_step++;
    bldc_step %= 6;
    i = i - 20;
  }
  motor_speed = PWM_START_DUTY;
  ACSR |= 0x08;                    // Enable analog comparator interrupt
  
  ADCSRA = (0 << ADEN);   // Disable the ADC module
  ADCSRB = (1 << ACME);
  
}


////////////////////////////////////////////////////////////////////////////////////////
void BEMF_A_RISING() {
  ADMUX = 4;              // Select analog channel 4 as comparator negative input
  ACSR |= 0x03;           // rising edge
}
void BEMF_A_FALLING() {
  ADMUX = 4;              // Select analog channel 4 as comparator negative input
  ACSR &= ~0x01;          // falling edge
}
void BEMF_B_RISING() {
  ADMUX = 2;              // Select analog channel 2 as comparator negative input
  ACSR |= 0x03;           // rising edge
}
void BEMF_B_FALLING() {
  ADMUX = 2;              // Select analog channel 2 as comparator negative input
  ACSR &= ~0x01;          // falling edge
}
void BEMF_C_RISING() {
  ADMUX = 3;              // Select analog channel 3 as comparator negative input
  ACSR |= 0x03;           // rising edge
}
void BEMF_C_FALLING() {
  ADMUX = 3;              // Select analog channel 3 as comparator negative input
  ACSR &= ~0x01;          // falling edge
}

////////////////////////////////////////////////////////////////////////////////////////////
void AH_BL() {
  PORTH &= ~0x40; // Pin 9
  PORTB |= 0x10; // Pin 10
  PORTB &= ~0x20; // Pin 11

  PORTE &= ~0x20; // Pin 3
  PORTG &= ~0x20; // Pin 4
  PORTE |= 0x08; // Pin 5

  TCCR2A = 0;
  TCCR1A = 0x81; // clear OC1A on compare match - pin 11
}
void AH_CL() {
  PORTH |= 0x40; // Pin 9
  PORTB &= ~0x10; // Pin 10
  PORTB &= ~0x20; // Pin 11

  PORTE &= ~0x20; // Pin 3
  PORTG &= ~0x20; // Pin 4
  PORTE |= 0x08; // Pin 5

  TCCR2A = 0;
  TCCR1A = 0x81; // clear OC1A on compare match - pin 11
}
void BH_CL() {
  PORTH |= 0x40; // Pin 9
  PORTB &= ~0x10; // Pin 10
  PORTB &= ~0x20; // Pin 11

  PORTE &= ~0x20; // Pin 3
  PORTG |= 0x20; // Pin 4
  PORTE &= ~0x08; // Pin 5

  TCCR1A = 0;
  TCCR2A = 0x81; // clear OC2A on compare match - pin 10
}
void BH_AL() {
  PORTH &= ~0x40; // Pin 9
  PORTB &= ~0x10; // Pin 10
  PORTB |= 0x20; // Pin 11

  PORTE &= ~0x20; // Pin 3
  PORTG |= 0x20; // Pin 4
  PORTE &= ~0x08; // Pin 5

  TCCR1A = 0;
  TCCR2A = 0x81; // clear OC2A on compare match - pin 10
}
void CH_AL() {
  PORTH &= ~0x40; // Pin 9
  PORTB &= ~0x10; // Pin 10
  PORTB |= 0x20; // Pin 11

  PORTE |= 0x20; // Pin 3
  PORTG &= ~0x20; // Pin 4
  PORTE &= ~0x08; // Pin 5

  TCCR1A = 0;
  TCCR2A = 0x21; // clear OC2B on compare match - pin 9
}
void CH_BL() {
  PORTH &= ~0x40; // Pin 9
  PORTB |= 0x10; // Pin 10
  PORTB &= ~0x20; // Pin 11

  PORTE |= 0x20; // Pin 3
  PORTG &= ~0x20; // Pin 4
  PORTE &= ~0x08; // Pin 5

  TCCR1A = 0;
  TCCR2A = 0x21; // clear OC2B on compare match - pin 9
}

void SET_PWM_DUTY(byte duty) {
  if (duty < PWM_MIN_DUTY)
    duty  = PWM_MIN_DUTY;
  if (duty > PWM_MAX_DUTY)
    duty  = PWM_MAX_DUTY;

  OCR1A  = duty;                   // Set pin 11  PWM duty cycle
  OCR2B  = duty;                   // Set pin 9 PWM duty cycle
  OCR2A  = duty;                   // Set pin 10 PWM duty cycle
}
