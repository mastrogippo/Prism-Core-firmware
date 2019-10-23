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


#include "utils.h"


uint16_t bytes_to_u16(uint8_t * b)
{
    uint16_t tmp = b[0];
    return (tmp << 8) + b[1];
}
void U16_to_bytes(uint16_t u16, uint8_t * dest)
{
    dest[0] = (unsigned char)(u16 >> 8);
    dest[1] = (unsigned char)(u16);
}

uint32_t bytes_to_u32(uint8_t * b)
{
    uint32_t tmp = b[0];
    tmp += (tmp << 8) + (uint32_t)b[1];
    tmp += (tmp << 8) + (uint32_t)b[2];
    tmp += (tmp << 8) + (uint32_t)b[3];
    return tmp;
}
void U32_to_bytes(uint32_t u32, uint8_t * dest)
{
    dest[0] = (unsigned char)(u32 >> 24);
    dest[1] = (unsigned char)(u32 >> 16);
    dest[2] = (unsigned char)(u32 >> 8);
    dest[3] = (unsigned char)(u32);
}

void BtoHexStr(uint32_t u32, uint8_t size, char * dest)
{
    uint8_t tmp;
    for(uint8_t i = ((size*2)+1); i > 1; i--)
    {
        tmp = (uint8_t)(u32 & 0x0F);
        if(tmp < 0x0A)
            dest[i-2] = tmp + '0';
        else
            dest[i-2] = (tmp-0x0A) + 'A';
        
        u32 = u32 >> 4;
    }
}

void DbgHex(char * prefix, uint32_t u32, uint8_t size)
{
    char tmp[20];
    int i = 0;
    while((i < 11) && (prefix[i] != 0))
    {
        tmp[i] = prefix[i];
        i++;
    }
    BtoHexStr(u32,size,tmp+i);
    tmp[i+(size*2)+1] = 0;
    DebugM(tmp);
}