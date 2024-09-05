#include <Arduino.h>

#define bt_up          A0
#define bt_down        A1
#define on_off_bt      A7
#define max_duty      255
#define min_duty      50
#define start_pwm    100

byte bldc_step = 0, motor_speed = 100, bit_a7 = 0;
unsigned int i, time =0;

 
void BEMF_A_RISING(){
  ADCSRB = (0 << ACME);    
  ACSR |= 0x03;           
}
void BEMF_A_FALLING(){
  ADCSRB = (0 << ACME);    
  ACSR &= ~0x01;           
}
void BEMF_B_RISING(){
  ADCSRA = (0 << ADEN);  
  ADCSRB = (1 << ACME);
  ADMUX = 2;              
  ACSR |= 0x03;
}
void BEMF_B_FALLING(){
  ADCSRA = (0 << ADEN);   
  ADCSRB = (1 << ACME);
  ADMUX = 2;              
  ACSR &= ~0x01;
}
void BEMF_C_RISING(){
  ADCSRA = (0 << ADEN);   
  ADCSRB = (1 << ACME);
  ADMUX = 3;              
  ACSR |= 0x03;
}
void BEMF_C_FALLING(){
  ADCSRA = (0 << ADEN);   
  ADCSRB = (1 << ACME);
  ADMUX = 3;             
  ACSR &= ~0x01;
}
 
void AH_BL(){
  PORTB  =  0x0B;
  PORTD &= ~0x18;
  PORTD |=  0x20;
  TCCR1A =  0;            
  TCCR2A =  0x81;       
}
void AH_CL(){
  PORTB  =  0x0D;
  PORTD &= ~0x18;
  PORTD |=  0x20;
  TCCR1A =  0;           
  TCCR2A =  0x81;        
}
void BH_CL(){
  PORTB  =  0x0D;
  PORTD &= ~0x28;
  PORTD |=  0x10;
  TCCR2A =  0;            
  TCCR1A =  0x21;        
}
void BH_AL(){
  PORTB  =  0x07;
  PORTD &= ~0x28;
  PORTD |=  0x10;
  TCCR2A =  0;           
  TCCR1A =  0x21;        
}
void CH_AL(){
  PORTB  =  0x07;
  PORTD &= ~0x30;
  PORTD |=  0x08;
  TCCR2A =  0;            // Turn pin 9 (OC1A) PWM ON (pin 10 & pin 11 OFF)
  TCCR1A =  0x81;         //
}
void CH_BL(){
  PORTB  =  0x0B;
  PORTD &= ~0x30;
  PORTD |=  0x08;
  TCCR2A =  0;            // Turn pin 9 (OC1A) PWM ON (pin 10 & pin 11 OFF)
  TCCR1A =  0x81;         //
}
 
void set_pwm_duty(byte duty){
  OCR1A  = duty;                 
  OCR1B  = duty;                 
  OCR2A  = duty;          
      
}

void bldc_move(){        
  switch(bldc_step){
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
 
ISR (ANALOG_COMP_vect) {
  for(i = 0; i < 10; i++) {
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
}

void setup() {
  DDRD  |= 0x38;        
  PORTD  = 0x00;
  DDRB  |= 0x0E;        
  PORTB  = 0x0E;
 
  TCCR1A = 0;
  TCCR1B = 0x01;
 
  TCCR2A = 0;
  TCCR2B = 0x01;
  ACSR   = 0x10;           
  Serial.begin(9600);
}

void loop() {
  set_pwm_duty(motor_speed);  
  ACSR |= 0x08;                    
  if(digitalRead(bt_up) == 0 && motor_speed < max_duty){
      motor_speed++;
      set_pwm_duty(motor_speed);
    }
  if(digitalRead(bt_down) == 0 && motor_speed > min_duty){
      motor_speed--;
      set_pwm_duty(motor_speed);
  }
  if(millis() - time  >= 1000){
    time = millis();
    Serial.println(analogRead(A4));
  }
    
}
