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

#include <LTC6811.h>

uint8_t ADCV[2]; //!< Cell Voltage conversion command.
uint8_t ADAX[2]; //!< GPIO conversion command.

/*
addressify_cmd: 

Adds address (lt_addr) to first byte of command (cmd0). 

See LTC6811 datasheet, pg 58, table 37. 

Inputs: 
lt_addr - address to add to command
cmd0 - first byte of command

Returns: 
cmd0 - first byte of command with address

Intended usage: 
cmd[0] = some_command;
ltc_address = some_address;
cmd[0] = addressify_cmd(ltc_address, cmd[0]);
 */
uint8_t addressify_cmd(uint8_t lt_addr, uint8_t cmd0)
{
    //uint8_t addr = lt_addr % IC_PER_BUS;  
    lt_addr <<= 3;           //bitshift to correct posotion
    lt_addr &= 0b01111000;   //clear unwanted bits
    
    cmd0 &= 0b10000111;      //clear address bits
    cmd0 |= lt_addr;         //set address bits
    
    return cmd0;
}

/*
LTC6811_initialize: 

Sets adc mode and initializes LTC6811 config. 

Parameters: 
adc_mode - MD_FAST, MD_NORMAL, or MD_FILTERED

*/
void LTC6811_initialize(uint8_t adc_mode)
{
  LTC6811_set_adc(adc_mode, DCP_DISABLED, CELL_CH_ALL, AUX_CH_GPIO5); // MD_FILTERED from MD_NORMAL
  LTC6811_init_cfg();
}

/*!******************************************************************************************************************
 \brief Maps  global ADC control variables to the appropriate control bytes for each of the different ADC commands
 
@param[in] uint8_t MD The adc conversion mode
@param[in] uint8_t DCP Controls if Discharge is permitted during cell conversions
@param[in] uint8_t CH Determines which cells are measured during an ADC conversion command
@param[in] uint8_t CHG Determines which GPIO channels are measured during Auxiliary conversion command
 
 Command Code: \n
			|command	|  10   |   9   |   8   |   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   | 
			|-----------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|
			|ADCV:	    |   0   |   1   | MD[1] | MD[2] |   1   |   1   |  DCP  |   0   | CH[2] | CH[1] | CH[0] | 
			|ADAX:	    |   1   |   0   | MD[1] | MD[2] |   1   |   1   |  DCP  |   0   | CHG[2]| CHG[1]| CHG[0]| 
 ******************************************************************************************************************/

void LTC6811_set_adc(uint8_t MD, //ADC Mode
			         uint8_t DCP, //Discharge Permit
			         uint8_t CH, //Cell Channels to be measured
			         uint8_t CHG) //GPIO Channels to be measured
{
  uint8_t md_bits;
  
  md_bits = (MD & 0x02) >> 1;
  ADCV[0] = md_bits + 0x02;
  md_bits = (MD & 0x01) << 7;
  ADCV[1] =  md_bits + 0x60 + (DCP<<4) + CH;
 
  md_bits = (MD & 0x02) >> 1;
  ADAX[0] = md_bits + 0x04;
  md_bits = (MD & 0x01) << 7;
  ADAX[1] = md_bits + 0x60 + CHG ;
  
}


void LTC6811_init_cfg()
{
  uint8_t i = 0;
  for(i = 0; i<IC_PER_BUS;i++)
  {
    tx_cfg[i][0] = 0xFE; // See notes and page 59 for reasoning.
    tx_cfg[i][1] = 0x00; 
    tx_cfg[i][2] = 0x00;
    tx_cfg[i][3] = 0x00; 
    tx_cfg[i][4] = 0x00;
    tx_cfg[i][5] = 0x20; // DCTO=0x2 1 min
  }
 
}

/*!****************************************************
  \brief Wake the LTC6804 from the sleep state
  
 Generic wakeup commannd to wake the LTC6804 from sleep
 *****************************************************/
void wakeup_sleep()
{
    CyDelay(1);
    LTC68_WriteTxData(0x4D);  //write dummy byte to wake up (ascii 'M')
    CyDelay(1);
    while(! (LTC68_ReadTxStatus() & LTC68_STS_SPI_DONE)){}
    LTC68_ReadRxData();
    CyDelayUs(WAKE_UP_DELAY_US);
}


/* [] END OF FILE */
