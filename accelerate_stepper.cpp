#include "Arduino.h"
#include "accelerate_stepper.h"
#include "power_stepper.h"

AccelerateStepper::AccelerateStepper(int step_pin, int direction_pin, int short_delay, int long_delay): 
    PowerStepper(step_pin, direction_pin, short_delay * 1000), 
    LongDelay(long_delay * 1000), CurrDelay(long_delay*1000), 
    MaxSpeed(1.0/short_delay), MinSpeed(1.0/long_delay), CurrSpeed(1.0/long_delay)
{
}

void AccelerateStepper::doTasks()
{
    if(Task == 0)
    {
        CurrSpeed = MinSpeed;
        CurrDelay = 1000.0/MinSpeed;
        return ;
    }

    long current = micros();
    if(current - LastExecuteTime >= CurrDelay)
    {
        if(Task > 0)
        {
            Task --;
            digitalWrite(DirectionPin, HIGH);
        }
        else
        {
            Task ++;
            digitalWrite(DirectionPin, LOW);
        }

        digitalWrite(StepPin, HIGH);
        digitalWrite(StepPin, LOW);
        LastExecuteTime = micros();
        
        if((Task > -150 && Task < 150) && CurrSpeed > MinSpeed) // decelerate
        {
          Serial.print("decelerate, a: ");
          Serial.println((MaxSpeed - MinSpeed)/150);
            CurrSpeed -= (MaxSpeed - MinSpeed)/150;
            CurrDelay = 1000.0/CurrSpeed;
        }
        else if((Task < -100 || Task > 100) && CurrSpeed < MaxSpeed)
        {
            CurrSpeed += (MaxSpeed - MinSpeed)/100;
            CurrDelay = 1000.0/CurrSpeed;
        }
    }
}