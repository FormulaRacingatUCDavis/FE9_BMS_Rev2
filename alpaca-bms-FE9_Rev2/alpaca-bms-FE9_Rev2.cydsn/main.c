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
//includes for current data and can manager
//#include "currrent_sense.h"
//#include "can_manager.h"
#include <LTC6811.h>

#include "math.h"
#include <time.h>

//The old code had many more BMS modes, are we ever going to need that?
typedef enum {
    BMS_NORMAL, 
    BMS_FAULT
}BMS_MODE;

void init(void){   //initialize modules
    SPI_Start();  
    //USB_Start(0, USB_5V_OPERATION);
    //USB_CDC_Init();
    //PCAN_Start();
    can_init();
    bms_init(MD_NORMAL); 
    mypack_init();
}

void process_event(){
    CyGlobalIntDisable
    //Old code: small delay(idk why)
    CyDelay(10);
    can_send_status(0xFE,
    	bat_pack.SOC_percent,
    	bat_pack.status,
    	0,0,0);
    CyDelay(10);
    // send voltage   
    can_send_volt(bat_pack.LO_voltage,
				bat_pack.HI_voltage,
				bat_pack.voltage);
    CyDelay(10);
    
    // TEST_DAY_1
    //send temp only if within reasonable range from last temperature

    can_send_temp(bat_pack.subpacks[0]->high_temp,
			bat_pack.subpacks[1]->high_temp,
            bat_pack.subpacks[2]->high_temp,
            bat_pack.subpacks[3]->high_temp,
            bat_pack.subpacks[4]->high_temp,
            //bat_pack.subpacks[5]->high_temp,
            bat_pack.HI_temp_node_index,
			bat_pack.HI_temp_node,
			bat_pack.HI_temp_c);
    
    can_send_volt(bat_pack.LO_voltage, bat_pack.HI_voltage, bat_pack.voltage);
    // send current
    //can_send_current(bat_pack.current);
    CyDelay(10);

    CyGlobalIntEnable;
}

int main(void)
{  //SEE ADOW ON DATASHEET PAGE 33

    //In old code, the loop had a BMS_BOOTUP case, but we're doin it all before it
    //goes into the while loop. I am assuming that nothing in the initialization process
    //will throw a BMS_FAULT. If that is the case, we'll need to change it back to how
    //it was.

    CyGlobalIntEnable; /* Enable global interrupts. */

    init();   //initialize modules
    
    LTC6811_initialize(MD_FAST);
    LTC6811_init_cfg();

    uint8_t cfga[6];
    uint16_t aux;
    volatile uint16_t volts[16];
    volatile uint16_t i = 0; 
    
    const char8 str[] = "Bitch"; 
    
    uint16_t cell_voltages[2][12];

    BMS_MODE bms_status = BMS_NORMAL;

    while(1) {
        switch(bms_status) {
            case BMS_NORMAL:
                //Start timer to time normal state
                Timer_1_Start();

                //Flowchart: "Tells board to keep HV path closed (CAN)"
                //Not quite sure wha that means.
                OK_SIG_Write(1);

                //Apparantly the filtered version gives more precision for measurements
                bms_init(MD_FILTERED);
                get_voltages();
                get_current(); //get_current used to be under bms_init(MD_NORMAL) but it seemed that
                               //the precision was necessary for SOC estimation
                //Not quite sure why it set it as normal, does it waste time when we leave it as MD_FILTERED?
                bms_init(MD_NORMAL); 
                get_all_temps();
                //SOC_estimation();
                bms_status = bat_health_check();

                //Calculating time spent in state
                //Old code said "do these time tests ONE FILE AT A TIME due to the hardcoded variable"
                //Not sure what that quite means. Also it seems like we didn't really use the time at all, so 
                //we can maybe send it over to the dashboard through UART?
                Timer_1_Stop();
                uint32 time_left = Timer_1_ReadCounter();
                double time_spent = time_spent_start - (double)time_left;
                time_spent_start = time_left;
                double time_spent_seconds = (double)(time_spent) / (double)(24000000); //gives time in seconds
                
                break;
            case BMS_FAULT:
                OK_SIG_Write(0u);
                bms_status = BMS_FAULT;
                system_interval = 500;
                process_failure();
                break;
            default:
                bms_status = BMS_FAULT;
                break;
        }
        process_event();
        CyDelay(system_intercal);
    }
}

/* [] END OF FILE */
