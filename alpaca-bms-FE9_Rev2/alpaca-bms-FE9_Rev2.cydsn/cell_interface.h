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

#ifndef CELL_INTERFACE_H
#define CELL_INTERFACE_H

#include <stdint.h>
#include <project.h>
#include "data.h"

#define ERROR_VOLTAGE_LIMIT (4u)
#define ERROR_TEMPERATURE_LIMIT (4u)
#define FUSE_BAD_LIMIT (10u)
#define BAD_FILTER_LIMIT (10u)

#define CELL_ENABLE_HIGH (0x7DF)
#define CELL_ENABLE_LOW (0x3DF)
#define OVER_VOLTAGE (4200u) // Updated for FE5 (4.2V)
#define UNDER_VOLTAGE (2500u) // Updated for FE5 (2.5V)
#define STACK_VOLT_DIFF_LIMIT (9000u)   //9 volt
#define CRITICAL_TEMP_L (0u)          // 0 C
#define CRITICAL_TEMP_H (60u)             //60 C
#define CRITICAL_TEMP_BOARD_L (0u)          // 0 C
#define CRITICAL_TEMP_BOARD_H (60u)  
#define BAD_THERM_LIMIT (15u)
#define SOC_NOMIAL      (50000*3600u)    //nomial SOC before calibration
#define SOC_CALI_HIGH (106000u)     //High cali point at 106V
#define SOC_SOC_HIGH  (60000*3600u)      //manually set it in mAh
#define SOC_CALI_LOW (80000u)     //Low Cali point at 80V
#define SOC_SOC_LOW   (10000*3600u)      //manually set it in mAh
#define SOC_FULL_CAP (75000*3600u)     //let's say, 75,000mAh
#define SOC_FULL      (115000u)   //when voltage reaches 115V, consider it full
#define BALANCE_THRESHOLD (10u)

// Stores cell data
uint16_t cell_codes[IC_PER_BUS * N_OF_BUSSES][12]; 
uint16_t cell_codes_lower[IC_PER_BUS][12]; 
uint16_t cell_codes_higher[IC_PER_BUS][12]; 

uint16_t aux_codes[IC_PER_BUS][5];

#define OVER_TEMP (60u)             //now it just for debug purpose
#define UNDER_TEMP (0u)

#define THERM_CELL (0u)
#define THERM_BOARD (1u)


// bms_status
#define NO_ERROR 0x0000
#define CHARGEMODE 0x0001
#define PACK_TEMP_OVER 0x0002
#define STACK_FUSE_BROKEN 0x0004
#define PACK_TEMP_UNDER 0x0008
#define LOW_SOC   0x0010
#define CRITICAL_SOC   0x0020
#define IMBALANCE   0x0040
#define COM_FAILURE   0x0080
#define NEG_CONT_CLOSED   0x0100
#define POS_CONT_CLOSED   0x0200 
#define ISO_FAULT   0x0400
#define CELL_VOLT_OVER   0x0800
#define CELL_VOLT_UNDER   0x1000
#define CHARGE_HAULT   0x2000
#define FULL   0x4000
#define PRECHARGE_CLOSED   0x8000


//new data stucture

typedef enum {
  NORMAL =0,
  WARNING =1,
  FAULT =2,
}BAT_HEALTH;


typedef struct
{
  volatile uint16_t err;
  volatile uint8_t bad_cell;
  volatile uint8_t bad_node;
}BAT_ERR_t;

typedef struct 
{
  volatile uint16_t voltage;
  volatile uint8_t bad_counter;
  volatile uint8_t bad_type;
}BAT_CELL_t;

typedef struct
{
  volatile uint16_t temp_raw;
  volatile double temp_c;
  volatile uint8_t bad_counter;
  volatile uint8_t type;
  volatile uint8_t bad_type;
  volatile uint16_t temp_ref;
}BAT_TEMP_t;

typedef struct
{
  volatile BAT_CELL_t *cells[N_OF_CELL / N_OF_SUBPACK]; // Cells per subpack
  volatile BAT_TEMP_t *temps[N_OF_TEMP / N_OF_SUBPACK]; // 14 Thermistors per subpack (measuring cells)
  volatile BAT_TEMP_t *board_temps[N_OF_TEMP_BOARD / N_OF_SUBPACK];
  volatile uint8 high_temp;
  volatile uint32_t over_temp;
  volatile uint32_t under_temp;
  volatile uint32_t over_voltage;
  volatile uint32_t under_temp_board;
  volatile uint32_t over_temp_board;
  volatile uint32_t under_voltage;
  volatile uint32_t voltage;
  volatile uint8_t bad_counter;
}BAT_SUBPACK_t;

typedef struct
{
  volatile BAT_SUBPACK_t *subpacks[N_OF_SUBPACK];
  volatile uint32_t voltage;
  volatile int16_t current;
  volatile uint8_t fuse_fault;
  volatile uint16_t status;
  volatile BAT_HEALTH health;
  volatile uint32_t current_charge;
  volatile uint8_t SOC_percent;
  volatile uint8_t SOC_cali_flag;
  volatile uint8_t HI_temp_c;
  volatile uint8_t HI_temp_board_c;
  volatile uint8_t HI_temp_board_node;
  volatile uint8_t HI_temp_node;
  volatile uint8_t HI_temp_node_index;
  volatile uint8_t HI_temp_raw;
  volatile uint16_t HI_voltage;
  volatile uint16_t LO_voltage;
  volatile uint16_t time_stamp;
}BAT_PACK_t;

typedef struct 
{
  volatile uint8_t percent_SOC;
  volatile uint32_t absolute_SOC;
}BAT_SOC_t;



//functions
void cell_interface_init();
void bms_init(uint8_t adc_mode);
void get_all_temps();
void get_voltages();

#endif
/* [] END OF FILE */
