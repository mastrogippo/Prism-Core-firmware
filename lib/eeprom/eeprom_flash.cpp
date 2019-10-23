// EEPROM mapping library for Prism Core
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
#include "eeprom_flash.h"
#include "utils.h"

//EEPROM structure: (NOTE: all address are 16bit wide! to get actual address, double it.)
//126-127 accumulator for energy counter
//128:
//0x00-0x2E  generic config
//0x30-0x62  ATM90 config

void _FLASH_PageErase(uint32_t PageAddress)
{
    /* Proceed to erase the page */
    SET_BIT(FLASH->CR, FLASH_CR_PER);
    while (FLASH->SR & FLASH_SR_BSY);
    WRITE_REG(FLASH->AR, PageAddress);
    SET_BIT(FLASH->CR, FLASH_CR_STRT);
    while (FLASH->SR & FLASH_SR_BSY);
    CLEAR_BIT(FLASH->CR, FLASH_CR_PER);
}

/*
 * Must call this first to enable writing
 */
void enableEEPROMWriting(uint32_t addrEE) {
    HAL_FLASH_Unlock();
    _FLASH_PageErase(addrEE);
}

void disableEEPROMWriting() {
    HAL_FLASH_Lock();
}

/*
 * Writing functions
 * Must call enableEEPROMWriting() first
 */
HAL_StatusTypeDef writeEEPROMHalfWord(uint32_t address, uint16_t data) {
    HAL_StatusTypeDef status;
    address = address + EEPROM_START_ADDRESS;
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address, data);
    return status;
}

HAL_StatusTypeDef writeEEPROMWord(uint32_t address, uint32_t data) {
    HAL_StatusTypeDef status;
    address = address + EEPROM_START_ADDRESS;
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data);
    return status;
}

/*
 * Reading functions
 */
uint16_t readEEPROMHalfWord(uint32_t address) {
    uint16_t val = 0;
    address = address + EEPROM_START_ADDRESS;
    val = *(__IO uint16_t*)address;
    return val;
}

uint32_t readEEPROMWord(uint32_t address) {
    uint32_t val = 0;
    address = address + EEPROM_START_ADDRESS;
    val = *(__IO uint32_t*)address;
    return val;
}


bool EEPROM_CHECK(uint32_t addr)
{
    //only check first byte 
    //TODO: implement later maybe
    uint16_t tmp = *(__IO uint16_t*)(addr+0x60);
    //first byte should be 0x5678
    if(tmp != 0x5678)
        return false;
    else
        return true;    
}


uint32_t ReadEnergy()
{
    uint32_t i;
    uint16_t r;
    uint32_t pageAddr;
    //read last number of first page
    r = *(__IO uint16_t*)(ENERGY_START_ADDRESS+0x400-2);

    if(r == 0xFFFF)
    {//use first page
        pageAddr = ENERGY_START_ADDRESS;
    }
    else
    {
        //read last number of second page
        r = *(__IO uint16_t*)(ENERGY_START_ADDRESS+0x800-2);
        if(r == 0xFFFF)
        {//use second page
            pageAddr = ENERGY_START_ADDRESS+0x400;
        }
        else
        {//should NEVER go here
            //TODO: handle
            return 0;
        }
    }
    //read Base
    i = *(__IO uint32_t*)(pageAddr);

    for(int j = 4; j < 0x400; j += 2)
    {
        r = *(__IO uint16_t*)(pageAddr+j);
        if(r == 0xFFFF) //found it
        {
            return i;
        }
        else if(r != 0xAA55) //error!!
        {
            //TODO: should never get here, manage
        }
        i++;
    }

    //TODO: should never get here, manage
    return 0;
}

HAL_StatusTypeDef addUnit()
{
    uint32_t i;
    uint16_t r;
    HAL_StatusTypeDef status;

    HAL_FLASH_Unlock();

    i = *(__IO uint32_t*)(ENERGY_START_ADDRESS);      
    if(i == 0xFFFFFFFF) //FIRST BOOT!
    { //Init all
        _FLASH_PageErase(ENERGY_START_ADDRESS);
        _FLASH_PageErase(ENERGY_START_ADDRESS + 0x400);

        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, ENERGY_START_ADDRESS, (uint32_t)(0));
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, ENERGY_START_ADDRESS + 0x400, (uint32_t)(0));
        //TODO: check status
    }
    
    //find first free slot
    //check first bank
    for(i = 4; i < 0x400; i += 2)
    {
        r = *(__IO uint16_t*)(ENERGY_START_ADDRESS+i);
        if(r == 0xFFFF) //found it
        {
            status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, ENERGY_START_ADDRESS+i, 0xAA55);
            if(i >= 0x3FE) //END OF PAGE!!
            {
                //read actual total energy
                uint32_t totEnergy = *(__IO uint32_t*)(ENERGY_START_ADDRESS);
                //sum all counts
                totEnergy += 0x1FE;
                //clear next page and write new value
                _FLASH_PageErase(ENERGY_START_ADDRESS + 0x400);
                status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, ENERGY_START_ADDRESS + 0x400, totEnergy);
                //TODO: check status
            }
            HAL_FLASH_Lock();
            return status;
        }
        else if(r != 0xAA55) //error!!
        {
            //manage
        }
    }

    //check second bank
    for(i = 4; i < 0x400; i += 2)
    {
        r = *(__IO uint16_t*)(ENERGY_START_ADDRESS+i+0x400); 
        if(r == 0xFFFF) //found it
        {
            status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, ENERGY_START_ADDRESS+i+0x400, 0xAA55);
            if(i >= 0x3FE) //END OF PAGE!!
            {
                //read actual total energy
                uint32_t totEnergy = *(__IO uint32_t*)(ENERGY_START_ADDRESS+0x400);
                //sum all counts
                totEnergy += 0x1FE;
                //clear next page and write new value
                _FLASH_PageErase(ENERGY_START_ADDRESS);
                status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, ENERGY_START_ADDRESS, totEnergy);
                //TODO: check status
            }
            HAL_FLASH_Lock();
            return status;
        }
        else if(r != 0xAA55) //error!!
        {
            //manage
        }
    }

    HAL_FLASH_Lock();
}

//overwrite config with data byte, from start
void EE_save_config(uint16_t * data, uint16_t len, uint16_t start)
{
    //TODO: test and implement
    uint32_t addr = EEPROM_START_ADDRESS;
    if((start+len) > (EEPROM_SIZE/2))
    {
       DebugM("Error: EE overflow"); 
       return;
    }
    DebugM("StartSave");

    //temporary save page (0x400 byte)
    uint16_t flash_data[0x200]; 
    uint16_t i;
    for(i = 0; i < 0x200; i++ )
    {
        flash_data[i] = *(__IO uint16_t*)(addr+(i*2));
    }

    //overwrite bytes
    for(i = 0; i < len; i++ )
    {
        flash_data[i+start] = data[i];
    }

    DebugM("Write");
    __disable_irq();

    enableEEPROMWriting(addr);
    for(i = 0; i < 0x200; i++ )
    { 
        HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr+(i*2),  flash_data[i]);
        //TODO: check result
        //writeEEPROMHalfWord(i*2, flash_data[i]);
    }
    disableEEPROMWriting();

    __enable_irq();
    DebugM("EndSave");
}