/*******************************************************************************
* File Name: BMS_OK.h  
* Version 2.20
*
* Description:
*  This file contains Pin function prototypes and register defines
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_BMS_OK_H) /* Pins BMS_OK_H */
#define CY_PINS_BMS_OK_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "BMS_OK_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 BMS_OK__PORT == 15 && ((BMS_OK__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    BMS_OK_Write(uint8 value);
void    BMS_OK_SetDriveMode(uint8 mode);
uint8   BMS_OK_ReadDataReg(void);
uint8   BMS_OK_Read(void);
void    BMS_OK_SetInterruptMode(uint16 position, uint16 mode);
uint8   BMS_OK_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the BMS_OK_SetDriveMode() function.
     *  @{
     */
        #define BMS_OK_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define BMS_OK_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define BMS_OK_DM_RES_UP          PIN_DM_RES_UP
        #define BMS_OK_DM_RES_DWN         PIN_DM_RES_DWN
        #define BMS_OK_DM_OD_LO           PIN_DM_OD_LO
        #define BMS_OK_DM_OD_HI           PIN_DM_OD_HI
        #define BMS_OK_DM_STRONG          PIN_DM_STRONG
        #define BMS_OK_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define BMS_OK_MASK               BMS_OK__MASK
#define BMS_OK_SHIFT              BMS_OK__SHIFT
#define BMS_OK_WIDTH              1u

/* Interrupt constants */
#if defined(BMS_OK__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in BMS_OK_SetInterruptMode() function.
     *  @{
     */
        #define BMS_OK_INTR_NONE      (uint16)(0x0000u)
        #define BMS_OK_INTR_RISING    (uint16)(0x0001u)
        #define BMS_OK_INTR_FALLING   (uint16)(0x0002u)
        #define BMS_OK_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define BMS_OK_INTR_MASK      (0x01u) 
#endif /* (BMS_OK__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define BMS_OK_PS                     (* (reg8 *) BMS_OK__PS)
/* Data Register */
#define BMS_OK_DR                     (* (reg8 *) BMS_OK__DR)
/* Port Number */
#define BMS_OK_PRT_NUM                (* (reg8 *) BMS_OK__PRT) 
/* Connect to Analog Globals */                                                  
#define BMS_OK_AG                     (* (reg8 *) BMS_OK__AG)                       
/* Analog MUX bux enable */
#define BMS_OK_AMUX                   (* (reg8 *) BMS_OK__AMUX) 
/* Bidirectional Enable */                                                        
#define BMS_OK_BIE                    (* (reg8 *) BMS_OK__BIE)
/* Bit-mask for Aliased Register Access */
#define BMS_OK_BIT_MASK               (* (reg8 *) BMS_OK__BIT_MASK)
/* Bypass Enable */
#define BMS_OK_BYP                    (* (reg8 *) BMS_OK__BYP)
/* Port wide control signals */                                                   
#define BMS_OK_CTL                    (* (reg8 *) BMS_OK__CTL)
/* Drive Modes */
#define BMS_OK_DM0                    (* (reg8 *) BMS_OK__DM0) 
#define BMS_OK_DM1                    (* (reg8 *) BMS_OK__DM1)
#define BMS_OK_DM2                    (* (reg8 *) BMS_OK__DM2) 
/* Input Buffer Disable Override */
#define BMS_OK_INP_DIS                (* (reg8 *) BMS_OK__INP_DIS)
/* LCD Common or Segment Drive */
#define BMS_OK_LCD_COM_SEG            (* (reg8 *) BMS_OK__LCD_COM_SEG)
/* Enable Segment LCD */
#define BMS_OK_LCD_EN                 (* (reg8 *) BMS_OK__LCD_EN)
/* Slew Rate Control */
#define BMS_OK_SLW                    (* (reg8 *) BMS_OK__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define BMS_OK_PRTDSI__CAPS_SEL       (* (reg8 *) BMS_OK__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define BMS_OK_PRTDSI__DBL_SYNC_IN    (* (reg8 *) BMS_OK__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define BMS_OK_PRTDSI__OE_SEL0        (* (reg8 *) BMS_OK__PRTDSI__OE_SEL0) 
#define BMS_OK_PRTDSI__OE_SEL1        (* (reg8 *) BMS_OK__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define BMS_OK_PRTDSI__OUT_SEL0       (* (reg8 *) BMS_OK__PRTDSI__OUT_SEL0) 
#define BMS_OK_PRTDSI__OUT_SEL1       (* (reg8 *) BMS_OK__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define BMS_OK_PRTDSI__SYNC_OUT       (* (reg8 *) BMS_OK__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(BMS_OK__SIO_CFG)
    #define BMS_OK_SIO_HYST_EN        (* (reg8 *) BMS_OK__SIO_HYST_EN)
    #define BMS_OK_SIO_REG_HIFREQ     (* (reg8 *) BMS_OK__SIO_REG_HIFREQ)
    #define BMS_OK_SIO_CFG            (* (reg8 *) BMS_OK__SIO_CFG)
    #define BMS_OK_SIO_DIFF           (* (reg8 *) BMS_OK__SIO_DIFF)
#endif /* (BMS_OK__SIO_CFG) */

/* Interrupt Registers */
#if defined(BMS_OK__INTSTAT)
    #define BMS_OK_INTSTAT            (* (reg8 *) BMS_OK__INTSTAT)
    #define BMS_OK_SNAP               (* (reg8 *) BMS_OK__SNAP)
    
	#define BMS_OK_0_INTTYPE_REG 		(* (reg8 *) BMS_OK__0__INTTYPE)
#endif /* (BMS_OK__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_BMS_OK_H */


/* [] END OF FILE */
