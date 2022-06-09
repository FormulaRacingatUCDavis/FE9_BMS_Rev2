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
#include "cyapicallbacks.h"

#define PCAN_RECEIVE_MSG_charger_CALLBACK

void PCAN_ReceiveMsg_charger_Callback(){
    OK_SIG_Write(0); 
}

int main(void)
{  //SEE ADOW ON DATASHEET PAGE 33

    //In old code, the loop had a BMS_BOOTUP case, but we're doin it all before it
    //goes into the while loop. I am assuming that nothing in the initialization process
    //will throw a BMS_FAULT. If that is the case, we'll need to change it back to how
    //it was.
    //We can probably get away with no bootup. I did have some odd issues with code outside of the while loop, but we'll see. 
    //Even if something throws a fault outside of the while loop, cant we still set the mode to BMS_FAULT to the same effect? 
    

    CyGlobalIntEnable; //Enable global interrupts. 

    PCAN_Start();   //initialize modules
    
    OK_SIG_Write(0);
    CyDelay(500); 
    OK_SIG_Write(1); 

    while(1) {
        
    }
}

/* [] END OF FILE */
