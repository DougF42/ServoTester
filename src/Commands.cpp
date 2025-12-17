/**
 * 
 */
#include <stdlib.h>
#include "Commands.h"
#include "string.h"
#include <errno.h>

/**
 * Initialize
 */
Commands::Commands(Servo *_servo)
{
    servo = _servo;
    nxtBufPos=0;
    stepState=STATE_IDLE;

    curMin=MIN_PW_DEFAULT;
    curMax=MAX_PW_DEFAULT;
    curPos=(curMax-curMin)/2;
    servo->writeMicroseconds(curPos);

    runFlag=0;
    totalSteps = DEFAULT_STEPCOUNT;
    duration = DEFAULT_LOOP_DURATION_MS;
    delayPerStep=1000; // default 1 sec between steps
    calcLoopParams();
    help();
}


/** = = = = = = = = = = = = = = = = = = = = = = = = = =
 * @brief Parse and adjust a time value
 * Adustthe  time, based on user'sinput.
 *    IF there is  a leading '+' or '-', this is an increment or decrement.
 *    otherwise, its an absolute value.
 * 
 * @param timeval - pointer to the actual vluetobe adjusted
 * @param token   - pointer to the string representing the change
 * @return true if successful, false iferror 
 *  = = = = = = = = = = = = = = = = = = = = = = = = = =
 */
bool Commands::parseTimeVal(CMDTIME_t *timeval, char *token, const char *name, CMDTIME_t lowLimit, CMDTIME_t highLimit)
{
    bool deltaFlag=false;
    CMDTIME_t val=0;
    if ((token[0]=='+') || (token[0] == '-'))
    {
        deltaFlag=true;
    }

    char *endptr;
    errno=0;
    val=strtol( token, &endptr, 10);
    if (*endptr != '\0')
    {
        Serial.print("Invalid value for "); Serial.println(name);
        return(false);
    }

    if ( (endptr!=nullptr) && (*endptr != '\0'))
    {
        Serial.print("Extra text after value for "); Serial.println(name);
        return(false);
    }
    
    if (deltaFlag)
    {
        *timeval += val;
    } else {
        *timeval = val;
    }

    if ((errno == ERANGE) || ((*timeval < lowLimit) || (*timeval > highLimit)))
        {
            Serial.print("Value is outside current limits for ");
            Serial.println(name);
            return (false);
        }
    return(true);
}

/** = = = = = = = = = = = = = = = = = = = = = = = = = =
 * output a list of all availablee commands AND the current settings
 *  = = = = = = = = = = = = = = = = = = = = = = = = = =
 */
void Commands::help()
{
    doFormFeed();
    Serial.println("COMMANDS:");
    Serial.println("help or ?  - this help message ");
    Serial.println("cur                             (report current position and settings)");
    Serial.println("min <uSeconds>                  (set min position)");
    Serial.println("max <uSeconds>                  (set max position)");
    Serial.println("set <uSeconds>                  (set current position)");
    Serial.println("cycle <noOfSteps> <time_msecs>  (Set the cycle params)");
    Serial.println("run  [<number of cycles>]       (start Running the cycle.)");
    Serial.println("stop                            (stop the current run cycle)");
    Serial.println("\n");
    showCurrentSettings();
}

/**  = = = = = = = = = = = = = = = = = = = = = = = = = =
 * Do a form feed to clear the screen
 *  = = = = = = = = = = = = = = = = = = = = = = = = = =
 */
 void Commands::doFormFeed()
 {
        char ff=0x0C;
        Serial.println(ff);

 }


/**  = = = = = = = = = = = = = = = = = = = = = = = = = =
 * output the current settings
 *  = = = = = = = = = = = = = = = = = = = = = = = = = =
 */
