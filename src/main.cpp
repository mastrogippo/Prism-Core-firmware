// STM32F103 Prism Core firmware
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

#include "mbed.h"
#include "main.h"

ATM90 pwrIC(pin_mosi, pin_miso, pin_sclk, pin_cs);
CTCOMM com(pin_u_tx, pin_u_rx, 115200);

//EVSE evse0(pin_pilot_d, pin_pilot_r, pin_rly1, 0);
//EVSE evse1(pin_pilot_d, pin_pilot_r, 0);
EVSE* evse[num_ports];
RCD rcd(pin_rcd, pin_rcd_t);

//EVSE evse2(pin_pilot_d, pin_pilot_r);

//Serial pc(pin_u_tx, pin_u_rx,115200);

//TEST
//DigitalOut rly1(pin_rly1);
//DigitalOut led1(pin_rly2);
//DigitalOut pil(pin_pilot_d);
//DigitalOut led1(pin_rcd_t);


DigitalIn hv_pres2(pin_hv_pres2);
DigitalIn hv_pres1(pin_hv_pres1);

unsigned char tmp[TX_BUF_LENGTH];

//uint32_t TotActiveEnergyCount = 0;

Ticker do1s;
void everySecond()
{
    //TODO:TESTARE!!!!
    static uint32_t tmpTotActEnCnt = 0;  
    uint16_t enRead = pwrIC.ReadSPI(0x90); //Sum Active Energy
    
    tmpTotActEnCnt += enRead;
    while(tmpTotActEnCnt > 320) //320 impulse = 0.1kWh
    {
        addUnit();
        tmpTotActEnCnt -= 320;
    }
}

Ticker flipper;
void flip() {
   // led1 = !led1;
   // rly = !rly;
}

void EVSE_changed(char ID, EVSE::cst NewStatus)
{
    send_status(ID);
    /*if(NewStatus == EVSE::cCharge)
        rly1 = 1;
    else
        rly1 = 0;*/
}

char EVSE_Auth(char ID)
{
    //TODO: do something to authorize charge start
    DebugM("Auth OK");
    return 0x55;
}

void RCD_cb(int newStatus)
{
    //TODO: add multiple evse support
    if(newStatus == 1)
    {
        evse[0]->RCDstop();
        send_error(err_RCDtrip);
    }
    else if(newStatus == 0)
    {
        evse[0]->RCDreset();
    }
}

int main() 
{
    DebugM(fw_ver_string);
    do1s.attach(&everySecond, 1);
    flipper.attach(&flip, 1.0); // the address of the function to be attached (flip) and the interval (2 seconds)

    com.attach(&StoreCmd);
    cmd_send_info();
    DebugM("Hello");
    //Activate and Test RCD
    while(!rcd.test())
    {    
        DebugM("RCD test ERROR");
        send_error(err_RCDtest);
        wait(5);
    }
    DebugM("RCD test OK!");
    rcd.attach(&RCD_cb);
    
    evse[0] = new EVSE(pin_pilot_d, pin_pilot_r, pin_rly2, 0);
    evse[0]->attach(EVSE_changed);
    evse[0]->attachAuth(EVSE_Auth);
    //evse[1] = &evse1;
    
    DebugM("EE check");
    
    //TODO: Check EEPROM data validity
    if(false)//EEPROM_CHECK()
    {
        pwrIC.load_config(EEPROM_START_ADDRESS);
    }
    else
    {
        //TODO: Throw error
    }
    
    //wait(1);
    while(1) {
        ParseCmd();
        wait(0.001);
    }
}



void DebugM(const char * cmd)
{
    com.dbg(cmd);
}


static unsigned char cmd_buff[RX_BUF_LENGTH+2];
static volatile unsigned char cmd_len = 0;

uint16_t bytes_to_u16(unsigned char * b)
{
    uint16_t tmp = b[0];
    return (tmp << 8) + b[1];
}
void U16_to_bytes(uint16_t u16, unsigned char * dest)
{
    dest[0] = (unsigned char)(u16 >> 8);
    dest[1] = (unsigned char)(u16);
}

