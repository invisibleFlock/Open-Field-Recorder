
#include <MP.h>
int LED = 0;
int led0 = 0;
int led1 = 0;
int led2 = 0;
int led3 = 0;
int ledall = 0;
unsigned long timer = 0;
int starter = 0;

void setup() {
  pinMode(7,OUTPUT);
  digitalWrite(7,HIGH); //initialise reset pin
  
  int ret = 0;

  ret = MP.begin();  //start multicore processing
  if (ret < 0) {
    errorLoop(2);
  }
MP.RecvTimeout(MP_RECV_POLLING);
}
void errorLoop(int num)  //if multicore doesn't start, flash led0
{
  int i;

  while (1) {
    for (i = 0; i < num; i++) {
      ledOn(LED0);
      delay(300);
      ledOff(LED0);
      delay(300);
    }
    delay(1000);
  }
}
void loop() {
  int      ret;
  int8_t   msgid;
  uint32_t msgdata;
  
  ret = MP.Recv(&msgid, &msgdata); // receive message from Main Core
  
if(msgdata == 6 && starter == 0){  // start core 1 checking for timeout on Main Core
  starter = 1;
  timer = millis();
}


if(starter == 1){
    if(msgdata == 1){         // on msg 1 (recording started), restart timer and change flashing LED to next LED
      timer = millis();
      if(LED == 0){
        ledOn(LED0);
        led0 = 1;
         (LED = 1);
      }
      if(LED == 2){
        ledOn(LED1);
        led1 = 1;
        (LED = 3);
      }
      if(LED == 4){
        ledOn(LED2);
        led2 = 1;
        (LED = 5);
      }
    }


    
    if(msgdata == 2){           // on msg 2 (recording finished), turn off flashng LED and reset timer
      timer = millis();
      if(LED == 1){
        ledOff(LED0);
        led0 = 0;
        (LED = 2);
      }
      if(LED == 3){
        ledOff(LED1);
        led1 = 0;
        (LED = 4);
      }
      if(LED == 5){
        ledOff(LED2);
        led2 = 0;
        (LED = 0);
      }
    }
    if(msgdata == 3){     // on msg 3 (file open error) turn on LED0
      led0 = 0;
      led1 = 0;
      led2 = 0;
      led3 = 0;
      ledOn(LED0);
      ledOff(LED1);
      ledOff(LED2);
      ledOff(LED3);
    }

    if(msgdata == 4){     // on msg 4 (read frames error) turn on LED1
      led0 = 0;
      led1 = 0;
      led2 = 0;
      led3 = 0;
      ledOff(LED0);
      ledOn(LED1);
      ledOff(LED2);
      ledOff(LED3);
    }

    if(msgdata == 5){     // on msg 5 (Error End) turn on LED2
      led0 = 0;
      led1 = 0;
      led2 = 0;
      led3 = 0;
      ledOff(LED0);
      ledOff(LED1);
      ledOn(LED2);
      ledOff(LED3);
    }

    if(msgdata == 7){     // on msg 7 (SD Card full) flash all LEDs
      led0 = 0;
      led1 = 0;
      led2 = 0;
      led3 = 0;
      ledall = 1;
    }
    
    if(led0 == 1){
      ledOff(LED0);
      led0 = 2;
      delay(2000);
    }
    if(led0 == 2){
      ledOn(LED0);
      led0 = 1;
      delay(100);
    }

    if(led1 == 1){
      ledOff(LED1);
      led1 = 2;
      delay(2000);
    }
    if(led1 == 2){
      ledOn(LED1);
      led1 = 1;
      delay(100);
    }
    if(led2 == 1){
      ledOff(LED2);
      led2 = 2;
      delay(2000);
    }
    if(led2 == 2){
      ledOn(LED2);
      led2 = 1;
      delay(100);
    }
    if(led3 == 1){
      ledOff(LED3);
      led3 = 2;
      delay(2000);
    }
    if(led3 == 2){
      ledOn(LED3);
      led3 = 1;
      delay(100);
    }

    if(ledall == 1){
      ledOff(LED0);
      ledOff(LED1);
      ledOff(LED2);
      ledOff(LED3);
      ledall = 2;
      delay(2000);
      timer = millis();
    }
    if(ledall == 2){
      ledOn(LED0);
      ledOn(LED1);
      ledOn(LED2);
      ledOn(LED3);
      ledall = 1;
      delay(100);
    }
  
    if((unsigned long)(millis()-timer) >= 660000){    // if timer goes on longer than 11 minutes without being reset, turn on all LEDs and reset board
      led1 = 3;
      led2 = 3;
      led3 = 3;
      ledOn(LED0);
      ledOn(LED1);
      ledOn(LED2);
      ledOn(LED3);
      digitalWrite(7,LOW);
    }

    if((((unsigned long)(millis()-timer)) <= 660000) && led1 == 3){
      led1 = 0;
      led2 = 0;
      led3 = 0;
      ledOff(LED0);
      ledOff(LED1);
      ledOff(LED2);
      ledOff(LED3);
    }
    
}
  // put your main code here, to run repeatedly:

}
