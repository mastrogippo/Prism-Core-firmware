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

#ifndef __EEPROM_FLASH_H
#define __EEPROM_FLASH_H

#include "mbed.h"


//#define FLASH_BASE   ((uint32_t)0x08000000) //defined in stm32f10x.h
#define FLASH_SIZE   ((uint32_t)0x20000) //128k
#define EEPROM_SIZE   ((uint32_t)0x400)  //1k

//#define EEPROM_START_ADDRESS   ((uint32_t)0x08019000) // EEPROM emulation start address: after 100KByte of used Flash memory
//#define EEPROM_START_ADDRESS   ((uint32_t)0x0801C000) // EEPROM emulation start address: after 112KByte of used Flash memory
#define EEPROM_START_ADDRESS   ((uint32_t)(FLASH_BASE+FLASH_SIZE-EEPROM_SIZE)) // EEPROM emulation start address: last KByte of Flash memory
#define ENERGY_START_ADDRESS (uint32_t)(EEPROM_START_ADDRESS - 0x800) //2 banchi, 2K

void enableEEPROMWriting(uint32_t addrEE); // Unlock and keep PER cleared
void disableEEPROMWriting(); // Lock

// Write functions
HAL_StatusTypeDef writeEEPROMHalfWord(uint32_t address, uint16_t data);
HAL_StatusTypeDef writeEEPROMWord(uint32_t address, uint32_t data);

// Read functions
uint16_t readEEPROMHalfWord(uint32_t address);
uint32_t readEEPROMWord(uint32_t address);

bool EEPROM_CHECK(uint32_t addr);

//Energy accumulator
HAL_StatusTypeDef addUnit();
uint32_t ReadEnergy();

void EE_save_config(uint16_t * data, uint16_t len, uint16_t start);

#endif