uint32_t bytes_to_u32(unsigned char * b)
{
    uint32_t tmp = b[0];
    tmp += (tmp << 8) + (uint32_t)b[1];
    tmp += (tmp << 8) + (uint32_t)b[2];
    tmp += (tmp << 8) + (uint32_t)b[3];
    return tmp;
}
void U32_to_bytes(uint32_t u32, unsigned char * dest)
{
    dest[0] = (unsigned char)(u32 >> 24);
    dest[1] = (unsigned char)(u32 >> 16);
    dest[2] = (unsigned char)(u32 >> 8);
    dest[3] = (unsigned char)(u32);
}

void StoreCmd(unsigned char * cmd, int len) 
{
    if(cmd_len == 0) //se ho un comando in coda, ignoro e droppo. implementare coda?
    {
        for(int i = 0; i < len; i++)
        {
            cmd_buff[i] = cmd[i + 1];
        }
        cmd_len = len - 1;
    }
}

void ParseCmd() 
{
    //parse only if we have received something
    if(cmd_len == 0)
        return;

    uint16_t t = 0;
    
    switch(cmd_buff[0])
    {
        case c_rst:
            if(cmd_buff[1] == 0x55)
            {
                pwrIC.WriteSPI(0x0000, 0x789A);
                //NVIC_SystemReset breaks communication
                //TODO: find out why
                //NVIC_SystemReset();
            }
        break;
        case c_ping:
            tmp[0] = c_pong;
            tmp[1] = ~cmd_buff[1];
            com.send_command(tmp, 2);
        break;
        case c_rstatus:
            if(cmd_buff[1] >= num_ports)
            {
                send_error(err_portUnknown);
            }
            else
            {
                send_status(cmd_buff[1]);
            }
        break;
        case c_set_current:
            //TODO: implement
            if(cmd_buff[1] > num_ports)
            {
                send_error(err_portUnknown);
            }
            else
            {
                evse[cmd_buff[1]]->set_commanded_current((int16_t)(bytes_to_u16(&cmd_buff[2])));
                send_c_current(cmd_buff[1]);
            }
        break;
        case c_atm_read_reg:
            t = cmd_buff[1];
            t = (t << 8) + cmd_buff[2];
            cmd_read_reg(t);
        break;
        case c_atm_read_all:
        {
            uint16_t reg = 0;
            for(int i = 0; i < 32; i++)
            {
                tmp[0] = c_atm_reg_multi;
                U16_to_bytes(reg, &tmp[1]);
                tmp[3] = 8;
                for(int j = 0; j < 8; j++)
                {
                    t = pwrIC.ReadSPI(reg);
                    U16_to_bytes(t, &tmp[(j*2) + 4]);
                    reg++;
                }
                com.send_command(tmp, 20);
            }
        }
        break;
        case c_atm_write_reg:
        {
            uint16_t reg = bytes_to_u16(&cmd_buff[1]);
            uint16_t val = bytes_to_u16(&cmd_buff[3]);
            pwrIC.WriteSPI(reg, val);
            cmd_read_reg(reg);
        }
        break;
        case c_get_info:
            cmd_send_info();
        break;

        case c_test:
            do_test();
        break;

        case c_get_port_info:
            if(cmd_buff[1] >= num_ports)
            {
                send_error(err_portUnknown);
            }
            else
            {
                get_port_info(cmd_buff[1]);
            }
        break;
        case c_set_port_info:
            if(cmd_buff[1] >= num_ports)
            {
                send_error(err_portUnknown);
            }
            else
            {
                set_port_info(cmd_buff[1]);
            }
        break;        

        default:
            send_error(err_unknownCMD);
        break;
    }
    cmd_len = 0;
    //com.send_command(cmd, len);
}

