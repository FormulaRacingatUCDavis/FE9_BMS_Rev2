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

extern volatile VCU_STATE vcu_state; 

BAT_PACK_t bat_pack;
BAT_SUBPACK_t bat_subpack[N_OF_SUBPACK];

BAT_CELL_t bat_cell[N_OF_CELL];
BAT_TEMP_t bat_temp[N_OF_TEMP_CELL];
PACK_HUMIDITY_t pack_humidity[N_OF_SUBPACK];

volatile uint16_t OW[N_OF_LTC];   //stores binary results of open wire check

uint8_t spi_error_counter[N_OF_LTC];   //stores the number of SPI communication errors for each LTC
uint16_t temps[N_OF_LTC][CELL_TEMPS_PER_LTC];   //store all temps in matrix

uint8_t counter = 0;  //counter for get_temps


//initialize important stuff
void cell_interface_init(){
    LTC6811_init_cfg();          //set cfga to defaults
    set_adc_mode(MD_FILTERED);   //set ADC mode to filtered
    SPI_Start();                 //start isoSPI module
    mypack_init();               //initialize bat_pack
}

/*
set_adc_mode: 

Sets LTC6811 adc mode. 

Parameters: 
adc_mode - MD_FAST, MD_NORMAL, or MD_FILTERED
*/
void set_adc_mode(uint8_t adc_mode){
    LTC6811_set_adc(adc_mode, DCP_DISABLED, CELL_CH_ALL, AUX_CH_GPIO5);
}

//reads cell voltages from LTCs
//stores in bat_pack
void get_voltages(){
    set_adc_mode(MD_FILTERED);  //higher accuracy for voltage measurements
    SPI_ClearFIFO();
    
    LTC6811_adcv();  //run ADC conversion (all LTCs)
    CyDelay(5);
    
    uint16_t cell_voltages[CELLS_PER_LTC];  //temporarily store LTC voltages
    uint8_t addr, result; 
    
    for(addr = 0; addr < N_OF_LTC; addr++){
        result = LTC6811_rdcv_ltc(addr, cell_voltages);
        update_spi_errors(addr, result);
        
        CyDelay(1);
        
        //move voltages into bat_pack if no SPI fault
        if(!result){
            for(uint8_t cell = 0; cell < CELLS_PER_LTC; cell++){
                setVoltage_addr(addr, cell, cell_voltages[cell]);
            }
        }
    }
    
    CyDelay(1);
}

//gets only a portion of temps each call to speed up main loop
//fraction of temps set by TEMP_LOOP_DIVISION
void get_temps(){
    if(counter < (TEMP_LOOP_DIVISION-1)){
        get_temps_from_to(counter*TEMPS_PER_LOOP, (counter + 1)*TEMPS_PER_LOOP);     //update temperatures from packs
        counter++;
    } else {
        get_temps_from_to(counter*TEMPS_PER_LOOP, CELL_TEMPS_PER_LTC);
        sort_temps();    //sort temps in to bat pack
        check_temps();   //parse temps
        counter = 0; 
    }
}

//collects subset of temperatures so voltages can be collected more often
void get_temps_from_to(uint8_t start_sel, uint8_t end_sel){
    set_adc_mode(MD_NORMAL); //lower accuracy for temp measurements
    SPI_ClearFIFO();
    
    uint8_t select, addr, result;
    
    for(select = start_sel; select < end_sel; select++){       //for each mux pin
        for(addr = 0; addr < N_OF_LTC; addr++){  //for each LTC
            LTC6811_set_cfga_mux(addr, select);             //set tx_cfga variable
            LTC6811_wrcfga(addr);                           //write to LTC with wrcfga
        }
        
        LTC6811_adax(); //run ADC conversion (all LTCs)
        CyDelay(1);
        
        for(addr = 0; addr < N_OF_LTC; addr++){                             //for each LTC
            result = LTC6811_rdaux_pin(addr, GPIO5, &temps[addr][select]);  //get ADC result
            update_spi_errors(addr, result);
        }
    }
}

void sort_temps(){
    //sort temps into bat_pack                             //board thermistors
    
    uint8_t ltc_addr; 
    for(uint8_t pack = 0; pack < N_OF_SUBPACK; pack++){
        uint8_t i;
        uint16_t raw_temp;
        
        ltc_addr = pack*IC_PER_SUBPACK;
        //IC1 cell temps
        for(i = 0; i < CELL_TEMPS_PER_LTC; i++){
            raw_temp = temps[ltc_addr][i];
            setCellTemp(pack, i, raw_temp);
        }  
    }  
}

