#include "Arduino.h"
#include "power_stepper.h"



PowerStepper::PowerStepper(int step_pin, int direction_pin, int micro_delay = 1000): StepPin(step_pin), DirectionPin(direction_pin), MicroDelay(micro_delay), Task(0)
{
  pinMode(StepPin, OUTPUT);
  pinMode(DirectionPin, OUTPUT);
  LastExecuteTime = micros();
  digitalWrite(DirectionPin, HIGH);
}

bool PowerStepper::taskComplete() const
{
  if(Task == 0)
    return true;

  else
    return false;
}

bool PowerStepper::moveSteps(int steps)
{
  if (steps == 0)
    return false;
  
  Task += steps;
  return true;
}

void PowerStepper::doTasks()
{
  if(Task == 0)
    return ;

  long current = micros();
  if(current - LastExecuteTime >= MicroDelay)
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
  }
}