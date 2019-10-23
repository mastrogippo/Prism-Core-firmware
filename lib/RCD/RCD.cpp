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

#include "RCD.h"

extern void DebugM(char * cmd);

RCD::~RCD()
{
    delete ptest;
    delete pdetect;
}

bool RCD::test()
{
	uint8_t testb = 0x01;
	testing = true;
	if(Status == 0)
		testb += 0x02;

	ptest->write(0);
	//should go in interrupt; wait 500ms
	wait(0.5);
	
	if(Status == 1)
		testb += 0x04;
	
	ptest->write(1);
	//should go in interrupt; wait 500ms
	wait(0.5);

	if(Status == 0)
		testb += 0x08;
	
	testing = false;
	
	if(testb == 0x0F)
		return true;
	else
		return false;
}

void RCD::attach(void(*fn)(uint8_t newStatus))
{
    callback = fn;
}

void RCD::cb(uint8_t newStatus)
{
    if(callback == NULL) return;
    callback(newStatus);
}


void RCD::pintH()
{
	Status = 1;
	if(testing == false)
	{
		cb(Status);
	}
}

void RCD::pintL()
{
	Status = 0;
	if(testing == false)
	{
		cb(Status);
	}
}

RCD::RCD(PinName pin_detect, PinName pin_test)
{
    pdetect = new InterruptIn(pin_detect);
    ptest = new DigitalOut(pin_test);
    ptest->write(1); //idle
	
	pdetect->rise(this, &RCD::pintH);
    pdetect->fall(this, &RCD::pintL);
}