void open_wire_check(){
    SPI_ClearFIFO();
    
    volatile uint16_t CELL[2][N_OF_LTC][CELLS_PER_LTC];   //stores pull up and pull down cell voltage readings
    volatile int16_t CELL_DELTA[N_OF_LTC][CELLS_PER_LTC]; 
    uint8_t addr, result; 
    
    //From page 34 of datasheet: 
    
    //1. Run the 12-cell command ADOW with PUP = 1 at least
    //twice. Read the cell voltages for cells 1 through 12 once
    //at the end and store them in array CELL_PD(n).
    
    for(int8_t pup = 1; pup >= 0; pup--){
        for(uint8_t i = 0; i < 10; i++){
            LTC6811_adow(pup);  //run ADC conversion (all LTCs)
            CyDelay(5);
        } 
        
        for(addr = 0; addr < N_OF_LTC; addr++){
            result = LTC6811_rdcv_ltc(addr, CELL[pup][addr]);
            update_spi_errors(addr, result);
        }
    }
    
    uint8_t cell; 
    
    for(addr = 0; addr < N_OF_LTC; addr++){
        //3.
        for(cell = 1; cell < CELLS_PER_LTC; cell++){
            CELL_DELTA[addr][cell] = CELL[1][addr][cell] - CELL[0][addr][cell]; 
        }
        
        OW[addr] = 0; 
        
        //4. 
        for(cell = 1; cell < CELLS_PER_LTC; cell++){
            if(CELL_DELTA[addr][cell] < -4000){  //< -400mV
                OW[addr] |= (1<<cell);  //(C(cell) is open
            }
        }
        
        if(CELL[1][addr][0] < 50){
            OW[addr] |= 1; //C(0) is open
        }
        
        if(CELL[0][addr][CELLS_PER_LTC-1] < 50){
            OW[addr] |= (1<<12); //C(12) is open
        }
        
    }
    
}


void check_voltages(){
    uint8_t cell, subpack;
    uint16_t voltage16 = 0;
    uint16_t max_voltage = 0;
    uint16_t min_voltage = 0xFFFF;
    uint32_t total_voltage = 0; 

    // Check each cell
    for (cell = 0; cell < N_OF_CELL; cell++){
        voltage16 = bat_cell[cell].voltage;
        
        //find max and min
        if (max_voltage < voltage16){
            max_voltage = voltage16;
        }
        if(min_voltage > voltage16){// && cell != 95 && cell != 24){ //ignore (3, 23), (1,0) TODO: REMOVE LATER
            min_voltage = voltage16;
        }
        
        if (voltage16 > (uint16_t)OVER_VOLTAGE){
            bat_cell[cell].bad_counter++;
            bat_cell[cell].bad_type = 1;
        }else if (voltage16 < (uint16_t)UNDER_VOLTAGE){
            bat_cell[cell].bad_counter++;
            bat_cell[cell].bad_type = 0;
        //Check if fuse blown
        } else if ((voltage16 - bat_pack.LO_voltage) > IMBALANCE_THRESHOLD){
            bat_cell[cell].bad_counter++;
            bat_cell[cell].bad_type = 2;
            //check where bad_type is checked
        } else {
            if (bat_cell[cell].bad_counter > 0){
                bat_cell[cell].bad_counter--;
            }           
        }
        
        total_voltage += bat_cell[cell].voltage; 
    }
    
    bat_pack.HI_voltage = max_voltage;
    bat_pack.LO_voltage = min_voltage;
    bat_pack.voltage = total_voltage/100; 

    
    // Update subpacks & pack for errors
    for (subpack = 0; subpack < N_OF_SUBPACK; subpack++){
        total_voltage = 0; 
        for (cell = 0; cell < (N_OF_CELL / N_OF_SUBPACK); cell++){
            if (bat_subpack[subpack].cells[cell]->bad_counter > ERROR_VOLTAGE_LIMIT){
                if (bat_subpack[subpack].cells[cell]->bad_type == 0){
                    bat_subpack[subpack].under_voltage |= (1u<<cell);
                    bat_pack.status |= CELL_VOLT_UNDER;
                } else if (bat_subpack[subpack].cells[cell]->bad_type == 1) {
                    bat_subpack[subpack].over_voltage |= (1u<<cell);
                    bat_pack.status  |= CELL_VOLT_OVER;
                } else if (bat_subpack[subpack].cells[cell]->bad_type == 2) {
                    // Added case for blown fuse
                    bat_subpack[subpack].fuse_blown|= (1u<<cell);
                    bat_pack.status  |= FUSE_BLOWN;                   
                }
            }
            total_voltage += bat_subpack[subpack].cells[cell]->voltage; 
        }
        bat_subpack[subpack].voltage = total_voltage; 
    } 
    
    //check for spi errors
    for(uint8_t addr = 0; addr < N_OF_LTC; addr++){
        if(spi_error_counter[addr] > SPI_ERROR_LIMIT){
            bat_pack.status |= SPI_FAULT;         
            bat_pack.spi_error_address |= (1u<<addr);
        }
    }
}