void do_test()
{
    if(cmd_buff[1] != 0x55)
        return;

    tmp[0] = c_test + 0x80;
    tmp[1] = cmd_buff[1];
    tmp[2] = cmd_buff[2];
    tmp[3] = cmd_buff[3];
    com.send_command(tmp, 4);
    
    switch(cmd_buff[2])
    {
        case 0x01:
            DebugM("[disabled]Rele ON");
            //rly1 = 1;
        break;
        case 0x00:
            DebugM("[disabled]Rele OFF");
            //rly1 = 0;
        break;
        case 0x42:
            if(rcd.test())
                DebugM("RCD test OK!");
            else
                DebugM("RCD test ERROR");            
        break;
        case 0x66:
            pwrIC.load_config(EEPROM_START_ADDRESS);
        break;
        case 0x77:
            DebugM("Call");
            pwrIC.save_config(EEPROM_START_ADDRESS);
        break;
    }
}

void send_error(unsigned char err_code)
{
    tmp[0] = c_error;
    tmp[1] = err_code;
    com.send_command(tmp, 2);
}

void cmd_read_reg(uint16_t addr)
{
    uint16_t val = pwrIC.ReadSPI(addr);
    tmp[0] = c_atm_reg;
    U16_to_bytes(addr, &tmp[1]);
    U16_to_bytes(val, &tmp[3]);
    com.send_command(tmp, 5);
}
void cmd_send_info()
{
    //TODO: implement
    //byte1: versione major,byte2: versione minor, byte3-4: fw checksum,  byte5-16: CPUID, byte17: ports#
    int len = 0;
    tmp[len++] = c_info;
    tmp[len++] = fw_ver_major;
    tmp[len++] = fw_ver_minor;
    
    //fw checksum
    tmp[len++] = 0x55;
    tmp[len++] = 0xAA;

    //CPUID
    for(int i = 0; i < 12; i++)
    {
        tmp[len++] = idBase0[i]; 
    }

    //num ports
    tmp[len++] = 0x01;
    com.send_command(tmp, len);
}

void send_status(char ID)
{
    //TODO: implement
    int len = 0;
    //byte1: ID lato, byte2: flags, byte3-4:corrente richiesta/10[SIG], 
    //byte 5-6 corrente attuale/10[SIG], byte 7-8 tensione attuale/10[SIG], byte9-10 Potenza attuale/10[SIG]
    tmp[len++] = c_status;
    tmp[len++] = ID;

    switch(evse[ID]->getStatus())
    {
        case EVSE::cNC:
            tmp[len] = cs_idle;
        break;
        case EVSE::cERR:
        case EVSE::cERRshort:
        case EVSE::cERRdiode:
        case EVSE::cERR1:
        case EVSE::cERR2:
        case EVSE::cERR3:
            tmp[len] = cs_error;
        break;
        case EVSE::cReady:
            tmp[len] = cs_waiting + cs_car_connected;
        break;
        case EVSE::cCharge:
        case EVSE::cVent:
            tmp[len] = cs_charging + cs_car_connected;
        break;   
    }
    len++;

    U16_to_bytes((uint16_t)(evse[ID]->commanded_current), &tmp[len]);
    len += 2;

    U16_to_bytes((uint16_t)(pwrIC.getMaxCurrent()/100), &tmp[len]);
    len += 2;

    U16_to_bytes((uint16_t)(pwrIC.getAVGvolt()/10), &tmp[len]);
    len += 2;

    //TODO: find this value in ATM90 when it's calibrated 
    tmp[len++] = 0x01; //(byte)(evse[ID].read_power>>8);
    tmp[len++] = 0x67; //3590W (3593.5W) //(byte)(evse[ID].read_power);

    //Total energy
    //TODO: Separate counters for different interfaces
    uint32_t TotEnergy = ReadEnergy();
    U32_to_bytes((uint32_t)(TotEnergy), &tmp[len]);
    len += 4;
    U32_to_bytes((uint32_t)(TotEnergy - evse[ID]->SessionStartEnergy), &tmp[len]);
    len += 4;


    com.send_command(tmp, len);
}

void send_c_current(char ID)
{
    tmp[0] = c_current;
    tmp[1] = ID;
    U16_to_bytes((uint16_t)(evse[cmd_buff[1]]->commanded_current), &tmp[2]);

    com.send_command(tmp, 4);
}