void Commands::showCurrentSettings()
{

    // Show current values
    Serial.println("CURRENT SETTINGS: ");
    Serial.print("* Cur POS SET             ");  Serial.println(curPos);
    Serial.print("- Cur POS actual          "); Serial.println(servo->readMicroseconds());
    Serial.print("* A cycle will Range from ");  Serial.print(curMin);       Serial.print(" to                 "); Serial.print(curMax); Serial.println(" uSecs");   
    Serial.print("* Cycle will be           ");  Serial.print(totalSteps);   Serial.print(" steps over         "); Serial.print(duration);  Serial.println(" milliseconds");
    Serial.print("- With a delay of          ");  Serial.print(delayPerStep); Serial.println(" milliseconds between each. ");    
    Serial.print("- Each step will increment "); Serial.print(stepSize);     Serial.println(" microseconds ");
    Serial.println("   ('*' indicates user defined parameters   '-' are calculated values)");

}



/** Get as many chars as available. IF CR/LF process the line */
void Commands::nextRead()
{
    while (Serial.available()>0)    
    {
        char ch = Serial.read();
        Serial.print(ch);  // echo the character, n matter what it is
        // printf("In nextRead - character is %d\n", ch);
        if ((ch==0x08) || (ch==0x7F))
        {   // BS or DEL (delete prev char)
            if (nxtBufPos >0)
            {
                nxtBufPos--;
            }

        } else  if ((ch=='\r') || (ch=='\n'))
        {   // End of line
            Serial.println(ch); // echo cr or lf
            buffer[nxtBufPos]='\0';
            processCommand(buffer, strlen(buffer));
            Serial.println("READY:");            
            nxtBufPos=0;
        } else
        {   // 'regular' character
            buffer[nxtBufPos++] = ch;
        }
        printf("\n");
        if (nxtBufPos >= sizeof(buffer))
        {
            Serial.println ("Buffer full - clearing buffer!");
            nxtBufPos=0;
        }
    }
}


/** = = = = = = = = = = = = = = = = = = = = = = = = = =
 * Process commands.
 *   It will be null termminated, and trailinig CR or NL not included.
 * 
 * @param cmdBuf - pointer to the buffer
 * @param buflen - how much of the buffer is full?
 *  = = = = = = = = = = = = = = = = = = = = = = = = = =
 */
void Commands::processCommand(char *cmdBuf, int buflen)
{
    char *tokList[MAX_TOKENS];
    // ignore blank lines...
    if (buflen <1) Serial.println(".");

    static const char *DELIM=" ,\t\n";
    
    // Non-blank line, lets tokenize (Spaces and commas, oh my!)
    int tokCount;
    for ( tokCount=0; tokCount<MAX_TOKENS-1; tokCount++)
    {
        if (tokCount==0)
        {   // first time
            tokList[tokCount] = strtok(cmdBuf, DELIM);
        } else {
            tokList[tokCount] = strtok( NULL, DELIM);
        }
        if (tokList[tokCount] == nullptr) break; // all done! List terminated by NULL
    }

    // NOW to do the command...
    #define isCommand(targ) (0==strcasecmp(tokList[0], targ))

    if (isCommand("?") || isCommand("help"))
    {
        help();
    }

    else if (isCommand("cur"))
    {
        if (tokCount != 1)
        {
            Serial.println("Wrong number of arguments for 'cur' command");
            return;
        }
        doFormFeed();
        showCurrentSettings();
    }
    
    else if (isCommand("min"))
    { // set the minimum (in uSecs)
        if (tokCount == 2)
        {
            parseTimeVal(&curMin, tokList[1], "Minimum ", MIN_PW_DEFAULT, MAX_PW_DEFAULT);
            calcLoopParams();
        }
        else if (tokCount != 1)
        {
            Serial.println("Wrong number of arguments for 'min' command");
            return;
        }
    
    Serial.print("Range is now ");    Serial.print(curMin);
    Serial.print(" through ");        Serial.println(curMax);
    }

    else if (isCommand("max"))
    { // Set the MAX time (uSecs)
        if (tokCount == 2)
        {
            parseTimeVal(&curMax, tokList[1], "Maximum ", MIN_PW_DEFAULT, MAX_PW_DEFAULT);
            calcLoopParams();
        }
        else if (tokCount != 1)
        {
            Serial.println("Wrong number of arguments for 'max' command");
            return;
        }
        Serial.print("Range is now "); Serial.print(curMin);
        Serial.print(" through ");     Serial.println(curMax);
    }

    else if (isCommand("set"))
    { // Set the current position in uSeconds
        if (tokCount == 2)
        {
            CMDTIME_t newPos;
            parseTimeVal(&newPos, tokList[1], "Position ", curMin, curMax);
            setCurPos(newPos);
        }
        else if (tokCount != 1)
        {
            Serial.println("Wrong number of arguments for 'set' command");
            return;
        }

        Serial.print("Current position is now ");
        Serial.println(curPos);
    }

    else if (isCommand("cycle")) // cycle Loop in ??? steps over ???? seconds
    {
        // cycle <time_msecs>   <noOfSteps>
        if (tokCount == 3)
        {
            if (!parseTimeVal(&duration, tokList[2], "Duration (milliseconds)", 300, 100000))
                return;
            if (!parseTimeVal(&totalSteps, tokList[1], "Steps ", 1, 1000))
                return;
            calcLoopParams();
        }
        else if (tokCount != 1)
        {
            Serial.println("Wrong number of arguments for 'cycle' command");
            return;
        }

        showCurrentSettings();

    }  else if (isCommand("run"))
    {
        calcLoopParams();
        if (tokCount == 2)
        { // decode number of times to do a cycle
            // how many times?
            if (!parseTimeVal(&runFlag, tokList[1], "Number of cycles", 0, 1000))
            {
                runFlag=0;
            }
        } else if (tokCount == 1)
        {
            runFlag = 1;
        }

        Serial.print("Initialize servo to MIN position (");Serial.print(curMin);Serial.println(")");
        setCurPos(curMin); // pout servo in Initial position
        stepDir = true;    // Going up...
        delay(1000);       // WAIT 1 second for servo toget to initial position
        Serial.print("Start run cycle. Repeat for "); Serial.print(runFlag);Serial.println(" times"); 

    } else if (isCommand("stop"))
    {
        runFlag=0;
        Serial.println("Cycle is now stopped");
    } else 
    {
        Serial.println("Unkown command");
    }
}


