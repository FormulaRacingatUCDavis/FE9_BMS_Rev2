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
#ifndef CAN_MANAGER_H
#define CAN_MANAGER_H

#include <project.h>
#include "data.h"
#include "LTC6811.h"
#include "cell_interface.h"
    
typedef enum {
    LV,
    PRECHARGING,
    HV_ENABLED,
    DRIVE,
    VCU_FAULT, 
    CHARGING
} VCU_STATE; 

typedef enum {
    NONE,
    DRIVE_REQUEST_FROM_LV,
    CONSERVATIVE_TIMER_MAXED,
    BRAKE_NOT_PRESSED,
    HV_DISABLED_WHILE_DRIVING,
    SENSOR_DISCREPANCY,
    BRAKE_IMPLAUSIBLE
} VCU_ERROR;

/* When cleaning and rebuilding project, change PCAN_TXn_FUNC_ENABLE
   for n in range [0, 4] to (1u) in lines 78 - 82 in file PCAN.h
*/    

void can_send_temp(volatile BAT_SUBPACK_t **,
    uint8_t high_tempNode,
    uint8_t high_temp);

void can_send_volt(
    uint16_t min_voltage,
    uint16_t max_voltage,
    uint32_t pack_voltage);

//void can_send_current(int16_t battery_current) is now obsolete

void can_send_status(uint8_t name,
                    uint8_t SOC_P,
                    uint16_t status,
                    uint8_t stack,
                    uint8_t cell,
                    uint16_t value16);

void PCAN_ReceiveMsg_current_Callback();
void PCAN_ReceiveMsg_vehicle_state_Callback(); 
void PCAN_ReceiveMsg_charger_Callback(); 

/*
// Function called from main to set current
void get_current(volatile BAT_PACK_t *bat_pack);
// Function called from PCAN_ReceiveMsg() at line 624 in file PCAN_TX_RX_func.c
void RX_get_current(uint8_t *msg, int CAN_ID);
*/
void can_init();

#endif // CAN_MANAGER_H
/* [] END OF FILE */
