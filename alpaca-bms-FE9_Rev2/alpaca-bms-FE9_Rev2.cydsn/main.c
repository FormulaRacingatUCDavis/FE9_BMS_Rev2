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
    volatile uint8_t read_cfga[15][6];
    uint16_t temp;
    volatile uint16_t aux[16];
    uint16_t auxb[2][6];
    //uint8_t sel = 5;
    
    BMS_OK_Write(1);

    while(1){
        LTC6811_rdcfga(1, cfga);
        CyDelay(1);
        for (uint8_t sel = 0; sel < 16; sel++){
            LTC6811_wrcfga(1, sel, cfga);
            CyDelay(100);
            LTC6811_rdcfga(1, read_cfga[sel]);
            CyDelay(50);
            LTC6811_adax();
            CyDelay(100);
            
            if(LTC6811_rdaux_pin(1, GPIO5, &temp)){
                BMS_OK_Write(0);
                aux[sel] = 0xFFFF;
            } else {
            aux[sel] = temp;
            }
            
            CyDelay(100);
        }
 
        //CyDelay(1);
       // 
        //
        //LTC6811_rdaux(0, 2, auxb);
        //BMS_OK_Write(0);
        //if(!LTC6811_rdcfga(1, cfga)){
            //if(cfga[0] == 0b00101110){
            //    BMS_OK_Write(1);
            //}
        //}
        CyDelay(100);
    }
}

/* [] END OF FILE */
