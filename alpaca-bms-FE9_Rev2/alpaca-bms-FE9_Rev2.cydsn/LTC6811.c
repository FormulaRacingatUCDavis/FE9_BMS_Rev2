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

uint8_t tx_cfg[IC_PER_BUS][6];   //!< 

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
void LTC6811_initialize(uint8_t adc_mode)  //tested 2/15/22
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
//tested 2/15/22
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


void LTC6811_init_cfg() //tested 2/15/22
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
void LTC6811_wakeup()  //tested 2/17
{
    CyDelay(1);
    SPI_WriteTxData(0x4D);  //write dummy byte to wake up (ascii 'M')
    CyDelay(1);
    while(! (SPI_ReadTxStatus() & SPI_STS_SPI_DONE)){}  //wait for tx to finish
    SPI_ReadRxData();                                   //read out buffer
    CyDelayUs(WAKE_UP_DELAY_US);       //wait specified time
}

/*
 * Broadcast write command -
 * select - the value the mux select pins should be set to
 * orig_cfga_data - old register values we don't want to change
 */
void LTC6811_wrcfga(uint8_t lt_addr, uint8_t select, uint8_t orig_cfga_data[6])  //tested 2/17
{
    uint8_t cmd[12];
    uint16_t temp_pec;
    
    
    // see LTC6811 datasheet for command codes
    if (lt_addr == 0xFF) {
        cmd[0] = 0; // For global write
    }
    else {    
        cmd[0] = 128;     // For addressed write
        cmd[0] = addressify_cmd(lt_addr, cmd[0]);
    }
    cmd[1] = 1;     // specifies wrcfga cmd
    
    // calculate pec for command code

    temp_pec = pec15_calc(2, (uint8_t *)cmd);
    cmd[2] = (uint8_t)(temp_pec >> 8);
    cmd[3] = (uint8_t)(temp_pec);

    /*
    cfga is an 8 byte register on the LTC6811, we care about cfga[0]:
    bits -  |7     |6       |5        |4        |3         |2     |1      |0          |
            |gpio5 |gpio4   |gpio3    |gpio2    |gpio1     |refon | dten  |adcopt (important) |
            | x    |select3 | select2 |select 1 |select 0  | 1    | 1     | 0         |
    
    In testing, we found that dten and refon must be 1, otherwise the function doesn't
    write the select (gpio) bits.
    GPIO1 is the output of the mux and must be written high. 
    If GPIO1 is written low then read function will get microvolts.
    */
    
    uint8_t cfgr0 = (select << 3) & 0b01111000; // 0000xxxx -> 0xxxx000
    
    cmd[4] = cfgr0 | 0b110;        //refon = 1 dten = 1 adcopt = 0
    cmd[5] = orig_cfga_data[1];    //rest of the register is written with its prev. values
    cmd[6] = orig_cfga_data[2];
    cmd[7] = orig_cfga_data[3];
    cmd[8] = orig_cfga_data[4];
    cmd[9] = orig_cfga_data[5];

    // calculate pec on data
    temp_pec = pec15_calc(6, (uint8_t*)(cmd + 4));
    cmd[10] = (uint8_t)(temp_pec >> 8);
    cmd[11] = (uint8_t)(temp_pec);
    
    // wakeup device and send cmd
    LTC6811_wakeup();
    spi_write_array(12, cmd);
}

/*
 *  Address write command for sending cell balance signals
 *  data[5] bytes 4 and 5 contain discharge time and cells to discharge
 *  lt_addr (0-17) corresponds to the address of an lt chip
*/

void LTC6811_wrcfga_balance(uint8_t lt_addr, uint8_t cfga_data[6]) {  //UNTESTED
    
    uint8_t cmd[12];
    uint16_t temp_pec;
    
    cmd[0] = 128;
    cmd[0] = addressify_cmd(lt_addr, cmd[0]);
    
    cmd[1] = 1;
    
    temp_pec = pec15_calc(2, (uint8_t*) cmd);
    cmd[2] = (uint8_t) (temp_pec >> 8);
    cmd[3] = (uint8_t) temp_pec;
    
    cmd[4] = 0x0E;
    cmd[5] = cfga_data[1];
    cmd[6] = cfga_data[2];
    cmd[7] = cfga_data[3];
    cmd[8] = cfga_data[4];
    cmd[9] = cfga_data[5] | 0x20;
    
    temp_pec = pec15_calc(6, (uint8_t*) cmd + 4);
    cmd[10] = (uint8_t) (temp_pec >> 8);
    cmd[11] = (uint8_t) temp_pec;
    
    LTC6811_wakeup();
    spi_write_array(12, cmd);
}