void get_port_info(char ID)
{
    //byte1: ID lato, byte2: flags, byte3-4: corrente massima
    tmp[0] = c_port_info;
    tmp[1] = ID;
    tmp[2] = 0x55;
    U16_to_bytes((uint16_t)(evse[cmd_buff[1]]->GetCurrentLimit()), &tmp[3]);

    com.send_command(tmp, 5);
}

void set_port_info(char ID)
{
    //TODO: This sets only one side current; find a way to
    //set the current limit of the whole system 
    uint16_t Pass = bytes_to_u16(&cmd_buff[9]);
    if(Pass != 0x55AA)
    {
        send_error(err_wrongPass);
        return;
    }

    uint8_t flags = bytes_to_u16(&cmd_buff[2]);
    uint16_t setC = bytes_to_u16(&cmd_buff[3]);
    evse[cmd_buff[1]]->SetCurrentLimit(setC);

    get_port_info(ID);
}






















/*
unsigned char smg[50];
int count = 0;
int num;
char tc;
bool flagser_getend;

void Wr(uint8_t * buf, int len)
{
    for(int i = 0; i < len; i++)
    {
        while(!pc.writable());
        pc.putc(buf[i]);
        wait(0.001);
    }
}

void ParseAnswer()
{
    if(smg[1] == 'R')
    {

        while(!pc.writable());
        pc.putc(0x69);
        while(!pc.writable());
        pc.putc(0x69);
        while(!pc.writable());
        pc.putc(smg[2]);
        while(!pc.writable());
        pc.putc(smg[3]);
        
        uint16_t tmp;
        for(uint16_t i = smg[2]; i <= smg[3]; i++)
        {
            tmp = pwrIC.ReadSPI(i);
            while(!pc.writable());
            pc.putc((uint8_t)(tmp));
            while(!pc.writable());
            pc.putc((uint8_t)(tmp >> 8));
        }
    }
    else if(smg[1] == 'W')
    {
        uint16_t Addr = smg[2];
        Addr = Addr<<8;
        Addr += smg[3];
        uint16_t Val = smg[4];
        Val = Val<<8;
        Val += smg[5];
        pwrIC.WriteSPI(Addr, Val);
    }
    else if(smg[1] == 'S')
    {
        if(smg[2] == 1)
        {
            if(smg[3] == 0)
                rly1 = 1;
            else
                rly1 = 0;
        }
        else if(smg[2] == 2)
        {
            if(smg[3] == 0)
                rly2 = 1;
            else
                rly2 = 0;
        }
    }
}

void SerInt() {
    unsigned char temp;
    static unsigned char max;
    while(pc.readable())
    {
        temp = pc.getc();
        if(count == 0)
        {
            if(temp == 0x69) //start
                smg[count++] = temp;
        }
        else if(count == 1)
        {
            smg[count++] = temp;
            if(temp == 'R')
                max = 4;
            else if(temp == 'W')
                max = 6;
            else if(temp == 'S')
                max = 4;
        }        
        else// if(count <= 3) //get address
        { 
            smg[count++] = temp;
            if(count >= max)
            {
                ParseAnswer();
                count = 0;
            }            
        }
    }
}

void SerialStart(void) {
    pc.baud(115200);
    //pc.baud(921600);
    flagser_getend = 0;
    pc.attach(&SerInt,Serial::RxIrq);
    count = 0;
}
 
int main() {
        rly1 = 1;
        rly2 = 1;
    pc.printf("Start\n");
    SerialStart();
    pc.printf("1\n");

    while(1) {
        led1 = 1;
        wait(0.2);
        led1 = 0;
        wait(0.2);
    }
}
/*
int main()
{
    
    
    while (1) {
        printf("%d \n", evse1.getStatus());
        //if(chg_status == cERR)
           wait(0.5);

           wait(2);
           rly1 = 1;
           wait(2);
           rly2 = 1;
           wait(2);
           rly1 = 0;
           wait(2);
           rly2 = 0;
           wait(2);
           
        
    }
}
*/