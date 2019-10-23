// ATM90E36 library for Prism Core
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

#include "atm90.h"
#include "eeprom_flash.h" 

extern void DebugM(char * cmd);

ATM90::~ATM90()
{
    delete device;
    delete pcs;
}

ATM90::ATM90(PinName mosi, PinName miso, PinName sclk, PinName cs)
{
    device = new SPI(mosi, miso, sclk);
    pcs = new DigitalOut(cs);
    
    pcs->write(1);
    
    // Setup the spi for 8 bit data, high steady state clock,
    // second edge capture, with a 1MHz clock rate
    device->format(16,0);
    device->frequency(500000);
}

uint16_t ATM90::ReadSPI(uint16_t addr)
{   
    // Select the device by seting chip select low
    pcs->write(0);

    device->write(0x8000+addr);

    // Send a dummy byte to receive the contents of the register
    int res = device->write(0x0000);

    // Deselect the device
    pcs->write(1);

    return res;
}

uint16_t ATM90::WriteSPI(uint16_t addr, uint16_t val)
{
    // Select the device by seting chip select low
    pcs->write(0);

    device->write(addr);

    // Send a dummy byte to receive the contents of the register
    int res = device->write(val);

    // Deselect the device
    pcs->write(1);

    return res;
}

//Returns actual power in W
uint16_t ATM90::getTotPower()
{
    return ReadSPI(0xB0)*4;
}      

//Returns the biggest current value of the 3 phases, in mA
uint16_t ATM90::getMaxCurrent()
{
    uint16_t tmp = ReadSPI(0xDD);
    if(ReadSPI(0xDE) > tmp)
        tmp = ReadSPI(0xDE);
    if(ReadSPI(0xDF) > tmp)
        tmp = ReadSPI(0xDF);
    return tmp;
}      

//Returns the avg voltage value of the 3 phases, in V/10
uint16_t ATM90::getAVGvolt()
{
    uint32_t tmp = ReadSPI(0xD9);
    tmp += ReadSPI(0xDA);
    tmp += ReadSPI(0xDB);
    return (uint16_t)(tmp/3);
}     

void ATM90::save_config(uint32_t addr)
{
    uint16_t flash_data[0x40];
    uint16_t tmpAddr = 0;
    uint16_t i;
    flash_data[tmpAddr++] = 0x5678;
    for(i = 0x31; i <= 0x3B; i++ )
        flash_data[tmpAddr++] = ReadSPI(i);
    flash_data[tmpAddr++] = 0x5678;
    for(i = 0x41; i <= 0x4D; i++ )
        flash_data[tmpAddr++] = ReadSPI(i);
    flash_data[tmpAddr++] = 0x5678;
    for(i = 0x51; i <= 0x57; i++ )
        flash_data[tmpAddr++] = ReadSPI(i);
    flash_data[tmpAddr++] = 0x5678;
    for(i = 0x61; i <= 0x6F; i++ )
        flash_data[tmpAddr++] = ReadSPI(i);
    EE_save_config(flash_data, 0x40, 0x30);

    /*
    DebugM("StartSave");

    uint16_t flash_data[0x200]; //temporary save page (0x400 byte)
    uint16_t i;
    for(i = 0; i < 0x200; i++ )
    {
        flash_data[i] = *(__IO uint16_t*)(addr+(i*2));
    }
    
    uint16_t tmpAddr = 0x30;
    flash_data[tmpAddr++] = 0x5678;
    for(i = 0x31; i <= 0x3B; i++ )
        flash_data[tmpAddr++] = ReadSPI(i);
    flash_data[tmpAddr++] = 0x5678;
    for(i = 0x41; i <= 0x4D; i++ )
        flash_data[tmpAddr++] = ReadSPI(i);
    flash_data[tmpAddr++] = 0x5678;
    for(i = 0x51; i <= 0x57; i++ )
        flash_data[tmpAddr++] = ReadSPI(i);
    flash_data[tmpAddr++] = 0x5678;
    for(i = 0x61; i <= 0x6F; i++ )
        flash_data[tmpAddr++] = ReadSPI(i);

    DebugM("EnableWrite");
    enableEEPROMWriting(addr);

    DebugM("Write");
    for(i = 0; i < 0x200; i++ )
    { 
        HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr+(i*2),  flash_data[i]);
        //TODO: check result
        //writeEEPROMHalfWord(i*2, flash_data[i]);
    }
    
    disableEEPROMWriting();
    DebugM("EndSave");*/
}
 
void ATM90::load_config(uint32_t addr)
{
    int i;
    addr += 0x60;
    for(i = 0x30; i <= 0x3B; i++ )
    {
        WriteSPI(i, *(__IO uint16_t*)addr);
        addr += 2;
    }
    WriteSPI(0x30, 0x8765);

    for(i = 0x40; i <= 0x4D; i++ )
    {
        WriteSPI(i, *(__IO uint16_t*)addr);
        addr += 2;
    }
    WriteSPI(0x40, 0x8765);

    for(i = 0x50; i <= 0x57; i++ )
    {
        WriteSPI(i, *(__IO uint16_t*)addr);
        addr += 2;
    }
    WriteSPI(0x50, 0x8765);

    for(i = 0x60; i <= 0x6F; i++ )
    {
        WriteSPI(i, *(__IO uint16_t*)addr);
        addr += 2;
    }
    WriteSPI(0x60, 0x8765);

    //TODO: Check checksum error registers. 
    //Saving checksum in ROM should automatically ensure data integrity
}
