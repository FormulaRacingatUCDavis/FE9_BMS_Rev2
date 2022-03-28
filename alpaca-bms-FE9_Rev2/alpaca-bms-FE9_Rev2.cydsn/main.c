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
        get_voltages();
        get_all_temps();
    }
}

/* [] END OF FILE */
