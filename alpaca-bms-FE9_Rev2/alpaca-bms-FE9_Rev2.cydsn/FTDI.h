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
#include "project.h"
#include "data.h"
#include "cell_interface.h"

#define ESCAPE_CHAR 0x05
#define FRAME_START 0x01
#define FRAME_END 0x0A

extern BAT_SUBPACK_t bat_subpack[N_OF_SUBPACK];

void send_byte_with_escape(uint8_t byte);
void send_uart_data();


/* [] END OF FILE */
