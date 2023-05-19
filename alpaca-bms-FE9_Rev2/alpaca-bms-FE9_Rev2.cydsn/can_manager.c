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

#include "can_manager.h"

extern BAT_PACK_t bat_pack;
extern BAT_CELL_t bat_cell[N_OF_CELL]; 

extern volatile VCU_STATE vcu_state; 
extern volatile VCU_ERROR vcu_error; 
extern volatile uint8_t charger_attached;
extern volatile uint8_t vcu_attached;
extern volatile uint8_t loop_counter;
volatile uint8_t baud = 0;


void can_send_status(){
//8 SOC Percent
//8 SOC Percent
//16 BMS Status bits (error flags)
    PCAN_TX_DATA_BYTE1(PCAN_TX_MAILBOX_status) = bat_pack.HI_temp_c;
    PCAN_TX_DATA_BYTE2(PCAN_TX_MAILBOX_status) = (uint8_t)(bat_pack.SOC_percent/10)<<4 | (uint8_t)(bat_pack.SOC_percent%10);
    PCAN_TX_DATA_BYTE3(PCAN_TX_MAILBOX_status) = HI8(bat_pack.status);
    PCAN_TX_DATA_BYTE4(PCAN_TX_MAILBOX_status) = LO8(bat_pack.status);
    PCAN_SendMsgstatus(); // Sends Status
    
}
                    
//IRQ handler for receiving current
//moves current into bat_pack struct
//handles state if charger is attached
void PCAN_ReceiveMsg_pei_current_Callback(){
    CyGlobalIntDisable; 

    int16_t current = 0; 
    current |= (PCAN_RX_DATA_BYTE1(PCAN_RX_MAILBOX_pei_current)<<8) & 0xFF00; 
    current |= PCAN_RX_DATA_BYTE2(PCAN_RX_MAILBOX_pei_current) & 0xFF; 
    bat_pack.current = current; 
    
    //if charger is attached, keep track of LV/HV state from PEI feedback
    if(charger_attached){
        uint8_t shutdown_status = PCAN_RX_DATA_BYTE3(PCAN_RX_MAILBOX_pei_current); 
        /*
        HV is enabled when shutdown_final is high and AIRs are low.
        
         bits -  |7     |6       |5      |4              |3               |2     |1     |0           |
                 |x     |x       |x      |Ready to drive |Shutdown final  |AIR1  |AIR2  |Precharge   |
                 |x     |x       |x      |x              |1               |0     |0     |x           |
        */
        if ((shutdown_status & 0b00001110) == 0b00001000){
            vcu_state = CHARGING; 
        } else {
            vcu_state = LV; 
        }
            
    }
    CyGlobalIntEnable; 
}

//IRQ handler for receiving VCU state
//moves state 
void PCAN_ReceiveMsg_vehicle_state_Callback(){
    uint8_t state = PCAN_RX_DATA_BYTE1(PCAN_RX_MAILBOX_vehicle_state); 
    
    if(state & 0x80){  //check fault bit
        vcu_state = VCU_FAULT; 
        vcu_error = state & 0x7F; 
    } else {      //no fault
        vcu_state = state & 0x7F;
        vcu_error = NONE; 
    }
    
    vcu_attached = 1;
    charger_attached = 0; //make sure we don't think the charger is attached, eh? 
}

//IRQ handler for charger
//If charger is attached, LV/HV state will be determined from PEI message
void PCAN_ReceiveMsg_charger_status_Callback(){
    charger_attached = 1;    
    vcu_attached = 0;
    
    uint8_t charger_status = PCAN_RX_DATA_BYTE5(PCAN_RX_MAILBOX_charger_status);
    if((charger_status & 0b1111) == 0){  //no faults, charger active
        vcu_state = CHARGING;
    } else if((charger_status & 0b1011) == 0){  //no faults, charger inactive
        vcu_state = LV;
    } else {
        vcu_state = CHARGER_FAULT;  //faults
    }
}

//checks to see if a can timout has occured and toggles can bus if so
void check_vcu_charger(){
    if(charger_attached || vcu_attached){
        loop_counter = 0;
        return;
    }
    
    if(loop_counter > CAN_TIMEOUT_LOOP_COUNT){
        PCAN_toggle_baud();
    }
}
        

//switches PCAN between 125kb/s (vehicle) and 500kb/s (charger)
void PCAN_toggle_baud(){
    if(baud == 0){
        PCAN_to_500KB();
        baud = 1;
    } else {
        PCAN_to_125KB();
        baud = 0;
    }
}

void PCAN_to_125KB(){
    PCAN_SetPreScaler(11);
}

void PCAN_to_500KB(){
    PCAN_SetPreScaler(2);
}
                    
void can_init(){
	PCAN_GlobalIntEnable(); // CAN Initialization
	PCAN_Start();
} // can_init(
/* [] END OF FILE */
