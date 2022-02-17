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
}

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */

    init();   //initialize modules
    
    LTC6811_initialize(MD_FILTERED);

    uint8_t cfga[6];
    int8_t success;
    
    LTC6811_wakeup();
    success = LTC6811_rdcfga(1, cfga);
    LTC6811_wrcfga(0xFF, 11, cfga);
    success = LTC6811_rdcfga(1, cfga);
    
    while(1)
    {
        BMS_OK_Write(0u);
        CyDelay(100); 
        BMS_OK_Write(1);
        CyDelay(100);
    }
}

/* [] END OF FILE */
