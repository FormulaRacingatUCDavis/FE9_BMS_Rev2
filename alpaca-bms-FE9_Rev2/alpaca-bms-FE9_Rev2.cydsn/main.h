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
#include "can_manager.h"
#include <LTC6811.h>
#include "RAD_PUMP_PWM.h"

#include "math.h"
#include <time.h>

//The old code had many more BMS modes, are we ever going to need that?
//Need BMS_CHARGING at least. We only want to balance cells during charging. 
//BMS_CHARGING will share a lot with BMS_NORMAL, so maybe charging shouldn't be its own mode
typedef enum {
    BMS_NORMAL, 
    BMS_FAULT
}BMS_MODE;

//global variables
//uint8_t cfga_on_init[6];
//uint8_t auxa[6];
volatile uint8_t CAN_UPDATE_FLAG=0;
extern volatile BAT_PACK_t bat_pack;
extern BAT_SUBPACK_t bat_subpack[N_OF_SUBPACK];
extern volatile float32 sortedTemps[N_OF_TEMP_CELL]; 
extern float32 high_temp_overall;
extern volatile BAT_ERR_t* bat_err_array;
extern volatile uint8_t bat_err_index;
extern volatile uint8_t bat_err_index_loop;
volatile uint8_t CAN_DEBUG=0;
//volatile uint8_t RACING_FLAG=0;    // this flag should be set in CAN handler
BAT_SOC_t bat_soc;
volatile VCU_STATE vcu_state = LV;
volatile VCU_ERROR vcu_error = NONE; 
uint8_t rx_cfg[IC_PER_BUS][8];
void DEBUG_send_cell_voltage();
void DEBUG_send_temp();
//void DEBUG_send_current();



/* [] END OF FILE */
