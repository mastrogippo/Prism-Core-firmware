// Communication protocol library for Prism Core
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

#ifndef MBED_CTCOMM_H
#define MBED_CTCOMM_H
 
#include "mbed.h"
 
#define _xt_start   0x7D
#define _xt_stop    0x7E
#define _xt_escape  0x7F

#define _xt_start_esc   0x22
#define _xt_stop_esc    0x33
#define _xt_escape_esc  0x44

#define RX_BUF_LENGTH 40
#define TX_BUF_LENGTH 40

 class CTCOMM {
public:
//SPI spi(p5, p6, p7); // mosi, miso, sclk
    CTCOMM(PinName ser_tx, PinName ser_rx, int ser_baud);
    ~CTCOMM();
    void attach(void(*fn)(unsigned char * cmd, int));
    
    void send_command(unsigned char * cmd, int len);
    //void send_test(char c); 
    
    void dbg(char * cmd);

private:  
    Serial *pc;
    void(*callback)(unsigned char * cmd, int);

    void serial_interrupt();

    void parse_packet(unsigned char * cmd, int len);
    void send_with_escape(unsigned char c);

    void get_command(unsigned char c);
};

#endif