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
{  //SEE ADOW ON DATASHEET PAGE 33
    CyGlobalIntEnable; /* Enable global interrupts. */

    init();   //initialize modules
    LTC6811_initialize(MD_FAST);

    uint8_t cfga[6];
    uint16_t aux;
    volatile uint16_t volts[16];
    uint8_t i; 
    
    uint16_t cell_voltages[2][12];
    
    BMS_OK_Write(0);

    while(1){
        LTC6811_wakeup();
        CyDelay(1);
        
        LTC6811_adcv();
        CyDelay(1);
        //LTC6811_rdcv(0, 2, cell_voltages);
        LTC6811_rdcv(0, 2, cell_voltages);
        
        LTC6811_rdcfga(1, cfga);
        BMS_OK_Write(1);
        for (i=0; i<16; i++){
            LTC6811_wrcfga(1, i, cfga);
            LTC6811_adax();
            CyDelayUs(200);
            LTC6811_rdaux_pin(1, GPIO5, &aux);
            volts[i] = aux;
        }
        BMS_OK_Write(0);
        CyDelay(1);
    }
}

/* [] END OF FILE */
