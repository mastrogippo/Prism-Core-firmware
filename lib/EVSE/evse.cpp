// EVSE car interface library for Prism Core
// Copyright (C) 2019 Mastro Gippo
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.

#include "evse.h"
#include "eeprom_flash.h" 

//NOTE! Modify line 106 of ..\framework-mbed\targets\TARGET_STM\TARGET_STM32F1\analogin_api.canalog_api.c
//This is to increase ADC sampling time to allow ADC cap charging due to high input impedance
//    sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;//ADC_SAMPLETIME_7CYCLES_5;

////
//DigitalOut ph(PB_12);
//Serial pc(PA_9, PA_10,115200);

void EVSE::SetCurrentLimit(uint16_t c)
{
    //Hardcoded formal checks
    if(c < 60) c = 60;
    if(c > 320) c = 320;
    CurrentLimit = c;

    //TODO: save in flash
}

uint16_t EVSE::GetCurrentLimit()
{
    return CurrentLimit;
}

void EVSE::LoadCurrentLimit()
{
    //TODO: load from flash
    CurrentLimit = 320;
}

void EVSE::attachAuth(char(*fn)(char eID))
{
    callbackAuth = fn;
}

bool EVSE::cbAuth()
{
    //TODO: add some checks, decide what to do if null
    if(callbackAuth == NULL) 
        return true;

    char c = callbackAuth(ID);
    if(c == 0x55)
        return true;
    else
        return false;
}

void EVSE::attach(void(*fn)(char eID, cst NewStatus))
{
    callback = fn;
}

void EVSE::cb(cst NewStatus)
{
    if(callback == NULL) return;
    callback(ID, NewStatus);
}

EVSE::cst EVSE::getStatus()
{
    return chg_status;
}
/*char* EVSE::getStatusStr()
{
    return StatD[chg_status];
}*/
void EVSE::set_commanded_current_pwm()
{
    float tmp = OUTpin->read();

    //TODO: check this better
    if(chg_status == cNC)
        return;

    //check boundaries
    //set_commanded_current(commanded_current);
    if(commanded_current < 60)
        commanded_current = 60;
    if(commanded_current > 510)
        commanded_current = 60;

    if(commanded_current > CurrentLimit)
        commanded_current = CurrentLimit;

    commanded_pwm = roundf((float)(commanded_current) / 6.0f) / 100.0f;
    //commanded_pwm = ((float)(commanded_current) / 6.0f) / 100.0f;

    if(tmp != commanded_pwm)
        OUTpin->write(commanded_pwm);
//        OUTpin->write(0.1);
}

void EVSE::set_commanded_current(int16_t current_dA)
{
    commanded_current = current_dA;

    //TODO: more than a formal validity check?
    //TODO: find a betterwayto check limits
    /*if(commanded_current < 60)
        commanded_current = 60;
    if(commanded_current > 510)
        commanded_current = 60;
    */

    set_commanded_current_pwm();
}

void EVSE::RCDstop()
{
    //TODO: check
    StopCharging();
    setNewStatus(cERRRCD);
}
void EVSE::RCDreset()
{
    //TODO: check
    if(chg_status == cERRRCD)
        setNewStatus(cNC);
}

void EVSE::StartCharging()
{
    //TODO: add checks and stuff?
    SessionStartEnergy = ReadEnergy();
    RLYpin->write(1);
}

void EVSE::StopCharging()
{
    //TODO: add checks and stuff?
    //SessionStartEnergy = ReadEnergy();
    RLYpin->write(0);
}

//Start request from outside
void EVSE::StartCharge()
{
    //TODO: add checks and stuff?
    if((chg_status == cCharge) && (!AutoStart))
        StartCharging();
}

