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
#include <can_manager.h>
#include <LTC6811.h>
#include <math.h>
#include <stdlib.h>

#define ERROR_VOLTAGE_LIMIT (4u)
#define ERROR_TEMPERATURE_LIMIT (4u)
#define FUSE_BAD_LIMIT (10u)
#define BAD_FILTER_LIMIT (10u)
#define SPI_ERROR_LIMIT (4u)

#define CELL_ENABLE_HIGH (0x7DF)
#define CELL_ENABLE_LOW (0x3DF)
#define OVER_VOLTAGE (42000u) //(4.2V)
#define UNDER_VOLTAGE (25000u) //(2.5V)
#define STACK_VOLT_DIFF_LIMIT (90000u)   //9 volt
#define CRITICAL_TEMP_L (0u)          // 0 C
#define CRITICAL_TEMP_H (60u)             //60 C
#define CRITICAL_TEMP_BOARD_L (0u)          // 0 C
#define CRITICAL_TEMP_BOARD_H (120u)  
#define BAD_THERM_LIMIT (15u)
#define SOC_NOMIAL      (50000*3600u)    //nomial SOC before calibration
#define SOC_CALI_HIGH (900000u)     //High cali point at 90V?
#define SOC_SOC_HIGH  (60000*3600u)      //manually set it in mAh
#define SOC_CALI_LOW (700000)     //Low Cali point at 70V
#define SOC_SOC_LOW   (10000*3600u)      //manually set it in mAh
#define SOC_FULL_CAP (75000*3600u)     //let's say, 75,000mAh
#define SOC_FULL (OVER_VOLTAGE*N_OF_CELL)   //when voltage reaches 100.8V, consider it full
#define BALANCE_THRESHOLD (20)     //balance to within 2mV
#define IMBALANCE_THRESHOLD (2000) //200mV imbalance = fault

uint16_t aux_codes[IC_PER_BUS][5];

#define OVER_TEMP (60u)             //now it just for debug purpose
#define UNDER_TEMP (0u)

#define THERM_CELL (0u)
#define THERM_BOARD (1u)

#define TEMP_IGNORE_LIMIT (500u) //ignore temperatures over this value, probably a bad thermistor

// bms_status
#define NO_ERROR 0x0000
#define CHARGEMODE 0x0001
#define PACK_TEMP_OVER 0x0002
#define FUSE_BLOWN 0x0004
#define PACK_TEMP_UNDER 0x0008
#define LOW_SOC   0x0010
#define CRITICAL_SOC   0x0020
#define IMBALANCE   0x0040
#define COM_FAILURE   0x0080
#define NEG_CONT_CLOSED   0x0100
#define POS_CONT_CLOSED   0x0200 
#define ISO_FAULT   0x0400
#define SPI_FAULT   0x0400
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
  volatile uint16_t humidity_raw; 
  volatile uint8_t humidity; 
}PACK_HUMIDITY_t;

typedef struct
{
  volatile BAT_CELL_t *cells[CELLS_PER_SUBPACK]; // Cells per subpack
  volatile BAT_TEMP_t *cell_temps[CELL_TEMPS_PER_PACK]; // 14 Thermistors per subpack (measuring cells)
  volatile uint8 high_temp;
  volatile uint32_t over_temp_cell;
  volatile uint32_t under_temp_cell;
  volatile uint32_t over_voltage;
  volatile uint32_t fuse_blown; 
  volatile uint32_t under_temp_board;
  volatile uint32_t over_temp_board;
  volatile uint32_t under_voltage;
  volatile uint32_t voltage;
  volatile uint8_t bad_counter;
}BAT_SUBPACK_t;

typedef struct
{
  volatile BAT_SUBPACK_t *subpacks[N_OF_SUBPACK];
  volatile uint16_t voltage;
  volatile int16_t current;
  //volatile uint8_t fuse_fault;
  volatile uint16_t status;
  volatile BAT_HEALTH health;
  //volatile uint32_t current_charge;
  volatile uint8_t SOC_percent;
  //volatile uint8_t SOC_cali_flag;
  volatile uint8_t HI_temp_c;
  volatile uint8_t HI_temp_subpack;
  volatile uint8_t HI_temp_subpack_index;
  volatile uint8_t HI_temp_raw;
  volatile uint16_t HI_voltage;
  volatile uint16_t LO_voltage;
  //volatile uint16_t time_stamp;
  volatile uint16_t spi_error_address;    
}BAT_PACK_t;

typedef struct 
{
  volatile uint8_t percent_SOC;
  volatile uint32_t absolute_SOC;
}BAT_SOC_t;


//FUNCTIONS
void cell_interface_init(); 
void set_adc_mode(uint8_t adc_mode);

void get_temps();
void get_temps_from_to(uint8_t start_sel, uint8_t end_sel);
void sort_temps();
void get_voltages();
void open_wire_check(); 

void check_voltages();
void check_temps();

void disable_cell_balancing();
void balance_cells();

void mypack_init();

void setVoltage(uint8_t pack, uint8_t index, uint16_t raw_voltage);
void setVoltage_addr(uint8_t addr, uint8_t index, uint16_t raw_voltage);
uint16_t getVoltage(uint8_t pack, uint8_t index);
uint16_t getVoltage_addr(uint8_t addr, uint8_t index);
void setCellTemp(uint8_t pack, uint8_t index, uint16_t raw_temp);
double getCellTemp(uint8_t pack, uint8_t index);

float32 rawToCelcius(uint16_t raw);

void update_spi_errors(uint8_t addr, uint8_t result);
uint8_t bat_health_check();

#endif
/* [] END OF FILE */
