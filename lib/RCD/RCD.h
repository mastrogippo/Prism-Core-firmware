// RCD interface library for Prism Core
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

#ifndef MBED_RCD_H
#define MBED_RCD_H
 
#include "mbed.h"

class RCD {
public:
    int Status = 0;

    RCD(PinName pin_detect, PinName pin_test);
    ~RCD();
	
	bool test();
    void attach(void(*fn)(int newStatus));
	
private:  

	bool testing = false;
	
    DigitalOut  *ptest;
    InterruptIn *pdetect;
	
	void pintH();
	void pintL();
	
    void(*callback)(int newStatus);
    void cb(int NewStatus);
};

#endif