void EVSE::setNewStatus(cst NewStatus)
{
    float tmp = OUTpin->read();
    chg_status = NewStatus;
    
    if(chg_status != cCharge)
        StopCharging();

    switch(chg_status)
    {
        case cReady:
            set_commanded_current_pwm();
            StopCharging();
            break;
        case cCharge:
            //set_commanded_current_pwm(); //Should already be set
            if(AutoStart)
                StartCharging();
            break;
        case cERRRCD:
        case cNC:
            if(tmp != 1)
            {
                OUTpin->write(1); //DC high
            }
            break;
        default:
            break;
    }
}

void EVSE::ChangeStatus(cst NewStatus)
{
    static volatile cst tmp_st;
    static volatile int cntCstatus = 0;

    if(chg_status != NewStatus) //Switch status
    {
        if(tmp_st != NewStatus)
        {//reset counter
            //printf("[*]");
            cntCstatus = 0;
            tmp_st = NewStatus;

            if(chg_status == cNC) //lower delay
                cntCstatus = 5;
        }
        if(cntCstatus++ > numAVG)
        {//received the request for more than [numAVG] times, changing status
            tmp_st = cNULL;
            cntCstatus = 0;
            setNewStatus(NewStatus);
        }
    }   
}

void EVSE::CheckPilotIdle()
{
    if(chg_status == cERRRCD)
    {
        //do nothing until RCD error is cleared
        return;
    }

    //callback on status change
    if(notify_status != chg_status)
    {
        notify_status = chg_status;
        cb(notify_status);
    }

    if(chg_status == cNC)
    {
        gh = FBpin->read_u16();
        if((gh < mVAL_12V) && (gh > mVAL_9V)) //>9V potrebbe essere un problema??
        {
            //my_pwm.write(0.1); //10%
            ChangeStatus(cReady);
        }
    }
}
void EVSE::pintH()
{
    wait_us(20);
    //ph=1;
    gh = FBpin->read_u16();
    //ph=0;
    
}
void EVSE::pintL()
{
    wait_us(20);
    //ph=1;
    gl = FBpin->read_u16();

    //pc.printf("l%d h%d \n\r", gl,gh);

    if(gl > mVAL_min) //ERR
    {
        ChangeStatus(cERR3);
        //TODO: Start timer to reset comm
    }
    else
    {
        if(gh >= mVAL_12V)
        {
            ChangeStatus(cNC);
        }    
        else if((gh < mVAL_12V) && (gh >= mVAL_9V))
        {
            ChangeStatus(cReady);
        }
        else if((gh < mVAL_9V) && (gh >= mVAL_3V))
        {
            ChangeStatus(cCharge);
        }
        /*else if((gh < mVAL_9V) && (gh >= mVAL_6V))
        {
            ChangeStatus(cCharge);
        }
        else if((gh < mVAL_6V) && (gh >= mVAL_3V))
        {
            ChangeStatus(cVent);
        }*/
        else if(gh <= mVAL_3V)
        {
            ChangeStatus(cERR1);
            //TODO: Start timer to reset comm
        }
        //else printf("*");
    }
    //ph=0;
}
 
EVSE::~EVSE()
{
    delete FBpin;
    delete INTpin;
    delete OUTpin;
    delete CheckPilot;
}

EVSE::EVSE(PinName PWM_PIN, PinName Feedback_PIN, PinName Relay_PIN, char set_ID)
{
    FBpin = new AnalogIn(Feedback_PIN);
    INTpin = new InterruptIn(PWM_PIN);
    OUTpin = new PwmOut(PWM_PIN);
    RLYpin = new DigitalOut(Relay_PIN);

    StopCharging();

    CheckPilot = new Ticker();
    // Set PWM
    OUTpin->period_ms(1);          
    setNewStatus(cNC);
    notify_status = cNC;

    ID = set_ID;
    
    // Set check
    INTpin->rise(this, &EVSE::pintH);
    INTpin->fall(this, &EVSE::pintL);
    CheckPilot->attach(this, &EVSE::CheckPilotIdle, 0.5);

    commanded_current = 0;
}
