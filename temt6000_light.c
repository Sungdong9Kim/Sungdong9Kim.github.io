#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <wiringPi.h>
#define LIGHT_SENSOR_IN 7
int main(void)
{
  if(wiringPiSetup() < 0)
  {
    fprintf(stderr, “Unable to setup wiringPi: %s\n”, strerror(errno));
    return 1;
  }
  pinMode(LIGHT_SENSOR_IN, INPUT);
  while(1)
  {
    if(digitalRead(LIGHT_SENSOR_IN) == 0)
    printf(“light dark\n”);
    if(digitalRead(LIGHT_SENSOR_IN) == 1)
    printf(“light full\n”);
    delay(1000); /// wait 1sec
  }
  return 0;
}
