#ifndef accelerate_stepper_h
#define accelerate_stepper_h
#include "power_stepper.h"

class AccelerateStepper: public PowerStepper
{
public:
    AccelerateStepper(int step_pin, int direction_pin, int short_delay = 3, int long_delay = 10);
    void doTasks();

private:
    int LongDelay;
    int CurrDelay;
    double MaxSpeed;
    double MinSpeed;
    double CurrSpeed;

};





#endif