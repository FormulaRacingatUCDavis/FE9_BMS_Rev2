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
#include <stdio.h>
#include "main.h"
#include "SOC.h"

volatile VCU_STATE vcu_state = LV;
volatile VCU_ERROR vcu_error = NONE; 
volatile uint8_t charger_attached = 0; 
volatile uint8_t vcu_attached = 0;
volatile uint32_t loop_counter = 0;

uint8 errorStatus = 0;

volatile uint8_t SOC = 0;

int main(void){ 

    CyGlobalIntEnable; //Enable global interrupts. 
    
    PCAN_Init();
   
    //volatile uint8_t result = PCAN_SetTsegSample(13, 2, 2, PCAN_ONE_SAMPLE_POINT);
    

    init();   //initialize modules
    
    //Initialize state machine
    BMS_MODE bms_status = BMS_NORMAL;
    
    //while(1) {
    //    uint8 rxStatus;         
    //        uint8 rxData;           
    //        
    //        /* Read status register. */
    //        rxStatus = PIC18_UART_ReadRxStatus();
    //        
    //        /* Check if data is received. */
    //        if((rxStatus & PIC18_UART_RX_STS_FIFO_NOTEMPTY) != 0u)    
    //        {
    //            /* Read received data */
    //            rxData = PIC18_UART_ReadRxData();
    //
    //            /* Check status on error*/
    //            if((rxStatus & (PIC18_UART_RX_STS_BREAK      | PIC18_UART_RX_STS_PAR_ERROR |
    //                            PIC18_UART_RX_STS_STOP_ERROR | PIC18_UART_RX_STS_OVERRUN)) != 0u)
    //            {
    //                errorStatus |= rxStatus & ( PIC18_UART_RX_STS_BREAK      | PIC18_UART_RX_STS_PAR_ERROR | 
    //                                            PIC18_UART_RX_STS_STOP_ERROR | PIC18_UART_RX_STS_OVERRUN);
    //            }
    //            else
    //            {
    //                /* Send data backward */
    //                char str[10];
    //                sprintf(str, "%d", rxData);
    //                FTDI_UART_PutString(str);
    //            }
    //        }
    //}

    while(1){
        
        get_voltages();     //update voltages from packs
        check_voltages();   //parse voltages
        //update_soc(); 
        get_temps();        //update temps
        set_pwm();
        
        update_SOC_input(bat_pack.voltage, bat_pack.current); // sus?
        
        switch(bms_status) {                
            case BMS_NORMAL:    
                OK_SIG_Write(1); 
                bms_status = bat_health_check();
                
                if(vcu_state == CHARGING){
                    balance_cells();
                } else {
                    disable_cell_balancing();
                }
                
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
        
        
        // -------------------- SOC ----------------------
        // FUNCTION TO CALCULATE xhatCorrected & PCorrected
        EKF(&xhatk_1, &Pk_1, I,  V, Rk, &Aprime, &Cprime, &Eprime,
            &fk_, &Qk1,
            &Aprime_transpose, &Eprime_transpose, &sub_mat_1, &sub_mat_2,
            &sub_mat_3, &xhat, &P, &Cprime_transpose, &Lk, &xhatCorrected,
            &PCorrected);


        // Setting the xhatk_1 and Pk_1 values
        for (i = 1; i <= 2; i++) {
            // set values of xhatk_1 to xhatCorrected
            mat_set(i, 1, mat_get(i, 1, &xhatCorrected), &xhatk_1);

            // For each column of second matrix
            for (j = 1; j <= 2; j++) {
                // set values of Pk_1 to PCorrected
                mat_set(i, j, mat_get(i, j, &PCorrected), &Pk_1);
            }
        }
        
        SOC = (uint8_t)(mat_get(1, 1, &xhatCorrected) * 100);
        // Send over USB
        char SOC_str[10];
        sprintf(SOC_str, "%d", SOC);   
        FTDI_UART_PutString(SOC_str);
        // TODO: Send to dashboard (see can_manager.c)

        // -------------------- SOC ----------------------
        
        can_tasks(); 
        CyWdtClear(); 
        
        loop_counter++;
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
    init_SOC_vars();
    
    CyWdtStart(CYWDT_1024_TICKS, CYWDT_LPMODE_NOCHANGE);
}

void can_tasks(){
    CyGlobalIntDisable;
    can_send_status();
    //check_vcu_charger();

    //dump BMS data over uart
    // TODO: fix formatting then uncomment
    //send_uart_data();
    
    //PIC18_UART_PutChar(0xAA);
    
    //send_soc_data();
    update_soc();
    
    CyGlobalIntEnable;
}

/* [] END OF FILE */
