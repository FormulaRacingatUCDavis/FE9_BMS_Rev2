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

extern volatile VCU_STATE vcu_state; 
extern volatile VCU_ERROR vcu_error; 
extern volatile uint8_t charger_attached; 

/* Data Frame format for Voltage and Temperature
The datatype consists of three bytes:
1. Identifying Number (eg cell #1)
2. upper byte of data
3. lower byte of data
*/

/* PCAN_SendMsgx() function associations
0. PCAN_SendMsg0() => Sends Temps
1. PCAN_SendMsg1() => Sends Voltage
2. PCAN_SendMsg2() => Sends Current
3. PCAN_SendMsg3() => Sends Status
4. PCAN_SendMsg4() => Sends SOC message(may not be used)
*/

/*
Temp CAN Message
ID: 0x488
Bytes: 
0: Pack 0 high temp
1: Pack 1 high temp
2: Pack 2 high temp
3: Pack 3 high temp
4: Pack 4 high temp
5: Pack 5 high temp
6: Subpack with highest temp
7: Highest temp
*/

void can_send_temp(volatile BAT_SUBPACK_t ** subpacks,
    volatile uint8_t high_temp_subpack,
    volatile uint8_t high_temp)
{
    for (unsigned int i = 0; i < 6; i++) {  //works up to 6 subpacks
        uint8_t temp = 0; 
        if(i < N_OF_SUBPACK){  //if subpack[i] exists
            temp = subpacks[i]->high_temp; 
        } 
        PCAN_TX_DATA_BYTE(PCAN_TX_MAILBOX_temp, i) = temp; 
    }    
    PCAN_TX_DATA_BYTE7(PCAN_TX_MAILBOX_temp) = 0xff & high_temp_subpack;
    PCAN_TX_DATA_BYTE8(PCAN_TX_MAILBOX_temp) = high_temp; //(high_temp/10)<<4 | (high_temp%10);

    //This works in the case that the number of subpacks is

	PCAN_SendMsgtemp(); // Sends Temps
    CyDelay(5);
} // can_send_temp() 


void can_send_volt(
    volatile uint16_t min_voltage,
    volatile uint16_t max_voltage,
    volatile uint32_t pack_voltage){
    //max and min voltage means the voltage of single cell
        PCAN_TX_DATA_BYTE1(PCAN_TX_MAILBOX_volt) = HI8(min_voltage);
        PCAN_TX_DATA_BYTE2(PCAN_TX_MAILBOX_volt) = LO8(min_voltage);

        PCAN_TX_DATA_BYTE3(PCAN_TX_MAILBOX_volt) = HI8(max_voltage);
        PCAN_TX_DATA_BYTE4(PCAN_TX_MAILBOX_volt) = LO8(max_voltage);

        PCAN_TX_DATA_BYTE5(PCAN_TX_MAILBOX_volt) = 0xFF & (pack_voltage >> 24);
        PCAN_TX_DATA_BYTE6(PCAN_TX_MAILBOX_volt) = 0xFF & (pack_voltage >> 16);
        PCAN_TX_DATA_BYTE7(PCAN_TX_MAILBOX_volt) = 0xFF & (pack_voltage >> 8);
        PCAN_TX_DATA_BYTE8(PCAN_TX_MAILBOX_volt) = 0xFF & (pack_voltage);


        PCAN_SendMsgvolt();  // Sends Voltage
        CyDelay(1);

} // can_send_volt()


void can_send_status(volatile uint8_t name,
                    volatile uint8_t SOC_P,
                    volatile uint16_t status,
                    volatile uint8_t stack,
                    volatile uint8_t cell,
                    volatile uint16_t value16){
//8 SOC Percent
//8 AH used since full charge
//16 BMS Status bits (error flags)
//16 Number of charge cycles
//16 Pack balance (delta) mV
    PCAN_TX_DATA_BYTE1(PCAN_TX_MAILBOX_status) = name;
    PCAN_TX_DATA_BYTE2(PCAN_TX_MAILBOX_status) = (uint8_t)(SOC_P/10)<<4 | (uint8_t)(SOC_P%10);
    PCAN_TX_DATA_BYTE3(PCAN_TX_MAILBOX_status) = HI8(status);
    PCAN_TX_DATA_BYTE4(PCAN_TX_MAILBOX_status) = LO8(status);
    PCAN_TX_DATA_BYTE5(PCAN_TX_MAILBOX_status) = stack & 0xFF;
    PCAN_TX_DATA_BYTE6(PCAN_TX_MAILBOX_status) = (cell) & 0xFF;
    PCAN_TX_DATA_BYTE7(PCAN_TX_MAILBOX_status) = HI8(value16);
    PCAN_TX_DATA_BYTE8(PCAN_TX_MAILBOX_status) = LO8(value16);

    PCAN_SendMsgstatus(); // Sends Status
}
                    
//IRQ handler for receiving current
//moves current into bat_pack struct
//handles state if charger is attached
void PCAN_ReceiveMsg_current_Callback(){
    CyGlobalIntDisable; 
    int16_t current = 0; 
    current |= (PCAN_RX_DATA_BYTE1(PCAN_RX_MAILBOX_current)<<8) & 0xFF00; 
    current |= PCAN_RX_DATA_BYTE2(PCAN_RX_MAILBOX_current) & 0xFF; 
    bat_pack.current = current; 
    
    //if charger is attached, keep track of LV/HV state from PEI feedback
    if(charger_attached){
        uint8_t shutdown_status = PCAN_RX_DATA_BYTE3(PCAN_RX_MAILBOX_current); 
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
        vcu_error = state & 0x0F; 
    } else {      //no fault
        vcu_state = state & 0x0F;
        vcu_error = NONE; 
    }
    
    charger_attached = 0; //make sure we don't think the charger is attached, eh? 
}

//IRQ handler for charger
//If charger is attached, LV/HV state will be determined from PEI message
void PCAN_ReceiveMsg_charger_Callback(){
    charger_attached = 1;   //simple as dat 
}

/*
void get_current(volatile BAT_PACK_t *bat_pack)
{
    bat_pack->current = current;
}

void RX_get_current(uint8_t *msg, int CAN_ID)
{
    uint8 InterruptState = CyEnterCriticalSection();
    
    if (CAN_ID == 0x069)
    {
        for (int i = 0; i < 8; i++) 
        {
            rx_can_buffer[i] = msg[i];
        }
        current += rx_can_buffer[0] << 8;
        current += rx_can_buffer[1];
    }
    
    
    CyExitCriticalSection(InterruptState);
    //return current;
}*/
                    
void can_init()
{
	PCAN_GlobalIntEnable(); // CAN Initialization
	PCAN_Start();
} // can_init(
/* [] END OF FILE */
