#include <Servo.h>

Servo ctrlservo;
int pos=0;
void setup()
{
  ctrlservo.attach(3);
}

void loop()
{
 for(pos=0;pos<=180;pos++)
 {
   ctrlservo.write(pos);
   delay(50);
 }
 for(pos=180;pos>=0;pos--)
 {
   ctrlservo.write(pos);
   delay(100);
 }

  
}