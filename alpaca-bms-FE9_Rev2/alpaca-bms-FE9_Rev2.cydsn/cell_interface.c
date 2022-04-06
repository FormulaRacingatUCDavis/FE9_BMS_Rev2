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
#include <math.h>
#include <stdlib.h>

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

BAT_CELL_t bat_cell[N_OF_CELL];
BAT_TEMP_t bat_temp[N_OF_TEMP_CELL];
BAT_TEMP_t board_temp[N_OF_TEMP_BOARD];
BAT_SUBPACK_t bat_subpack[N_OF_SUBPACK];
volatile BAT_ERR_t bat_err;
BAT_PACK_t bat_pack;

void bms_init(uint8_t adc_mode){
    LTC6811_initialize(adc_mode);
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
        uint8_t cell_temp_counter = 0;
        uint8_t board_temp_counter = 0; 
        uint8_t i;
        uint16_t raw_temp; 
        
        ltc_addr = pack*IC_PER_PACK;
        //IC1 cell temps
        for(i = 0; i < sizeof(CELL_TEMP_INDEXES_IC1); i++){
            raw_temp = temps[ltc_addr][CELL_TEMP_INDEXES_IC1[i]];
            bat_pack.subpacks[pack]->cell_temps[cell_temp_counter]->temp_raw = raw_temp;
            bat_pack.subpacks[pack]->cell_temps[cell_temp_counter]->temp_c = rawToCelcius(raw_temp);
            cell_temp_counter++;
        }
        
        //IC1 board temps
        for(i = 0; i < sizeof(BOARD_TEMP_INDEXES_IC1); i++){
            raw_temp = temps[ltc_addr][BOARD_TEMP_INDEXES_IC1[i]];
            bat_pack.subpacks[pack]->board_temps[board_temp_counter]->temp_raw = raw_temp;
            bat_pack.subpacks[pack]->board_temps[board_temp_counter]->temp_c = rawToCelcius(raw_temp);
            board_temp_counter++;
        }
        
        ltc_addr = pack*IC_PER_PACK + 1;
        //IC2 cell temps
        for(i = 0; i < sizeof(CELL_TEMP_INDEXES_IC2); i++){
            raw_temp = temps[ltc_addr][CELL_TEMP_INDEXES_IC2[i]];
            bat_pack.subpacks[pack]->cell_temps[cell_temp_counter]->temp_raw = raw_temp;
            bat_pack.subpacks[pack]->cell_temps[cell_temp_counter]->temp_c = rawToCelcius(raw_temp);
            cell_temp_counter++;
        }
        
        //IC2 board temps
        for(i = 0; i < sizeof(BOARD_TEMP_INDEXES_IC2); i++){
            raw_temp = temps[ltc_addr][BOARD_TEMP_INDEXES_IC2[i]];
            bat_pack.subpacks[pack]->board_temps[board_temp_counter]->temp_raw = raw_temp;
            bat_pack.subpacks[pack]->board_temps[board_temp_counter]->temp_c = rawToCelcius(raw_temp);
            board_temp_counter++;
        }
        
        CyDelay(1);
        
    }
    CyDelay(1); 
}

float32 rawToCelcius(uint16_t raw){
    float32 temp = (float32)raw/10000;
    //temp = (1/((1/298.15) + ((1/3428.0)*log(temp/(3-temp))))) - 273.15;
    
    return 0;
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

/**
 * @initialize the mypack struct. 
 *
 * @param no input parameters.
 * @return 1 if everything is OK. 0 for hard failure.
 */
void mypack_init(){
    
    uint8_t cell = 0;
    uint8_t subpack = 0;
    uint8_t temp = 0;
    //bat_err_index = 0;
    //bat_err_index_loop = 0;

    // initialize cells and temps
    
    for (cell = 0; cell < N_OF_CELL; cell++){
        bat_cell[cell].voltage = 0;
        bat_cell[cell].bad_counter = 0;
    }
    
    for (temp = 0; temp < N_OF_TEMP_CELL; temp++){
        bat_temp[temp].temp_c = (uint8_t)temp;
        bat_temp[temp].temp_raw = (uint16_t)temp;
        bat_temp[temp].bad_counter = 0;
        bat_temp[temp].bad_type = 0;
        bat_temp[temp].type = THERM_CELL;
    }
    for (temp = 0; temp < N_OF_TEMP_BOARD; temp++){
        board_temp[temp].temp_c = (uint8_t)temp;
        board_temp[temp].temp_raw = (uint16_t)temp;
        board_temp[temp].bad_counter = 0;
        board_temp[temp].bad_type = 0;
        board_temp[temp].type = THERM_BOARD;
    }

    for (subpack = 0; subpack < N_OF_SUBPACK; subpack++){
        for (cell = 0; cell < (CELLS_PER_SUBPACK); cell++){    
            bat_subpack[subpack].cells[cell] = &(bat_cell[subpack*(CELLS_PER_SUBPACK)+cell]);
        }
        for (temp = 0; temp < (CELL_TEMPS_PER_PACK); temp++){
            bat_subpack[subpack].cell_temps[temp] = &(bat_temp[subpack*(CELL_TEMPS_PER_PACK)+temp]);
        }
        for (temp = 0; temp < (BOARD_TEMPS_PER_PACK); temp++){
            bat_subpack[subpack].board_temps[temp] = &(board_temp[subpack*(BOARD_TEMPS_PER_PACK)+temp]);
        }
        
        bat_subpack[subpack].over_temp_cell = 0;
        bat_subpack[subpack].under_temp_cell = 0;
        bat_subpack[subpack].over_voltage = 0;
        bat_subpack[subpack].under_voltage = 0;
        bat_subpack[subpack].voltage = 0;
    }
    
    // Register pack
    for (subpack = 0; subpack < N_OF_SUBPACK; subpack++){
        bat_pack.subpacks[subpack] = &(bat_subpack[subpack]);
    }
    
    //initialize pack values
    bat_pack.voltage = 0;
    bat_pack.current = 0;
    bat_pack.fuse_fault = 0;
    bat_pack.status = 0; 
    bat_pack.health = NORMAL;
    bat_pack.SOC_cali_flag =0;
    bat_pack.HI_temp_c = 0;
    bat_pack.HI_temp_raw = 0;
    bat_pack.HI_voltage = 0;
    bat_pack.LO_voltage = 0;
    bat_pack.time_stamp = 0;
    bat_pack.SOC_percent = 0;
}

/* [] END OF FILE */
