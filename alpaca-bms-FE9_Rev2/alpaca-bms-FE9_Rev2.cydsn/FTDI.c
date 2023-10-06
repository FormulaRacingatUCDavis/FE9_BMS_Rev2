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
    uint8_t i = 0;
    uint8_t j = 0;
    
    for(i = 0; i < N_OF_SUBPACK; i++){
        FTDI_UART_PutChar(ESCAPE_CHAR);
        FTDI_UART_PutChar(i);
        
        for(j = 0; j < CELLS_PER_SUBPACK; j++){
            uint16_t v = bat_subpack[i].cells[j]->voltage;
            send_byte_with_escape(HI8(v));
            send_byte_with_escape(LO8(v));
        }
        for(j = 0; j < CELL_TEMPS_PER_PACK; j++){
            uint8_t t = (uint8_t)(bat_subpack[i].cell_temps[j]->temp_c);
            send_byte_with_escape(t);
        }
        
        FTDI_UART_PutChar(ESCAPE_CHAR);
        FTDI_UART_PutChar(FRAME_END);
    }
    
    FTDI_UART_PutChar(ESCAPE_CHAR);
    FTDI_UART_PutChar(PACK_FRAME_START);
    
    uint16_t v = (uint16_t)bat_pack.voltage;
    send_byte_with_escape(HI8(v));
    send_byte_with_escape(LO8(v));
    
    v = bat_pack.LO_voltage;
    send_byte_with_escape(HI8(v));
    send_byte_with_escape(LO8(v));
    
    v = bat_pack.HI_voltage;
    send_byte_with_escape(HI8(v));
    send_byte_with_escape(LO8(v));
    
    uint8_t t = (uint8_t)bat_pack.HI_temp_c;
    send_byte_with_escape(t);
    
    t = (uint8_t)bat_pack.LO_temp_c;
    send_byte_with_escape(t);
    
    t = (uint8_t)bat_pack.AVG_temp_c;
    send_byte_with_escape(t);
    
    send_byte_with_escape(bat_pack.SOC_percent);
    
    v = (uint16_t)bat_pack.status;
    send_byte_with_escape(HI8(v));
    send_byte_with_escape(LO8(v));
       
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

//sends extra escape byte if byte is escape byte
void send_byte_with_escape_pic18(uint8_t byte){
    if(byte == 0x05){
        PIC18_UART_PutChar(0x05);
    }
    PIC18_UART_PutChar(byte);
}

void send_soc_data()
{
    PIC18_UART_PutChar(0x05);
    PIC18_UART_PutChar(0x0A);
    
    send_byte_with_escape(LO8(bat_pack.voltage));
    send_byte_with_escape(HI8(bat_pack.voltage));
    send_byte_with_escape(LO8(bat_pack.voltage));
    send_byte_with_escape(HI8(bat_pack.voltage));
    send_byte_with_escape(LO8(bat_pack.current));
    send_byte_with_escape(HI8(bat_pack.current));
    
    PIC18_UART_PutChar(0x05);
    PIC18_UART_PutChar(0x0B);
}

void update_soc()
{
    while(PIC18_UART_GetRxBufferSize() > 0)
    {
        bat_pack.SOC_percent = PIC18_UART_ReadRxData();
    }
}

/* [] END OF FILE */
