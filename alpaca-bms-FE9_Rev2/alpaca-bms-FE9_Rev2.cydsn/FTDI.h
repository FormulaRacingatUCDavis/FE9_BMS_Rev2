/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#ifndef FTDI_H
#define FTDI_H

#include "project.h"
#include "data.h"
#include "cell_interface.h"

#define ESCAPE_CHAR 0xAA
#define PACK_FRAME_START 0xBB
#define FRAME_END 0x0A

extern BAT_SUBPACK_t bat_subpack[N_OF_SUBPACK];
extern BAT_PACK_t bat_pack;

void send_byte_with_escape(uint8_t byte);
void send_uart_data();
void send_soc_data();
void update_soc();

#endif
/* [] END OF FILE */
