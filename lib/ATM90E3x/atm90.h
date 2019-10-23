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

#ifndef MBED_ATM90_H
#define MBED_ATM90_H
 
#include "mbed.h"

class ATM90 {
public:
//SPI spi(p5, p6, p7); // mosi, miso, sclk
    ATM90(PinName mosi, PinName miso, PinName sclk, PinName cs);
    ~ATM90();

    uint16_t ReadSPI(uint16_t addr);
    uint16_t WriteSPI(uint16_t addr, uint16_t val);
    
    uint16_t getTotPower();
    uint16_t getMaxCurrent();
    uint16_t getAVGvolt();

    void load_config(uint32_t addr);
    void save_config(uint32_t addr);
    
private:  
    SPI         *device;
    DigitalOut  *pcs;
};

#endif