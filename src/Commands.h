/*
 * Read (and decode) commands from the serial port
 */
#pragma once
#include "Arduino.h"
#include "Configuration.h"
#include "Servo.h"

typedef  uint32_t CMDTIME_t;

class Commands
{
    private:
    char buffer[CMD_BUF_SIZE]; // More than enough room
    unsigned int nxtBufPos;
    Servo *servo; // point to the RunServo instance

    void processCommand(char *cmdBuf, int len);
    bool parseTimeVal(CMDTIME_t *timeval, char *token, const char *name, CMDTIME_t lowLimit, CMDTIME_t highLimit);
    void help(); 
    void showCurrentSettings();

    // THESE ARE SETTINGS
    CMDTIME_t curPos;         // limits, and actual current position (uSecs)
    CMDTIME_t curMin, curMax; // in uSeconds
    CMDTIME_t totalSteps;      //  Configured: How many steps will be taken?
    CMDTIME_t duration;       // Configured: How long will the 'up' sequence take (milliseconds)

    // These are ccalculated when we start the 'step' routine.
    CMDTIME_t stepSize;       // How much to change for each step? (uSecs)
    CMDTIME_t delayPerStep;   // How long between each step
    CMDTIME_t count;          // What is our current step count?
    CMDTIME_t lastTime;       // Time of last entry into loop (msecs)
    bool      stepDir;        // true going up, false going down
    CMDTIME_t runFlag;        // Run the loop, this many times (default is 1 time)

    void      setCurPos(CMDTIME_t pos);
    void      calcLoopParams();




    typedef enum STEP_STATE{STATE_IDLE, STATE_FWD, STATE_REV} STEP_STATE_t;
    STEP_STATE_t stepState;


    public:
        Commands(Servo *_servo);
        void nextRead(); // read (and process) more input. CALL FREQUENTLY
        void loop(); 
        void doFormFeed();

};