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

#include <cell_interface.h>
#include <LTC6811.h>

void cell_interface_init(){
    LTC6811_init_cfg();
}

void  bms_init(uint8_t adc_mode){
    ss_SetDriveMode(ss_DM_RES_UP);
    SPI_Start();
    LTC6811_initialize(adc_mode);
    LTC6811_init_cfg();             //initialize local cfga values
    LTC6811_wrcfga(0xFF);           //write to cfga register for all LTC6811s
}

void get_all_temps(){
    uint16_t temps[N_OF_LTC][TEMPS_PER_LTC];
    uint8_t select;
    uint8_t ltc_addr;
    
    for(select = 0; select < TEMPS_PER_LTC; select++){       //for each mux pin
        for(ltc_addr = 0; ltc_addr < N_OF_LTC; ltc_addr++){  //for each LTC
            LTC6811_set_cfga_mux(ltc_addr, select);             //set tx_cfg variable
            LTC6811_wrcfga(ltc_addr);                           //write to LTC with wrcfga
        }
        
        LTC6811_adax(); //run ADC conversion (all LTCs)
        CyDelay(1);
        
        for(ltc_addr = 0; ltc_addr < N_OF_LTC; ltc_addr++){                //for each LTC
            LTC6811_rdaux_pin(ltc_addr, GPIO5, &temps[ltc_addr][select]);      //get ADC result
        }
    }
    CyDelay(1); 
}

void get_voltages(){
    LTC6811_adcv();  //run ADC conversion (all LTCs)
    uint16_t cell_voltages[N_OF_LTC][CELLS_PER_LTC];
    
    if(LTC6811_rdcv_ltc(1, cell_voltages[1])){
        CyDelay(1);
    } else {
        CyDelay(1);
    }
}

/* [] END OF FILE */
