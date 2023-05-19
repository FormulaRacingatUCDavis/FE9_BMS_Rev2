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

#include "main.h"

volatile VCU_STATE vcu_state = LV;
volatile VCU_ERROR vcu_error = NONE; 
volatile uint8_t charger_attached = 0; 

//UNCOMMENT FOR CHARGING
//vcu_state = CHARGING;

int main(void){ 

    CyGlobalIntEnable; //Enable global interrupts. 
    init();   //initialize modules
    
    //Initialize state machine
    BMS_MODE bms_status = BMS_NORMAL;

    while(1){
        switch(bms_status) {
            case BMS_NORMAL:
                  
                OK_SIG_Write(1);   //Make sure OK signal is high
                
                if(vcu_state == CHARGING){
                    balance_cells();
                }
                
                //Update status
                bms_status = bat_health_check();
                
                break;
                
            case BMS_FAULT:  //fault state (no shit)
                
                OK_SIG_Write(0u);   //set OK signal to low to open shutdown circuit

                disable_cell_balancing();  //make sure cell balancing is not active
                bms_status = BMS_FAULT;  //make sure to stay in this state
                
                break;
                
            default:
                //shouldn't be here, must be broken
                bms_status = BMS_FAULT;
                break;
        }

        get_voltages();     //update voltages from packs
        check_voltages();   //parse voltages
        update_soc(); 
        get_temps();        //update temps
        can_tasks();
        
        //open_wire_check();
        
        CyWdtClear(); 
    }
}


void init(void){   //initialize modules
    FTDI_UART_Start();
    PIC18_UART_Start();
    can_init();
    cell_interface_init(); 
    pwm_init(); 
    set_pwm(); //set PWM values for fans & pump
    //PIC18_UART_RxISR_Start();
    
    CyWdtStart(CYWDT_1024_TICKS, CYWDT_LPMODE_NOCHANGE);
}

void can_tasks(){
    CyGlobalIntDisable;
    //can_send_status();
    PCAN_SendMsgcharger_bms();

    //dump BMS data over uart
    //send_uart_data();
    
    CyGlobalIntEnable;
}

/* [] END OF FILE */
