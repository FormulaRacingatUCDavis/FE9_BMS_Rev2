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
#include "cell_interface.h"
#include <LTC6811.h>

void init(void){   //initialize modules
    SPI_Start();  
    LTC6811_initialize(MD_NORMAL); 
    LTC6811_init_cfg();
}


int main(void)
{  //SEE ADOW ON DATASHEET PAGE 33
    
    CyGlobalIntEnable; /* Enable global interrupts. */

    init();   //initialize modules

    while(1) {
        /*for(uint8_t i = 0; i < 12; i++){
            LTC6811_set_cfga_reset_discharge(1);
            LTC6811_set_cfga_discharge_cell(1, i); 
            LTC6811_wrcfga(1);
            get_voltages();
        }*/
        //LTC6811_set_cfga_discharge_cell(1, 5);
        //get_voltages();
        get_all_temps();
    }
}

/* [] END OF FILE */
