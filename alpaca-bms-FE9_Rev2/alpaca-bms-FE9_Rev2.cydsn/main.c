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
#include "FTDI.h"


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

// mimiced in charger code
// so master board can send SOC back and forth
// 240 values taken from data sheet
// used to approximate state of charge
int SOC_LUT[240] =  {
    0, 5, 13, 22, 31, 39,
    48, 57, 67, 76, 86, 
    90, 106, 117, 127, 138,
    150, 162, 174, 186, 199,
    212, 226, 241, 256, 271, 
    288, 205, 324, 345, 364,
    386, 410, 436, 465, 497,
    534, 577, 629, 695, 780,
    881, 972, 1044, 1103, 1157,
    1206, 1253, 1299, 1344, 1389, 
    1434, 1479, 1527, 1576, 1628, 
    1682, 1738, 1798, 1859, 1924,
    1992, 2062, 2134, 2208, 2281, 
    2424, 2492, 2557, 2620, 2681,
    2743, 2804, 2868, 2903, 3003,
    3078, 3161, 3253, 3354, 3467, 
    3589, 3720, 3851, 3976, 4092, 
    4200, 4303, 4404, 4504, 4603,
    4700, 4792, 4878, 4958, 5032,
    5101, 5166, 5228, 5289, 5347,
    5405, 5462, 5518, 5573, 5628,
    5680, 5731, 5780, 5826, 5869,
    5911, 5951, 5988, 6024, 6059,
    6092, 6124, 6156, 6187, 6217, 
    6247, 6278, 6308, 6337, 6368,
    6398, 6428, 6459, 6491, 6523,
    6556, 6590, 6625, 6660, 6696,
    6733, 6770, 6808, 6846, 6884,
    6923, 6961, 7000, 7039, 7077,
    7115, 7153, 7191, 7228, 7266, 
    7303, 7340, 7376, 7413, 7449,
    7484, 7520, 7555, 7590, 7625,
    7659, 7694, 7728, 7762, 7796,
    7830, 7864, 7898, 7932, 7966, 
    8000, 8034, 8068, 8102, 8136,
    8170, 8204, 8238, 8272, 8306,
    8340, 8373, 8406, 8440, 8472,
    8505, 8538, 8570, 8602, 8632,
    8666, 8697, 8729, 8760, 8791,
    8822, 8853, 8884, 8915, 8945, 
    8976, 9006, 9036, 9067, 9097, 
    9127, 9157, 9186, 9216, 9245,
    9275, 9304, 9333, 9362, 9390,
    9419, 9447, 9475, 9503, 9531,
    9559, 9586, 9613, 9640, 9647, 
    9693, 9720, 9746, 9772, 9797,
    9823, 9848, 9873, 9898, 9923,
    9947, 9971, 9995, 100000    
};

void init(void){   //initialize modules
<<<<<<< Updated upstream
    FTDI_UART_Start();
    PIC18_UART_Start();
=======
    //FTDI_UART_Start();
    //PIC18_UART_Start();

    //initialize CAN
>>>>>>> Stashed changes
    can_init();
    cell_interface_init(); 
    pwm_init(); 
    
    // Initialize the Kalman Filter variables
    //init_kalman();
}

