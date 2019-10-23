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

#ifndef MBED_EVSE_H
#define MBED_EVSE_H
 
#include "mbed.h"
 
 
//VERIFICARE
#define hVAL_12V 65000
#define mVAL_12V 60000
#define mVAL_9V 50000
#define mVAL_6V 40000
#define mVAL_3V 30000
#define mVAL_min 5000 //abbassare dopo aver messo OPAMP?

#define numAVG 10 //how many cycles before changing 
#define numAVGerr1 100 //how many cycles before changing 

class EVSE {
public:
    enum cst {cNULL, cERR, cNC, cReady, cCharge, cVent, cERRshort, cERRdiode, cERR1, cERR2, cERR3, cERRRCD, cERRRLY};
    const char* StatD[13] = {"cNULL", "cERR", "cNC", "cReady", "cCharge", "cVent", "cERRshort", "cERRdiode", "cERR1", "cERR2", "cERR3", "cERRRCD", "cERRRLY"} ;

    uint8_t ID;
    bool AutoStart = true;

    cst chg_status;
    
    EVSE(PinName PWM_PIN, PinName Feedback_PIN, PinName Relay_PIN, PinName Sense1_PIN, PinName Sense2_PIN, uint8_t set_ID);
    ~EVSE();
    cst getStatus();
    char* getStatusStr();

    int16_t commanded_current;
    void set_commanded_current(int16_t current_dA);

    uint16_t GetCurrentLimit();
    void SetCurrentLimit(uint16_t c);
   
    uint32_t SessionStartEnergy = 0;

    void attach(void(*fn)(uint8_t eID, cst NewStatus));
    void attachAuth(uint8_t(*fn)(uint8_t eID));

    void StartCharge();

    void RCDstop();
    void RCDreset();

private:  
    AnalogIn    *FBpin;  //ain(A0);
    InterruptIn *INTpin; //pp(PA_9);
    PwmOut      *OUTpin; //my_pwm(PA_9);
    DigitalOut  *RLYpin; 
    DigitalIn   *SNS1pin; 
    DigitalIn   *SNS2pin; 

    Ticker      *CheckPilot;

    cst notify_status;

    volatile uint16_t gh;
    volatile uint16_t gl;

    volatile uint16_t CurrentLimit;

    volatile float commanded_pwm;
    void LoadCurrentLimit();

    void StartCharging();
    void StopCharging();
    
    void ResetRelayErr();
    bool CheckOutVoltage();
    
    uint8_t(*callbackAuth)(uint8_t eID);
    bool cbAuth();

    void(*callback)(uint8_t eID, cst NewStatus);
    void cb(cst NewStatus);
    
    void setNewStatus(cst NewStatus);
    void ChangeStatus(cst NewStatus);
    void CheckPilotIdle();
    void pintH();
    void pintL();

    void set_commanded_current_pwm();
};
#endif