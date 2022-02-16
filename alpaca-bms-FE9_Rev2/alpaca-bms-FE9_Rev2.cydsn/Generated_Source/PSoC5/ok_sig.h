/*******************************************************************************
* File Name: ok_sig.h  
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

#if !defined(CY_PINS_ok_sig_H) /* Pins ok_sig_H */
#define CY_PINS_ok_sig_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "ok_sig_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 ok_sig__PORT == 15 && ((ok_sig__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    ok_sig_Write(uint8 value);
void    ok_sig_SetDriveMode(uint8 mode);
uint8   ok_sig_ReadDataReg(void);
uint8   ok_sig_Read(void);
void    ok_sig_SetInterruptMode(uint16 position, uint16 mode);
uint8   ok_sig_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the ok_sig_SetDriveMode() function.
     *  @{
     */
        #define ok_sig_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define ok_sig_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define ok_sig_DM_RES_UP          PIN_DM_RES_UP
        #define ok_sig_DM_RES_DWN         PIN_DM_RES_DWN
        #define ok_sig_DM_OD_LO           PIN_DM_OD_LO
        #define ok_sig_DM_OD_HI           PIN_DM_OD_HI
        #define ok_sig_DM_STRONG          PIN_DM_STRONG
        #define ok_sig_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define ok_sig_MASK               ok_sig__MASK
#define ok_sig_SHIFT              ok_sig__SHIFT
#define ok_sig_WIDTH              1u

/* Interrupt constants */
#if defined(ok_sig__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in ok_sig_SetInterruptMode() function.
     *  @{
     */
        #define ok_sig_INTR_NONE      (uint16)(0x0000u)
        #define ok_sig_INTR_RISING    (uint16)(0x0001u)
        #define ok_sig_INTR_FALLING   (uint16)(0x0002u)
        #define ok_sig_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define ok_sig_INTR_MASK      (0x01u) 
#endif /* (ok_sig__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define ok_sig_PS                     (* (reg8 *) ok_sig__PS)
/* Data Register */
#define ok_sig_DR                     (* (reg8 *) ok_sig__DR)
/* Port Number */
#define ok_sig_PRT_NUM                (* (reg8 *) ok_sig__PRT) 
/* Connect to Analog Globals */                                                  
#define ok_sig_AG                     (* (reg8 *) ok_sig__AG)                       
/* Analog MUX bux enable */
#define ok_sig_AMUX                   (* (reg8 *) ok_sig__AMUX) 
/* Bidirectional Enable */                                                        
#define ok_sig_BIE                    (* (reg8 *) ok_sig__BIE)
/* Bit-mask for Aliased Register Access */
#define ok_sig_BIT_MASK               (* (reg8 *) ok_sig__BIT_MASK)
/* Bypass Enable */
#define ok_sig_BYP                    (* (reg8 *) ok_sig__BYP)
/* Port wide control signals */                                                   
#define ok_sig_CTL                    (* (reg8 *) ok_sig__CTL)
/* Drive Modes */
#define ok_sig_DM0                    (* (reg8 *) ok_sig__DM0) 
#define ok_sig_DM1                    (* (reg8 *) ok_sig__DM1)
#define ok_sig_DM2                    (* (reg8 *) ok_sig__DM2) 
/* Input Buffer Disable Override */
#define ok_sig_INP_DIS                (* (reg8 *) ok_sig__INP_DIS)
/* LCD Common or Segment Drive */
#define ok_sig_LCD_COM_SEG            (* (reg8 *) ok_sig__LCD_COM_SEG)
/* Enable Segment LCD */
#define ok_sig_LCD_EN                 (* (reg8 *) ok_sig__LCD_EN)
/* Slew Rate Control */
#define ok_sig_SLW                    (* (reg8 *) ok_sig__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define ok_sig_PRTDSI__CAPS_SEL       (* (reg8 *) ok_sig__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define ok_sig_PRTDSI__DBL_SYNC_IN    (* (reg8 *) ok_sig__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define ok_sig_PRTDSI__OE_SEL0        (* (reg8 *) ok_sig__PRTDSI__OE_SEL0) 
#define ok_sig_PRTDSI__OE_SEL1        (* (reg8 *) ok_sig__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define ok_sig_PRTDSI__OUT_SEL0       (* (reg8 *) ok_sig__PRTDSI__OUT_SEL0) 
#define ok_sig_PRTDSI__OUT_SEL1       (* (reg8 *) ok_sig__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define ok_sig_PRTDSI__SYNC_OUT       (* (reg8 *) ok_sig__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(ok_sig__SIO_CFG)
    #define ok_sig_SIO_HYST_EN        (* (reg8 *) ok_sig__SIO_HYST_EN)
    #define ok_sig_SIO_REG_HIFREQ     (* (reg8 *) ok_sig__SIO_REG_HIFREQ)
    #define ok_sig_SIO_CFG            (* (reg8 *) ok_sig__SIO_CFG)
    #define ok_sig_SIO_DIFF           (* (reg8 *) ok_sig__SIO_DIFF)
#endif /* (ok_sig__SIO_CFG) */

/* Interrupt Registers */
#if defined(ok_sig__INTSTAT)
    #define ok_sig_INTSTAT            (* (reg8 *) ok_sig__INTSTAT)
    #define ok_sig_SNAP               (* (reg8 *) ok_sig__SNAP)
    
	#define ok_sig_0_INTTYPE_REG 		(* (reg8 *) ok_sig__0__INTTYPE)
#endif /* (ok_sig__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_ok_sig_H */


/* [] END OF FILE */
