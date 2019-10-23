// Utilities for Prism Core
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

#ifndef MBED_UTILS_H
#define MBED_UTILS_H
 
#include "mbed.h"


extern void DebugM(char * cmd);
uint16_t bytes_to_u16(uint8_t * b);
void U16_to_bytes(uint16_t u16, uint8_t * dest);
uint32_t bytes_to_u32(uint8_t * b);
void U32_to_bytes(uint32_t u32, uint8_t * dest);
void BtoHexStr(uint32_t u32, uint8_t size, char * dest);
void DbgHex(char * prefix, uint32_t u32, uint8_t size);


#endif