void check_temps(){
    uint8_t subpack, cell;
    uint16_t temp_c, temp;
    
    //float median = getMedianTemp();
    
    // check cell temps
    for (cell = 0; cell < N_OF_TEMP_CELL; cell++){
        temp_c = bat_temp[cell].temp_c;

        if(temp_c < TEMP_IGNORE_LIMIT){
            if (temp_c > (uint8_t)CRITICAL_TEMP_H){  //if over temperature
                bat_temp[cell].bad_counter++;
                bat_temp[cell].bad_type = 1;
            }else if (temp_c < (uint8_t)CRITICAL_TEMP_L){ // if under temperature
                bat_temp[cell].bad_counter++;
                bat_temp[cell].bad_type = 0;
            }else{    //if there is no error
                if (bat_temp[cell].bad_counter > 0){
                    bat_temp[cell].bad_counter--;
                }           
            }
        }
    }
   
   
    // Update subpacks
    for (subpack = 0; subpack < N_OF_SUBPACK; subpack++){
        for (temp = 0; temp < (CELL_TEMPS_PER_PACK); temp++){
            if (bat_subpack[subpack].cell_temps[temp]->bad_counter > ERROR_TEMPERATURE_LIMIT){
                if (bat_subpack[subpack].cell_temps[temp]->bad_type == 0){
                    bat_subpack[subpack].under_temp_cell |= (1u<<temp);
                }else{
                    bat_subpack[subpack].over_temp_cell |= (1u<<temp);
                }
            }
        }
    }
    
    // update max temps
    uint16_t max_temp;
    uint32_t avg_temp = 0;
    uint8_t i;
    
    bat_pack.HI_temp_c = 0;
    bat_pack.HI_temp_raw = 0;
    bat_pack.LO_temp_c = 0xFF;
    
    
    for (subpack = 0; subpack < N_OF_SUBPACK; subpack++){
        max_temp = 0;
        
        for (i = 0; i < (CELL_TEMPS_PER_PACK); i++){
            temp = (uint16_t)getCellTemp(subpack, i);
            
            if(temp < TEMP_IGNORE_LIMIT){
                //update subpack max temp
                if (max_temp < temp){
                    max_temp = temp;
                }
                
                //update pack max temp
                if (temp > bat_pack.HI_temp_c){
                    bat_pack.HI_temp_c = temp;
                    bat_pack.HI_temp_raw = bat_pack.subpacks[subpack]->cell_temps[i]->temp_raw;
                    bat_pack.HI_temp_subpack = subpack;
                    bat_pack.HI_temp_subpack_index = i;
                } 
                
                //update pack min temp
                if (temp < bat_pack.LO_temp_c){
                    bat_pack.LO_temp_c = temp;
                }
                
                avg_temp += temp;
            }
        }
        
        bat_pack.subpacks[subpack]->high_temp = max_temp;
    }
    
    bat_pack.AVG_temp_c = avg_temp/N_OF_TEMP_CELL;

    // update pack of temp error
    for (subpack = 0; subpack < N_OF_SUBPACK; subpack++){
        if (bat_pack.subpacks[subpack]->over_temp_cell != 0){
            bat_pack.status |= PACK_TEMP_OVER;  
        }

        if (bat_pack.subpacks[subpack]->under_temp_cell != 0){
            bat_pack.status  |= PACK_TEMP_UNDER;
        }
    }
    
}

//make sure no cells will discharge
void disable_cell_balancing(){
    LTC6811_set_cfga_reset_discharge(); 
    
    for(uint8_t addr = 0; addr < N_OF_LTC; addr++){
        LTC6811_wrcfga(addr);
    }
}

