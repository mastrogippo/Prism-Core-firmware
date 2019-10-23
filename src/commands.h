// STM32F103 Prism Core firmware
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

#define c_get_info      0x01
#define c_info          0x81
#define c_rst           0x05
#define c_rstatus       0x07
#define c_status        0x87
#define c_set_current	0x10
#define c_current	    0x90
#define c_ping	        0x06
#define c_pong	        0x86
#define c_atm_read_reg  0x30
#define c_atm_reg       0xB0
#define c_atm_read_all  0x32
#define c_atm_reg_multi 0xB2
#define c_atm_write_reg 0x35
#define c_test          0x55
#define c_error	        0xF1

#define c_get_port_info	0x02
#define c_set_port_info	0x42
#define c_port_info	    0x82

//errors
#define err_unknownCMD  0x01
#define err_portUnknown 0x02
#define err_outOfRange  0x03
#define err_wrongPass   0x04
#define err_EEPROM      0x10 
#define err_RCDtest     0x50
#define err_RCDtrip     0x51


//charger status
//lower 3 bits
#define cs_idle 0x00
#define cs_waiting 0x01
#define cs_charging 0x02
#define cs_stopped 0x03

#define cs_3phase 0x08
#define cs_car_connected 0x10
#define cs_error 0x80

bool message_to_parse;

//Helpers
uint16_t bytes_to_u16(unsigned char * b);
void U16_to_bytes(uint16_t u16, unsigned char * dest);
//
void StoreCmd(unsigned char * cmd, int len);
void ParseCmd();
void send_error(unsigned char err_code);
void cmd_read_reg(uint16_t addr);

