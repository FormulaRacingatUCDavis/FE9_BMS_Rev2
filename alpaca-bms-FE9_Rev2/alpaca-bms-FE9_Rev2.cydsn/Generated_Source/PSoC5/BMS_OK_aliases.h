/*******************************************************************************
* File Name: BMS_OK.h  
* Version 2.20
*
* Description:
*  This file contains the Alias definitions for Per-Pin APIs in cypins.h. 
*  Information on using these APIs can be found in the System Reference Guide.
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_BMS_OK_ALIASES_H) /* Pins BMS_OK_ALIASES_H */
#define CY_PINS_BMS_OK_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"


/***************************************
*              Constants        
***************************************/
#define BMS_OK_0			(BMS_OK__0__PC)
#define BMS_OK_0_INTR	((uint16)((uint16)0x0001u << BMS_OK__0__SHIFT))

#define BMS_OK_INTR_ALL	 ((uint16)(BMS_OK_0_INTR))

#endif /* End Pins BMS_OK_ALIASES_H */


/* [] END OF FILE */
