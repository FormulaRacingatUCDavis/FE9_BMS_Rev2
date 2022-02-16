/*******************************************************************************
* File Name: BMS_OK_PM.c
* Version 1.80
*
* Description:
*  This file contains the setup, control, and status commands to support 
*  the component operation in the low power mode. 
*
* Note:
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#include "BMS_OK.h"

/* Check for removal by optimization */
#if !defined(BMS_OK_Sync_ctrl_reg__REMOVED)

static BMS_OK_BACKUP_STRUCT  BMS_OK_backup = {0u};

    
/*******************************************************************************
* Function Name: BMS_OK_SaveConfig
********************************************************************************
*
* Summary:
*  Saves the control register value.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void BMS_OK_SaveConfig(void) 
{
    BMS_OK_backup.controlState = BMS_OK_Control;
}


/*******************************************************************************
* Function Name: BMS_OK_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores the control register value.
*
* Parameters:
*  None
*
* Return:
*  None
*
*
*******************************************************************************/
void BMS_OK_RestoreConfig(void) 
{
     BMS_OK_Control = BMS_OK_backup.controlState;
}


/*******************************************************************************
* Function Name: BMS_OK_Sleep
********************************************************************************
*
* Summary:
*  Prepares the component for entering the low power mode.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void BMS_OK_Sleep(void) 
{
    BMS_OK_SaveConfig();
}


/*******************************************************************************
* Function Name: BMS_OK_Wakeup
********************************************************************************
*
* Summary:
*  Restores the component after waking up from the low power mode.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void BMS_OK_Wakeup(void)  
{
    BMS_OK_RestoreConfig();
}

#endif /* End check for removal by optimization */


/* [] END OF FILE */
