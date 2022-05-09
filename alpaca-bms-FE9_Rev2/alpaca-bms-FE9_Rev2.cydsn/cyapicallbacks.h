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
#ifndef CYAPICALLBACKS_H
#define CYAPICALLBACKS_H
    
//callbacks defined in can_manager
#include "can_manager.h"
    
//define these so that PCAN_TX_RX_func will call the IRQ handlers
#define PCAN_RECEIVE_MSG_current_CALLBACK
#define PCAN_RECEIVE_MSG_vehicle_state_CALLBACK
#define PCAN_RECEIVE_MSG_charger_CALLBACK

#endif /* CYAPICALLBACKS_H */   
/* [] */
