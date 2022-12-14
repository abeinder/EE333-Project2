
/* Sensorless brushless DC (BLDC) motor control with Arduino UNO (Arduino DIY ESC).
   This is a free software with NO WARRANTY.
   https://simple-circuit.com/
*/
// include the library code:
#include <LiquidCrystal.h>
//#include <Servo.h>
#include <Servo_Hardware_PWM.h>

#define PWM_MAX_DUTY      255
#define PWM_MIN_DUTY      20
#define PWM_START_DUTY    50

byte bldc_step = 0, motor_speed;
unsigned int i;
unsigned int a;

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 27, en = 26, d4 = 25, d5 = 24, d6 = 23, d7 = 22;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int NUM_ITERS_BLUETOOTH = 1;
const int NUM_ITERS_LCD = 50;

Servo servo1;
Servo servo2;

char msg[4];
char servo1Pos;
char servo2Pos;
bool increaseSpeed;
bool decreaseSpeed;
int q0;
int q1;
int q2;
int rw = 0;
int iters_b = 0;
int iters_l = 0;


void sendCommand(const char * command){
  Serial1.println(command);
  
  char reply[100];
  int i = 0;
  while (Serial1.available()) {
    reply[i] = Serial1.read();
    i += 1;
  }
  //end the string
  reply[i] = '\0';
}

void writeToBLE(char value) {
  Serial1.write(value);
}

void readFromBLE(char* reply) {
  int i = 0;
  while (Serial1.available()) {
    reply[i] = Serial1.read();
    i++;
  }

  reply[i] = '\0';
  
  
  return;
}

void handleBluetooth() {
  char message[20];

  int i;
  for (i=0; i<20; i++) {
    message[i] = ' ';
  }
  
  readFromBLE(message);
  int o = 0;
  int p = 0;

  char command[3];
  command[0] = 'A';
  command[1] = 'T';
  command[2] = '+';

  char blank[3];
  blank[0] = '\0';
  blank[1] = ' ';
  blank[2] = ' ';

  if (message[0] == command[0]) {
    if (message[1] == command[1]) {
      if (message[1] == command[2]) {
        o = 1;
      }
    }
  }

  if (message[0] == blank[0]) {
    if (message[1] == blank[1]) {
      if (message[1] == blank[2]) {
        o = 1;
      }
    }
  }
  
  delay(1);

  if (o != 1 && p != 1) {
    msg[0] = message[0];
    msg[1] = message[1];
    msg[2] = message[2];
    msg[3] = '\0';

    int q01;
    int q11;
    q01 = msg[0]-'0';
    q11 = msg[1]-'0';
    q2 = (msg[2]-'0'+48);

    //Serial.print(msg);
    //Serial.print("\n");

    if (q01 != -16 && q01 != 78 && q01 != 21 && q01 != 29 && q01 != 69) {
      q0 = (q01+48)*2;
      if (q0 < 55) {
        q0 = 55;
      }

      int speedCommand = msg[0] >> 5;
      //Serial.write("Speed command: ");
      byte speedVal = msg[0];
      //Serial.println(speedVal);
      if (speedCommand == 1) {
        increaseSpeed = true;
        Serial.write("Increase speed\n");
      } else if (speedCommand == 2) {
        decreaseSpeed = true;
        Serial.write("Decrease speed\n");
      }
      
      char servo1PosPotential = (((msg[0] - (speedCommand << 5)) + (msg[1] >> 6)) << 3)-1;
      byte valCast = servo1PosPotential;
      if (valCast < 128 && valCast != 7) {
        servo1Pos = servo1PosPotential;
        byte val = servo1Pos;
        //Serial.write("Servo 1 rotation: ");
        //Serial.println(val);
        //Serial.write("\n");
      }
      
      servo1Pos = servo1PosPotential;
      //Serial.write("Servo 1 rotation: ");
      //Serial.write(servo1Pos);
      //Serial.write("\n");
      
    }

    if (q11 != -16 && q11 != -48) {
      q1 = (q11+48)*2;

      char servo1LastBit = msg[1] >> 6;
      servo2Pos = (msg[1] - (servo1LastBit << 6)) << 2;
      byte val = servo2Pos;
      //Serial.write("Servo 2 rotation: ");
      //Serial.println(val);
      //Serial.write("\n");
    
      //sendCommand("AT+");
  }
  }
}

void handleLCD() {
  // set the cursor to column 0, line 1
    
    // (note: line 1 is the second row, since counting begins with 0):
    lcd.setCursor(0, 1);
    byte val1 = msg[0];
    
    lcd.print(val1);
    lcd.print("   ");
    //Serial.print(msg);
    //Serial.print("\n");
    
}

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
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.print("hello, world!");
  // Print a message to the LCD.
  delay(1000);

  Serial1.begin(115200);
  Serial.begin(115200);

  sendCommand("AT");

  servo1.attach(44);
  servo2.attach(45);
  
  
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
  i = 2000;
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

  while (1) {
    //Serial.write(msg);
    //Serial.write("\n");
      
    if (iters_b >= NUM_ITERS_BLUETOOTH) {
      handleBluetooth();
      //Serial.print(m);
      //Serial.print("\n");
      iters_b = 0;
      
      byte rot1 = servo1Pos;
      byte rot2 = servo2Pos;
      
      Serial.println(rot1);
      Serial.println(rot2);
      servo1.write(servo1Pos);
      servo2.write(servo2Pos);
    } else {
      iters_b++;
    }

    
  
    if (iters_l >= NUM_ITERS_LCD) {
      handleLCD();
      iters_l = 0;
    } else {
      iters_l++;
    }

    if (increaseSpeed && motor_speed < PWM_MAX_DUTY) {
      motor_speed = motor_speed + 10;
      SET_PWM_DUTY(motor_speed);
      increaseSpeed = false;
      delay(100);
      
    } else if (decreaseSpeed && motor_speed > PWM_MIN_DUTY) {
      motor_speed = motor_speed - 10;
      SET_PWM_DUTY(motor_speed);
      decreaseSpeed = false;
      delay(100);
      
    }
    
  }
  
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