/** = = = = = = = = = = = = = = = = = = = = = = = = = =
 * Set the current position
 * = = = = = = = = = = = = = = = = = = = = = = = = = =
 */
void Commands::setCurPos(CMDTIME_t pos)
{
    servo->writeMicroseconds(pos);
    curPos = pos;
}


/**  = = = = = = = = = = = = = = = = = = = = = = = = = =
 *  Calculate some parameters for the 'run' command:
 *    delayPerStep
 *    stepSize
 *    lastTime
 *    runFlag
 *  = = = = = = = = = = = = = = = = = = = = = = = = = =
*/
void Commands::calcLoopParams()
{
        delayPerStep  =  (duration/2) / totalSteps;    // delay in milliseconds
        stepSize      = (curMax-curMin) / totalSteps; // number of steps
        count         = 0;

        delay(500);  // Time to get into 'min' position
        lastTime      = millis();
        runFlag       = 0;
}


/**  = = = = = = = = = = = = = = = = = = = = = = = = = =
 * LOOP - changing the Servo position each time
 * CALL FREQUENTLY FROM MAI 'loop()'
 * TODO:  Cycle BOTH up and DOWN!
 *  = = = = = = = = = = = = = = = = = = = = = = = = = =
 */
void Commands::loop()
{
    if (runFlag<=0)
    {
        return;
    }

    if ((millis() - lastTime) < delayPerStep)
    {
        return;
    }

    // Time for next step
    lastTime = millis();
    if (stepDir)
    { // Going up
        count++;
        curPos += stepSize;
        setCurPos(curPos);
        if (curPos >= curMax)
        {
            stepDir = false; // Now go down
        }

    }  else  {
        // Going down
        count++;
        curPos -= stepSize;
        setCurPos(curPos);
        if (curPos <= curMin)
        {
            stepDir = true;
            setCurPos(curMin);
            runFlag--;
            if (runFlag <=0)
            {
                Serial.println(" ");
                Serial.println("Run complete");
                runFlag=0; // make sure we are not negative
            }
        }
    }
    //Serial.print(curPos); Serial.print(" "); Serial.flush();

}