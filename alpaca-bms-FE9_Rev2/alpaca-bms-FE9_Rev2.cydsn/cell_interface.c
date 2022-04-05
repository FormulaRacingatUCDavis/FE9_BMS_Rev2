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

//MUX Indexes:   
//First LTC on Node: 
//IC1_ADDRESSES: 0, 2, 4, 6, 8 
uint8_t CELL_TEMP_INDEXES_IC1[12] = {0, 1, 2, 3, 4, 5, 6, 7, 12, 13, 14, 15};  //cell thermistors
uint8_t BOARD_TEMP_INDEXES_IC1[3] = {8, 10, 11};                                //board thermistors
uint8_t HUMIDITY_INDEXES_IC1 = {9};                                            //humidity sensors

//Second LTC on Node: 
//IC2_ADDRESSES: 1, 3, 5, 7, 9 
uint8_t CELL_TEMP_INDEXES_IC2[12] = {0, 1, 2, 3, 8, 9, 10, 11, 12, 13, 14, 15};  //cell thermistors
uint8_t BOARD_TEMP_INDEXES_IC2[4] = {4, 5, 6, 7};                               //board thermistors


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
        
        for(ltc_addr = 0; ltc_addr < N_OF_LTC; ltc_addr++){                    //for each LTC
            if(LTC6811_rdaux_pin(ltc_addr, GPIO5, &temps[ltc_addr][select])){  //get ADC result
                CyDelay(1); //TODO: pec error
            }
        }
    }
    
    //sort temps
    for(uint8_t pack = 0; pack < N_OF_SUBPACK; pack++){
        volatile uint16_t cell_temps[CELL_TEMPS_PER_PACK]; 
        volatile uint16_t board_temps[BOARD_TEMPS_PER_PACK];
        uint8_t cell_temp_counter = 0;
        uint8_t board_temp_counter = 0; 
        uint8_t i;
        
        ltc_addr = pack*IC_PER_PACK;
        //IC1 cell temps
        for(i = 0; i < sizeof(CELL_TEMP_INDEXES_IC1); i++){
            cell_temps[cell_temp_counter] = temps[ltc_addr][CELL_TEMP_INDEXES_IC1[i]];
            cell_temp_counter++;
        }
        
        //IC1 board temps
        for(i = 0; i < sizeof(BOARD_TEMP_INDEXES_IC1); i++){
            board_temps[board_temp_counter] = temps[ltc_addr][BOARD_TEMP_INDEXES_IC1[i]];
            board_temp_counter++;
        }
        
        ltc_addr = pack*IC_PER_PACK + 1;
        //IC2 cell temps
        for(i = 0; i < sizeof(CELL_TEMP_INDEXES_IC2); i++){
            cell_temps[cell_temp_counter] = temps[ltc_addr][CELL_TEMP_INDEXES_IC2[i]];
            cell_temp_counter++;
        }
        
        //IC2 board temps
        for(i = 0; i < sizeof(BOARD_TEMP_INDEXES_IC2); i++){
            board_temps[board_temp_counter] = temps[ltc_addr][BOARD_TEMP_INDEXES_IC2[i]];
            board_temp_counter++;
        }
        
        CyDelay(1);
        
    }
    CyDelay(1); 
}

void get_voltages(){
    LTC6811_adcv();  //run ADC conversion (all LTCs)
    CyDelay(1);
    
    uint16_t cell_voltages[N_OF_LTC][CELLS_PER_LTC];
    
    for(uint8_t addr = 0; addr < N_OF_LTC; addr++){
        if(LTC6811_rdcv_ltc(addr, cell_voltages[addr])){
            CyDelay(1);  //TODO: pec error
        }
    }
    
    CyDelay(1);
}

/* [] END OF FILE */