void process_event(){
    CyGlobalIntDisable
<<<<<<< Updated upstream
    //CAN doesn't seem to work without delay. For FE10 can be reduced to one message instead of 3
    CyDelay(50);
    can_send_status(0);
    CyDelay(50); 
    can_send_volt();
    CyDelay(50);
    can_send_temp();
    
    //dump BMS data over uart
    send_uart_data();
    
=======
    //Old code: small delay(idk why)
    //CyDelay(50);
    can_send_status(bat_pack.HI_temp_c,
    	bat_pack.SOC_percent,
    	bat_pack.status,
    	bat_pack.HI_temp_board_c,
        0,0);
    CyDelay(50);
    // send voltage   
    //can_send_volt(bat_pack.LO_voltage, bat_pack.HI_voltage, bat_pack.voltage);
    //CyDelay(50);
    
    // TEST_DAY_1
    //send temp only if within reasonable range from last temperature

    //TODO: rewrite this for dynamic number of subpacks? 
    /*can_send_temp(bat_pack.subpacks,
			bat_pack.HI_temp_subpack,
			bat_pack.HI_temp_c);
    */
    
    //TODO: current will be sent by PEI board
    CyDelay(50);

>>>>>>> Stashed changes
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
        //Added case for if fuse blown, redundant?
        case FUSE_BLOWN:
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
<<<<<<< Updated upstream

=======
   
    
    
>>>>>>> Stashed changes
    //In old code, the loop had a BMS_BOOTUP case, but we're doin it all before it
    //goes into the while loop. I am assuming that nothing in the initialization process
    //will throw a BMS_FAULT. If that is the case, we'll need to change it back to how
    //it was.
    //We can probably get away with no bootup. I did have some odd issues with code outside of the while loop, but we'll see. 

    
    CyWdtStart(CYWDT_1024_TICKS, CYWDT_LPMODE_NOCHANGE);
    //Even if something throws a fault outside of the while loop, cant we still set the mode to BMS_FAULT to the same effect?   
    
    CyGlobalIntEnable; //Enable global interrupts. 

    init();   //initialize modules
    
    set_pwm(); //set PWM values for fans & pump
    
    //Initialize state machine
    BMS_MODE bms_status = BMS_NORMAL;
    uint32_t system_interval = 0;
    
    uint8_t counter = 0; 
    uint8_t counter2 = 0; 

    //volatile double prev_time_interval;

    while(1) {
        /*while(PIC18_UART_GetRxBufferSize()){
            FTDI_UART_PutChar(PIC18_UART_ReadRxData());
        }
        
        PIC18_UART_PutString("Sup bitches\n");*/
        
        switch(bms_status) {
            case BMS_NORMAL:
                //Start timer to time normal state
                //Timer_1_Start();

                //UNCOMMENT FOR CHARGING
                //vcu_state = CHARGING;
            
                //Make sure OK signal is high
                OK_SIG_Write(1);

                 
                if(counter2 < 30){
                    disable_cell_balancing();
                    set_adc_mode(MD_FILTERED);  
                    get_voltages();     //update voltages from packs
                    check_voltages();   //parse voltages
                } else if (counter2 < 90){
                    balance_cells();
                } else {
                    disable_cell_balancing();
                }
                counter2++;
                if (counter2 > 100){
                    counter2 = 0; 
                }
                
                
                //open_wire_check(); 
                
                bat_pack.SOC_percent = SOC_LUT[((uint32_t)bat_pack.LO_voltage * 28 - 934000) / 1000] / 100;
                
                //Balancing should only be done when charger is attached and HV is enabled
                //This corresponds to vcu_state == CHARGING (see can_manager.c)
                //should not be balancing if a fault exists
                
                
                /*if(bat_pack.HI_temp_board_c > 85){
                    vcu_state = LV; 
                } else if (bat_pack.HI_temp_board_c < 60) {
                    vcu_state = CHARGING; 
                }*/
                
                //if(vcu_state == CHARGING){
                    //balance_cells();
                //} else {
                    //disable_cell_balancing();  //this should be redundant
                //}
          
                /*if(counter < 15){
                    counter++; 
                } else {
                    disable_cell_balancing(); 
                    for(counter = 0; counter < 75; counter++){
                        CyDelay(1000); 
                    }
                    counter = 0; 
                }*/

                //Set lower accuracy (higher speed) for temp measurement
                set_adc_mode(MD_NORMAL); 
                if(counter < TEMP_LOOP_DIVISION){
                    get_temps(counter*TEMPS_PER_LOOP, (counter + 1)*TEMPS_PER_LOOP);     //update temperatures from packs
                    counter++;
                } else {
                    get_temps(counter*TEMPS_PER_LOOP, TEMPS_PER_LTC);
                    sort_temps();    //sort temps in to bat pack
                    check_temps();   //parse temps
                    counter = 0; 
                }
                
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
                
                //make sure cell balancing is not active
                disable_cell_balancing(); 
                
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
        set_pwm();
        process_event();
        CyWdtClear(); 
        //debug_balance(); 
        CyDelay(system_interval);
    }
}

/* [] END OF FILE */
