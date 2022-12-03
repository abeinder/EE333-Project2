// include the library code:
#include <LiquidCrystal.h>
#include <Servo.h>

#define PWM_MAX_DUTY      255
#define PWM_MIN_DUTY      50
#define PWM_START_DUTY    100

byte bldc_step = 0, motor_speed;
unsigned int i;
unsigned int a;
unsigned int e = 0;

char command[3];

char blank[3];

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 28, en = 29, d4 = 25, d5 = 24, d6 = 23, d7 = 22;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int NUM_ITERS_BLUETOOTH = 1000;
const int NUM_ITERS_LCD = 100;

char msg[4];
char msg1[4];
int rw = 0;
int iters_b = 0;
int iters_l = 0;

Servo servo1;
Servo servo2;

// --------------------------------------

void sendCommand(const char * command){
  Serial1.println(command);
  
  char reply[100];
  int u = 0;
  while (Serial1.available()) {
    reply[u] = Serial1.read();
    u += 1;
  }
  //end the string
  reply[u] = '\0';
}

void writeToBLE(char value) {
  Serial1.write(value);
}

void readFromBLE(char* reply) {
  
  //ACSR   = 0x10;           // Disable and clear (flag bit) analog comparator interrupt
  int u = 0;
  while (Serial1.available()) {
    reply[u] = Serial1.read();
    u++;
  }
  
  //ACSR |= 0x08;                    // Enable analog comparator interrupt

  reply[u] = '\0';
  
  Serial.print(reply[0]);
  Serial.print(reply[1]);
  Serial.print(reply[2]);
  Serial.print(reply[3]);
  Serial.print(reply[4]);
  Serial.print(reply[5]);
  Serial.print(reply[6]);
  Serial.print(reply[7]);
  Serial.print(reply[8]);
  Serial.print(reply[9]);
  Serial.print(reply[10]);
  Serial.print(reply[11]);
  Serial.print(reply[12]);
  Serial.print(reply[13]);
  Serial.print(reply[14]);
  Serial.print(reply[15]);
  Serial.print("\n");
  
  return;
}


void handleBluetooth() {
  char message[20];
  
  int u;
  for (u=0; u<20; u++) {
    message[u] = ' ';
  }
  readFromBLE(message);
    int o = 0;
    int p = 0;
    int v = 0;
    
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

      msg1[0] = message[0];
      msg1[1] = message[1];
      msg1[2] = message[2];

        
      msg[0] = message[0];
      msg[1] = message[1];
      msg[2] = message[2];
      msg[3] = '\0';
      
      //sendCommand("AT+");
    }

  return;
}

void handleLCD() {
  // set the cursor to column 0, line 1
    
    // (note: line 1 is the second row, since counting begins with 0):
    lcd.setCursor(0, 1);
    lcd.print(msg);
    lcd.print("   ");
    //Serial.print(msg);
    //Serial.print("\n");
    delay(1);
    
}


// ----------------------------------------

