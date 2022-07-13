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

void pwm_init(){
    RAD_PUMP_PWM_Start();
    ACC_PWM_Start();
}

void set_pwm(){
    //Set duty cycle for accumulator fan
    ACC_PWM_WriteCompare1(193);
    
    //set Duty cycle for the radiator fan
    //RAD_PUMP_PWM_WriteCompare1(128);
    
    //set Duty cycle for water pump
    RAD_PUMP_PWM_WriteCompare2(127);
}


/* [] END OF FILE */