//update LTCs to balance cells too far above minimum voltage
void balance_cells(){
    uint8_t addr, cell; 
    uint16_t target_voltage = bat_pack.LO_voltage;
    
    LTC6811_set_cfga_reset_discharge(); //clear previous discharges
    
    for(addr = 0; addr<N_OF_LTC; addr++){
        for(cell=0; cell<CELLS_PER_LTC; cell++){
            
            volatile int32_t difference = getVoltage_addr(addr, cell);
            difference -= target_voltage; 
            if(difference > BALANCE_THRESHOLD){
                LTC6811_set_cfga_discharge_cell(addr, cell); //discharge cell
            }
        }
         
        LTC6811_wrcfga(addr);  //write updated cfga values to LTC
    }
}

void setVoltage(uint8_t pack, uint8_t index, uint16_t raw_voltage){
    bat_pack.subpacks[pack]->cells[index]->voltage = raw_voltage;
}

void setVoltage_addr(uint8_t addr, uint8_t index, uint16_t raw_voltage){
    uint8_t subpack = addr/IC_PER_SUBPACK;
    uint8_t ic_num = addr%IC_PER_SUBPACK;
    uint8_t pack_index = ic_num*CELLS_PER_LTC + index;
    setVoltage(subpack, pack_index, raw_voltage);
}

uint16_t getVoltage(uint8_t pack, uint8_t index){
    return bat_pack.subpacks[pack]->cells[index]->voltage;
}

uint16_t getVoltage_addr(uint8_t addr, uint8_t index){
    uint8_t pack = addr/IC_PER_SUBPACK;
    index = index % CELLS_PER_LTC;
    return getVoltage(pack, index);
}

void setCellTemp(uint8_t pack, uint8_t index, uint16_t raw_temp){
    bat_pack.subpacks[pack]->cell_temps[index]->temp_raw = raw_temp;
    bat_pack.subpacks[pack]->cell_temps[index]->temp_c = rawToCelcius(raw_temp);
}

double getCellTemp(uint8_t pack, uint8_t index){
    return bat_pack.subpacks[pack]->cell_temps[index]->temp_c;
}

float32 rawToCelcius(uint16_t raw){
    float32 temp = (float32)raw/10000;
    temp = (1/((1/298.15) + ((1/3428.0)*log(temp/(3-temp))))) - 273.15;
    
    return temp;
}

void update_spi_errors(uint8_t addr, uint8_t result){
    if(result){
        spi_error_counter[addr]++;   //spi error
    } else {
        spi_error_counter[addr] = 0; //no spi error
    }
}

// Also copied from old code, to be edited to fit our use.
uint8_t bat_health_check(){
    if (
        (bat_pack.status & PACK_TEMP_OVER) ||
        //(bat_pack.status & FUSE_BLOWN) ||
        (bat_pack.status & PACK_TEMP_UNDER) ||
        //(bat_pack.status & IMBALANCE) || not in use
        (bat_pack.status & COM_FAILURE) ||
        (bat_pack.status & ISO_FAULT) || 
        (bat_pack.status & CELL_VOLT_OVER) ||
        (bat_pack.status & CELL_VOLT_UNDER)      
    ){
        bat_pack.health = FAULT;
        return 1;
    }else{
        return 0;
    }   
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
    
    //initialize humidity values
    for (uint8_t i=0; i<N_OF_SUBPACK;i++){
        pack_humidity[i].humidity_raw=0;
        pack_humidity[i].humidity=0;
    }
    
    //initialize spi error counter
    for (uint8_t addr = 0; addr < N_OF_LTC; addr++){
        spi_error_counter[addr] = 0;
    }

    for (subpack = 0; subpack < N_OF_SUBPACK; subpack++){
        for (cell = 0; cell < (CELLS_PER_SUBPACK); cell++){
            //bat_subpack is a pointer to bat_cell
            bat_subpack[subpack].cells[cell] = &(bat_cell[subpack*(CELLS_PER_SUBPACK)+cell]);
        }
        for (temp = 0; temp < (CELL_TEMPS_PER_PACK); temp++){
            bat_subpack[subpack].cell_temps[temp] = &(bat_temp[subpack*(CELL_TEMPS_PER_PACK)+temp]);
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
    //bat_pack.fuse_fault = 0;
    bat_pack.status = 0; 
    bat_pack.health = NORMAL;
    //bat_pack.SOC_cali_flag =0;
    bat_pack.HI_temp_c = 0;
    bat_pack.HI_temp_raw = 0;
    bat_pack.HI_voltage = 0;
    bat_pack.LO_voltage = 0;
    //bat_pack.time_stamp = 0;
    bat_pack.SOC_percent = 0;
}

// End of copy
/* [] END OF FILE */