// Analog comparator ISR
ISR (ANALOG_COMP_vect) {
  // BEMF debounce
    for(i = 0; i < 100; i++) {
      if(bldc_step & 1){
        if(!(ACSR & 0x20)) i -= 1;
      }
      else {
        if((ACSR & 0x20))  i -= 1;
      }
    }
    
    bldc_move();
    bldc_step++;
    bldc_step %= 6;
    e = 1;
  
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
  //Serial.print("c\n");
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

  TCCR1A = 0x81;
  TCCR2A = 0;
}
void AH_CL() {
  PORTH |= 0x40; // Pin 9
  PORTB &= ~0x10; // Pin 10
  PORTB &= ~0x20; // Pin 11

  PORTE &= ~0x20; // Pin 3
  PORTG &= ~0x20; // Pin 4
  PORTE |= 0x08; // Pin 5

  TCCR1A = 0x81;
  TCCR2A = 0;
}
void BH_CL() {
  PORTH |= 0x40; // Pin 9
  PORTB &= ~0x10; // Pin 10
  PORTB &= ~0x20; // Pin 11

  PORTE &= ~0x20; // Pin 3
  PORTG |= 0x20; // Pin 4
  PORTE &= ~0x08; // Pin 5

  TCCR1A = 0;
  TCCR2A = 0x81;
}
void BH_AL() {
  PORTH &= ~0x40; // Pin 9
  PORTB &= ~0x10; // Pin 10
  PORTB |= 0x20; // Pin 11

  PORTE &= ~0x20; // Pin 3
  PORTG |= 0x20; // Pin 4
  PORTE &= ~0x08; // Pin 5

  TCCR1A = 0;
  TCCR2A = 0x81;
}
void CH_AL() {
  PORTH &= ~0x40; // Pin 9
  PORTB &= ~0x10; // Pin 10
  PORTB |= 0x20; // Pin 11

  PORTE |= 0x20; // Pin 3
  PORTG &= ~0x20; // Pin 4
  PORTE &= ~0x08; // Pin 5

  TCCR1A = 0;
  TCCR2A = 0x21;
}
void CH_BL() {
  PORTH &= ~0x40; // Pin 9
  PORTB |= 0x10; // Pin 10
  PORTB &= ~0x20; // Pin 11

  PORTE |= 0x20; // Pin 3
  PORTG &= ~0x20; // Pin 4
  PORTE &= ~0x08; // Pin 5

  TCCR1A = 0;
  TCCR2A = 0x21;
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

// -----------------------------------------

void setup() {


  command[0] = 'A';
  command[1] = 'T';
  command[2] = '+';
  
  blank[0] = '\0';
  blank[1] = ' ';
  blank[2] = ' ';
  
  // put your setup code here, to run once:
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");
  delay(100);

  Serial1.begin(115200);
  Serial.begin(115200);

  sendCommand("AT");


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
  
  servo1.attach(12);
  servo2.attach(13);

  
}

void loop() {
  // put your main code here, to run repeatedly:
  

  SET_PWM_DUTY(PWM_START_DUTY);    // Setup starting PWM with duty cycle = PWM_START_DUTY
  i = 5000;
  // Motor start
  delay(1000);
  while (i > 100) {
    delayMicroseconds(i);
    bldc_move();
    bldc_step++;
    bldc_step %= 6;
    i = i - 20;
  }

  motor_speed = PWM_START_DUTY;
  ACSR |= 0x08;                    // Enable analog comparator interrupt

  //Serial.print("main");
  ADCSRA = (0 << ADEN);   // Disable the ADC module
  ADCSRB = (1 << ACME);
  
  while (1) {
    /*while (!(digitalRead(SPEED_UP)) && motor_speed < PWM_MAX_DUTY) {
      motor_speed++;
      SET_PWM_DUTY(motor_speed);
      Serial.print("u\n");
      delay(100);
    }
    while (!(digitalRead(SPEED_DOWN)) && motor_speed > PWM_MIN_DUTY) {
      motor_speed--;
      SET_PWM_DUTY(motor_speed);
      Serial.print("d\n");
      delay(100);
    }*/

    int m;
    m = atoi(msg);
    analogWrite(12, m);
    analogWrite(13, m);
    Serial.write(msg);
    Serial.write("\n");
    

  
    if (iters_b >= NUM_ITERS_BLUETOOTH) {
      handleBluetooth();
      iters_b = 0;
    } else {
      iters_b++;
      //Serial.print("it: ");
      //Serial.print(iters_b);
      //Serial.print("\n");
    }
  
    if (iters_l >= NUM_ITERS_LCD) {
      handleLCD();
      iters_l = 0;
    } else {
      iters_l++;
      //Serial.print("i lt: ");
      //Serial.print(iters_l);
      //Serial.print("\n");
    }

    
  }
    
  
}
