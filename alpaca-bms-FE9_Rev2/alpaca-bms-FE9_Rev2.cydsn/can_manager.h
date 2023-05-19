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

#include "data.h"
#include "LTC6811.h"
#include "cell_interface.h"
#include "project.h"
    
typedef enum {
    LV,
    PRECHARGING,
    HV_ENABLED,
    DRIVE,
    VCU_FAULT,
    CHARGING, 
    CHARGER_FAULT
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

#define CAN_TIMEOUT_LOOP_COUNT 30

/* When cleaning and rebuilding project, change PCAN_TXn_FUNC_ENABLE
   for n in range [0, 4] to (1u) in lines 78 - 82 in file PCAN.h
*/    

void can_send_status();

void PCAN_ReceiveMsg_pei_current_Callback();
void PCAN_ReceiveMsg_vehicle_state_Callback(); 
void PCAN_ReceiveMsg_charger_status_Callback(); 

void check_vcu_charger();
void PCAN_toggle_baud();
void PCAN_to_125KB();
void PCAN_to_500KB();

/*
// Function called from main to set current
void get_current(volatile BAT_PACK_t *bat_pack);
// Function called from PCAN_ReceiveMsg() at line 624 in file PCAN_TX_RX_func.c
void RX_get_current(uint8_t *msg, int CAN_ID);
*/
void can_init();

#endif // CAN_MANAGER_H
/* [] END OF FILE */
