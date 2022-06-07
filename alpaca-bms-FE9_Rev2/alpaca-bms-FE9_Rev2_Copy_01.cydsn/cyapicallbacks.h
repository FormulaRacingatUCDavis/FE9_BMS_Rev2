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
    
//define these so that PCAN_TX_RX_func will call the IRQ handlers

#define PCAN_RECEIVE_MSG_charger_CALLBACK
    
extern void PCAN_ReceiveMsg_charger_Callback();

#endif /* CYAPICALLBACKS_H */   
/* [] */
