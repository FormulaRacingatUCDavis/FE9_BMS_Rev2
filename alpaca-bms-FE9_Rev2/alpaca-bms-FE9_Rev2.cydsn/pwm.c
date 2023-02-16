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
#include "pwm.h"
#include "can_manager.h"

extern volatile VCU_STATE vcu_state;
extern volatile BAT_PACK_t bat_pack;

void pwm_init(){
    RAD_PUMP_PWM_Start();
    ACC_PWM_Start();
}

void set_pwm(){
    //Set duty cycle for accumulator fan
    if(vcu_state == LV && bat_pack.HI_temp_c < 40){
        ACC_PWM_WriteCompare1(0);
    } else if (bat_pack.HI_temp_c < 30){
        ACC_PWM_WriteCompare1(0);
    }else if (bat_pack.HI_temp_c < 50){
        ACC_PWM_WriteCompare1(80);
    }else if(bat_pack.HI_temp_c > 50){
        ACC_PWM_WriteCompare1(150);
    }
    
    //set Duty cycle for the radiator fan
    //RAD_PUMP_PWM_WriteCompare1(128);
    
    //set Duty cycle for water pump
    RAD_PUMP_PWM_WriteCompare2(127);
}


/* [] END OF FILE */
