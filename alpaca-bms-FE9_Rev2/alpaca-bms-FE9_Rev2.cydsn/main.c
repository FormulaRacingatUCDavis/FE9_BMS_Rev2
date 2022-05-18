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

#include "project.h"
#include "cell_interface.h"
#include "can_manager.h"
#include "LTC6811.h"
#include "math.h"
#include "time.h"
#include "pwm.h"
#include "cyapicallbacks.h"

//The old code had many more BMS modes, are we ever going to need that?
//Need BMS_CHARGING at least. We only want to balance cells during charging. 
//BMS_CHARGING will share a lot with BMS_NORMAL, so maybe charging shouldn't be its own mode
typedef enum {
    BMS_NORMAL, 
    BMS_FAULT
}BMS_MODE;

extern volatile BAT_PACK_t bat_pack;
extern BAT_SUBPACK_t bat_subpack[N_OF_SUBPACK];
extern volatile float32 sortedTemps[N_OF_TEMP_CELL]; 
extern volatile BAT_ERR_t* bat_err_array;
extern volatile uint8_t bat_err_index;
extern volatile uint8_t bat_err_index_loop;

volatile VCU_STATE vcu_state = LV;
volatile VCU_ERROR vcu_error = NONE; 
volatile uint8_t charger_attached = 0;  

void init(void){   //initialize modules
    //FTDI_UART_Start();
    //PIC18_UART_Start();
    //USB_Start(0, USB_5V_OPERATION);
    //USB_CDC_Init();

    //initialize CAN
    can_init();
    
    //initialize cell interface
    cell_interface_init(); 
    
    //Initialize and enable radiator fan, water pump, and accumulator fan PWM modules
    pwm_init(); 
}

void process_event(){
    CyGlobalIntDisable
    //Old code: small delay(idk why)
    CyDelay(50);
    can_send_status(0xFE,
    	bat_pack.SOC_percent,
    	bat_pack.status,
    	0,0,0);
    CyDelay(50);
    // send voltage   
    can_send_volt(bat_pack.LO_voltage, bat_pack.HI_voltage, bat_pack.voltage);
    CyDelay(50);
    
    // TEST_DAY_1
    //send temp only if within reasonable range from last temperature

    //TODO: rewrite this for dynamic number of subpacks? 
    can_send_temp(bat_pack.subpacks,
			bat_pack.HI_temp_subpack,
			bat_pack.HI_temp_c);
    
    
    //TODO: current will be sent by PEI board
    CyDelay(50);

    CyGlobalIntEnable;
}

void process_failure_helper(BAT_ERR_t err){
	switch(err.err){
		case CELL_VOLT_OVER:
        	can_send_volt(((err.bad_node<<8) | err.bad_cell),
			    bat_subpack[err.bad_node].cells[err.bad_cell]->voltage, bat_pack.voltage);
		case CELL_VOLT_UNDER:
			can_send_volt(((err.bad_node<<8) | err.bad_cell),
				bat_subpack[err.bad_node].cells[err.bad_cell]->voltage, bat_pack.voltage);
			break;
		case PACK_TEMP_OVER:
		case PACK_TEMP_UNDER:
			// waiting for CAN mesg been defined clearly
			break;

	}
	return;
}

// Copied from old code
void process_failure(){
	int8_t i=0;
	// broadcast error in inverse chronological order
	if (bat_err_index_loop){
		// start from bat_err_index back to 0
		for (i=bat_err_index;i>=0;i--){
			process_failure_helper(bat_err_array[i]);
		}
		// start from index=99 to bat_err_index+1
		for (i=99;i>bat_err_index;i--){
			process_failure_helper(bat_err_array[i]);
		}
	}else{
		// start from bat_err_index back to 0
		for (i=bat_err_index;i>=0;i--){
			process_failure_helper(bat_err_array[i]);
		}
	}
}
// End of copy


int main(void)
{  //SEE ADOW ON DATASHEET PAGE 33

    //In old code, the loop had a BMS_BOOTUP case, but we're doin it all before it
    //goes into the while loop. I am assuming that nothing in the initialization process
    //will throw a BMS_FAULT. If that is the case, we'll need to change it back to how
    //it was.
    //We can probably get away with no bootup. I did have some odd issues with code outside of the while loop, but we'll see. 
    //Even if something throws a fault outside of the while loop, cant we still set the mode to BMS_FAULT to the same effect? 
    

    CyGlobalIntEnable; //Enable global interrupts. 

    init();   //initialize modules
    
    set_pwm(); //set PWM values for fans & pump
    
    //Initialize state machine
    BMS_MODE bms_status = BMS_NORMAL;
    uint32_t system_interval = 0;

    volatile double prev_time_interval;

    while(1) {
        switch(bms_status) {
            case BMS_NORMAL:
                //Start timer to time normal state
                //Timer_1_Start();

                //Make sure OK signal is high
                OK_SIG_Write(1);

                //Set higher acuracy for voltages
                set_adc_mode(MD_FILTERED);
                get_voltages();     //update voltages from packs
                check_voltages();   //parse voltages
                
                //Balancing should only be done when charger is attached and HV is enabled
                //This corresponds to vcu_state == CHARGING (see can_manager.c)
                if(vcu_state == CHARGING){
                    balance_cells();
                } else {
                    disable_cell_balancing();  //this should be redundant
                }

                //Set lower accuracy (higher speed) for temp measurement
                set_adc_mode(MD_NORMAL); 
                get_temps();     //update temperatures from packs
                check_temps();   //parse temps
                
		        //double SOC;
                //SOC = SOC_estimation(double prev_time_interval, voltage, current);
                
                //Update status
                bms_status = bat_health_check();

                //Calculating time spent in state
                //Old code said "do these time tests ONE FILE AT A TIME due to the hardcoded variable"
                //Not sure what that quite means. Also it seems like we didn't really use the time at all, so 
                //we can maybe send it over to the dashboard through UART?
                //I think time was used to estimate an integral for coulomb counting. Unless the Kalmann filter needs time, we can get rid of it. 
                //Timer_1_Stop();
                //uint32 time_spent_cycle = Timer_1_ReadCounter();
                //prev_time_interval = (double)(time_spent_cycle) / (double)(24000000); //gives time in seconds
                
                break;
                
            case BMS_FAULT:  //fault state (no shit)
                
                //set OK signal to low to open shutdown circuit
                OK_SIG_Write(0u);
                
                //make sure to stay in this state
                bms_status = BMS_FAULT;
                
                //will send error code every 500ms
                system_interval = 500;
                
                //send error codes over CAN
                process_failure();
                
                break;
            default:
                //shouldn't be here, must be broken
                bms_status = BMS_FAULT;
                break;
        }
        process_event();
        CyDelay(system_interval);
    }
}

/* [] END OF FILE */
