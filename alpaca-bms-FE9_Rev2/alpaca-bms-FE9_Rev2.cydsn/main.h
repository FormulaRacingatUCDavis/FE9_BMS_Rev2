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
#ifndef MAIN_H
#define MAIN_H

#include "project.h"
#include "cell_interface.h"
#include "can_manager.h"
#include "LTC6811.h"
#include "math.h"
#include "time.h"
#include "pwm.h"
#include "cyapicallbacks.h"
#include "FTDI.h"
#include "SOC.h"


//The old code had many more BMS modes, are we ever going to need that?
//Need BMS_CHARGING at least. We only want to balance cells during charging. 
//BMS_CHARGING will share a lot with BMS_NORMAL, so maybe charging shouldn't be its own mode
typedef enum {
    BMS_NORMAL, 
    BMS_FAULT
}BMS_MODE;

extern BAT_PACK_t bat_pack;
extern BAT_SUBPACK_t bat_subpack[N_OF_SUBPACK];
extern volatile float32 sortedTemps[N_OF_TEMP_CELL]; 
extern volatile BAT_ERR_t* bat_err_array;
extern volatile uint8_t bat_err_index;
extern volatile uint8_t bat_err_index_loop;

void init();
int main();
void can_tasks();

#endif

/* [] END OF FILE */
