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
#include <LTC6811.h>

void init(void){   //initialize modules
    SPI_Start();  
    //USB_Start(0, USB_5V_OPERATION);
    //USB_CDC_Init();
    //PCAN_Start();
}

int main(void)
{  //SEE ADOW ON DATASHEET PAGE 33
    CyGlobalIntEnable; /* Enable global interrupts. */

    init();   //initialize modules
    
    LTC6811_initialize(MD_FAST);
    LTC6811_init_cfg();

    uint8_t cfga[6];
    uint16_t aux;
    volatile uint16_t volts[16];
    volatile uint16_t i = 0; 
    
    const char8 str[] = "Bitch"; 
    
    uint16_t cell_voltages[2][12];

    
}

/* [] END OF FILE */
