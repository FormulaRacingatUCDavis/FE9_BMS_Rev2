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
    SS_SetDriveMode(SS_DM_RES_UP);
    LTC68_Start();
    LTC6804_initialize(adc_mode);
    Select6820_Write(1); // Configure each bus
    LTC6804_wrcfg(IC_PER_BUS, tx_cfg);
    //Select6820_Write(1);
    //LTC6804_wrcfg(IC_PER_BUS, tx_cfg);
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
}

void get_voltages(){
    LTC6811_adcv();  //run ADC conversion (all LTCs)
    uint16_t cell_voltages[N_OF_LTC][CELLS_PER_LTC];
    
    LTC6811_rdcv(0, N_OF_LTC, cell_voltages);
}

/* [] END OF FILE */
