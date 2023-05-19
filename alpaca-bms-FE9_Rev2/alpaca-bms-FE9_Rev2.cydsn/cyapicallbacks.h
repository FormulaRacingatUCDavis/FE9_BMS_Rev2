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
#define PCAN_RECEIVE_MSG_pei_current_CALLBACK
#define PCAN_RECEIVE_MSG_vehicle_state_CALLBACK
#define PCAN_RECEIVE_MSG_charger_CALLBACK
// Defined to enable interrupts for Kalman Filter
#define KalmanFilt_Int_INTERRUPT_INTERRUPT_CALLBACK
    
extern void PCAN_ReceiveMsg_pei_current_Callback();
extern void PCAN_ReceiveMsg_vehicle_state_Callback(); 
extern void PCAN_ReceiveMsg_charger_status_Callback();
// Defined to enable interrupts for Kalman Filter
extern void KalmanFilt_Int_Interrupt_InterruptCallback();

#endif /* CYAPICALLBACKS_H */   
/* [] */
