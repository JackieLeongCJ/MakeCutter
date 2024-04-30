#ifndef power_stepper_h
#define power_stepper_h

class PowerStepper
{
    public:
        PowerStepper(int stepPin, int directionPin, int micro_delay = 1000);
        bool taskComplete() const;
        bool moveSteps(int steps);
        virtual void doTasks();

    protected:
        int Task;
        long LastExecuteTime;
        int StepPin;
        int DirectionPin;
        int MicroDelay;

};

#endif