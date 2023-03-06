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
#include "FTDI.h"

void send_uart_data(){
    FTDI_UART_PutChar(ESCAPE_CHAR);
    FTDI_UART_PutChar(FRAME_START);
    
    uint8_t i = 0;
    uint8_t j = 0;
    
    for(i = 0; i < N_OF_SUBPACK; i++){
        for(j = 0; j < CELLS_PER_SUBPACK; j++){
            uint16_t v = bat_subpack[i].cells[j]->voltage;
            send_byte_with_escape(HI8(v));
            send_byte_with_escape(LO8(v));
        }
        for(j = 0; j < CELL_TEMPS_PER_PACK; j++){
            uint8_t t = (uint8_t)(bat_subpack[i].cell_temps[j]->temp_c);
            send_byte_with_escape(t);
        }
    }
    
    FTDI_UART_PutChar(ESCAPE_CHAR);
    FTDI_UART_PutChar(FRAME_END);
}

//sends extra escape byte if byte is escape byte
void send_byte_with_escape(uint8_t byte){
    if(byte == ESCAPE_CHAR){
        FTDI_UART_PutChar(ESCAPE_CHAR);
    }
    FTDI_UART_PutChar(byte);
}

/* [] END OF FILE */
