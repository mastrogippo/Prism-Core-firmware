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

#include "ctcomm.h"


CTCOMM::~CTCOMM()
{
    delete pc;
}

CTCOMM::CTCOMM(PinName ser_tx, PinName ser_rx, int ser_baud)
{
    pc = new Serial(ser_tx, ser_rx, ser_baud);
    //This is deprecated but works:
    pc->attach(this, &CTCOMM::serial_interrupt);
    //This should be the correct form, but doesn't work:
    //pc->attach(callback(this, &CTCOMM::serial_interrupt));
    //see https://stackoverflow.com/questions/53471303/c-pass-reference-to-function-inside-same-class
}

void CTCOMM::serial_interrupt() {
    char c;
    static unsigned char buff[RX_BUF_LENGTH+2];
    static unsigned char cont = 0;
    static unsigned char escape = 0;

    while(pc->readable())
    {
        c = pc->getc();

        //TEST
        //    pc->putc(c);
        //    return;
        //END TEST

        if(c == _xt_start)
        {
            buff[0] = c;
            cont = 1;
            escape = 0;
        }
        else if(cont > 0)
        {
            if((cont+2) >= RX_BUF_LENGTH)
            { //overflow
                cont = 0;
                return;
            }

            //7D 01 02 03 04 7F 22 69 CS 7E
            //I received an escape, let's parse it
            if(escape == 1)
            {
                if(c == _xt_start_esc) c = _xt_start;
                else if(c == _xt_stop_esc) c = _xt_stop;
                else if(c == _xt_escape_esc) c = _xt_escape;
                else
                {
                    //SHOULD NEVER GET HERE! RESET
                    cont = 0;
                    return;
                }
                escape = 0;
                buff[cont] = c;
                cont++;
            }
            else if(c == _xt_escape)
            {
                escape = 1;
            }
            else if(c == _xt_stop)
            {
                //check checksum
                char crc = _xt_start;
                for(int i = 1; i < cont; i++)
                    crc ^= buff[i];

                if(crc == 0) //correct!
                    parse_packet(buff, cont);

                cont = 0;
            }
            else
            {
                buff[cont] = c;
                cont++;
            }
        }
    }
}

void CTCOMM::dbg(char * cmd)
{
  // Calculate CRC
  char crc = _xt_start;
  pc->putc(_xt_start);
  pc->putc(0xF0);
  crc ^= 0xF0;
  int i = 0;
  
  char ch = *cmd++;

  while((ch != 0) && (i < (TX_BUF_LENGTH-5)))
  {
    crc ^= ch;
    send_with_escape(ch);
    ch = *cmd++;
  }

  send_with_escape(crc);
  pc->putc(_xt_stop);
}

void CTCOMM::attach(void(*fn)(unsigned char * cmd, int len))
{
    callback = fn;
}

void CTCOMM::parse_packet(unsigned char * cmd, int len)
{
    callback(cmd, len);
}

void CTCOMM::send_with_escape(unsigned char c)
{
    if(c == _xt_start)
    {
        pc->putc(_xt_escape);
        pc->putc(_xt_start_esc);
    }
    else if(c == _xt_stop)
    {
        pc->putc(_xt_escape);
        pc->putc(_xt_stop_esc);
    }
    else if(c == _xt_escape)
    {
        pc->putc(_xt_escape);
        pc->putc(_xt_escape_esc);
    }
    else
    {
        pc->putc(c);
    }
}

/*****************
* Sends a command
******************/
//void XeThruRadar::send_command(const unsigned char * cmd, int len) {
void CTCOMM::send_command(unsigned char * cmd, int len) {

  if((len+5) >= TX_BUF_LENGTH) return; //too long!

  // Calculate CRC
  char crc = _xt_start;
  pc->putc(_xt_start);

  for (int i = 0; i < len; i++)
  {
    crc ^= cmd[i];
    send_with_escape(cmd[i]);
  }

  send_with_escape(crc);
  pc->putc(_xt_stop);
}