/*
 * Address read command 
 * TODO: implement address parameter
 * cfga[6] - stores values read from the 6811
 */
int8_t LTC6811_rdcfga(uint8_t lt_addr, uint8_t cfga[6])   //tested 2/17
{
    uint8_t cmd[4];         // bytes for rdcfga cmd; sent to slaves
    int8_t pec_error;       // returned, indicates whether cmd failed or not
    uint16_t data_pec;      // stores our calculated pec
    uint16_t recieved_pec;  // stores 6811's calculated pec
    uint16_t cmd_pec;
    uint8_t rx_data[8];     // stores slave to master data (6 bytes data, 2 bytes pec)
   
    cmd[0] = 128;           // See pg. 55-63 of 6811 datasheet for cmd formats
    cmd[0] = addressify_cmd(lt_addr, cmd[0]);
    
    cmd[1] = 2;                                              
    cmd_pec = pec15_calc(2, cmd);
    cmd[2] = (uint8_t)(cmd_pec >> 8);
    cmd[3] = (uint8_t)(cmd_pec);
    
    LTC6811_wakeup();
    spi_write_read(cmd, 4, rx_data, 8);
    
    recieved_pec = *(uint16_t *)(rx_data + 6);
    data_pec = pec15_calc(6, rx_data);
    
    if(recieved_pec != data_pec){
        pec_error = -1;
    }
    
    for(int i = 0; i < 6; i++){
        cfga[i] = rx_data[i];
    }
    
    return pec_error;
}

/*!
 \brief Writes and read a set number of bytes using the SPI port.

@param[in] uint8_t tx_data[] array of data to be written on the SPI port
@param[in] uint8_t tx_len length of the tx_data array
@param[out] uint8_t rx_data array that read data will be written too. 
@param[in] uint8_t rx_len number of bytes to be read from the SPI port.

//Returns 0 on success and -1 on fail

*/

/*!**********************************************************
 \brief calaculates  and returns the CRC15
  

@param[in]  uint8_t len: the length of the data array being passed to the function
               
@param[in]  uint8_t data[] : the array of data that the PEC will be generated from
  

@return  The calculated pec15 as an unsigned int16_t
***********************************************************/
uint16_t pec15_calc(uint8_t len, uint8_t *data)
{
	uint16_t remainder,addr;
	
	remainder = 16;//initialize the PEC
	for(uint8_t i = 0; i<len;i++) // loops for each byte in data array
	{
		addr = ((remainder>>7)^data[i])&0xff;//calculate PEC table address 
		remainder = (remainder<<8)^crc15Table[addr];
	}
	return(remainder*2);//The CRC15 has a 0 in the LSB so the remainder must be multiplied by 2
}


/*!
 \brief Writes an array of bytes out of the SPI port
 
 @param[in] uint8_t len length of the data array being written on the SPI port
 @param[in] uint8_t data[] the data array to be written on the SPI port
 
*/
void spi_write_array(uint8_t len, // Option: Number of bytes to be written on the SPI port
					 uint8_t data[] //Array of bytes to be written on the SPI port
					 )
{ // SKY_ADDED
  CyDelay(1);
  for(uint8_t i = 0; i < len; i++)
  {
     SPI_WriteTxData((int8_t)data[i]);
  }
  CyDelay(1);
}

int8_t spi_write_read(uint8_t tx_Data[],//array of data to be written on SPI port 
					uint8_t tx_len, //length of the tx data arry
					uint8_t *rx_data,//Input: array that will store the data read by the SPI port
					uint8_t rx_len //Option: number of bytes to be read from the SPI port
					)
{
    
    SPI_ClearRxBuffer();
    
    uint8_t i = 0;
    uint8_t dummy_read;         //stores uneeded values
    
    for(i = 0; i < tx_len; i++){
        SPI_WriteTxData(tx_Data[i]);
    }
   
    for(i = 0; i < rx_len; i++){
        SPI_WriteTxData(0x00);  //just to send SPI clock pulse? 
    }
    
    CyDelay(1);
        
    while(dummy_read!=rx_len+tx_len){     //check if data has been received
        dummy_read=(uint8_t)SPI_GetRxBufferSize();    
    };
    
    while(!(SPI_ReadTxStatus() & SPI_STS_SPI_DONE)){}  //await tx finished
    
    for(i = 0; i < tx_len; i++){
        dummy_read = (uint8_t)SPI_ReadRxData();   //read out tx values from rx buffer
    }

    for(i = 0; i < rx_len; i++){
        rx_data[i] = (uint8_t)SPI_ReadRxData();   //read out rx values
    }

    CyDelayUs(200);
    return 0;
}


/* [] END OF FILE */
