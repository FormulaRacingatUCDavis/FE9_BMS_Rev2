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

void LTC6811_wrcfga_mux(uint8_t addr, uint8_t select){
    
    uint8_t cfg0 = (select << 3) & 0b01111000;
    cfg0 |= 0b10000100;
    
    if (addr == 0xFF){
        for (uint8_t i = 0; i < IC_PER_BUS; i++){
            tx_cfg[i][0] = cfg0;
        }
    } else {
        tx_cfg[addr][0] = cfg0; 
    }
    
    //LTC6811_wrcfga(addr); 
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
    bits -  |7     |6       |5        |4        |3         |2     |1            |0                  |
            |gpio5 |gpio4   |gpio3    |gpio2    |gpio1     |refon | dten        |adcopt (important) |
            | 1    |select3 | select2 |select 1 |select 0  | 1    | read only   | 0     (adc mode)  |
    
    In testing, we found that dten and refon must be 1, otherwise the function doesn't
    write the select (gpio) bits.
    GPIO5 is the output of the mux and must be written high. 
    If GPIO1 is written low then read function will get microvolts.
    */
    
    uint8_t cfgr0 = (select << 3) & 0b01111000; // 0000xxxx -> 0xxxx000
    
    cmd[4] = cfgr0 | 0b10000100;        //gpio5 = 1 refon = 1 adcopt = 0
    cmd[5] = orig_cfga_data[1];         //rest of the register is written with its prev. values
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
    spi_write(cmd, 12);
}

/*
 *  Address write command for sending cell balance signals
 *  data[5] bytes 4 and 5 contain discharge time and cells to discharge
 *  lt_addr (0-17) corresponds to the address of an lt chip
*/

void LTC6811_wrcfga_balance(uint8_t lt_addr){//, uint8_t cfga_data[6]) {  //UNTESTED
    
    uint8_t cmd[12];
    uint16_t temp_pec;
    
    cmd[0] = 128;
    cmd[0] = addressify_cmd(lt_addr, cmd[0]);
    
    cmd[1] = 1;
    
    temp_pec = pec15_calc(2, (uint8_t*) cmd);
    cmd[2] = (uint8_t) (temp_pec >> 8);
    cmd[3] = (uint8_t) temp_pec;
    
    cmd[4] = 0x0E;
    cmd[5] = 0x00; //cfga_data[1];
    cmd[6] = 0x00; //cfga_data[2];
    cmd[7] = 0x00; //cfga_data[3];
    cmd[8] = 0x02;
    cmd[9] = 0x20;
    
    temp_pec = pec15_calc(6, (uint8_t*) cmd + 4);
    cmd[10] = (uint8_t) (temp_pec >> 8);
    cmd[11] = (uint8_t) temp_pec;
    
    LTC6811_wakeup();
    spi_write(cmd, 12);
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



/*!*********************************************************************************************
  \brief Starts cell voltage conversion
  
  Starts ADC conversions of the LTC6804 Cpin inputs.
  The type of ADC conversion done is set using the associated global variables:
 |Variable|Function                                      | 
 |--------|----------------------------------------------|
 | MD     | Determines the filter corner of the ADC      |
 | CH     | Determines which cell channels are converted |
 | DCP    | Determines if Discharge is Permitted	     |
  
***********************************************************************************************/
void LTC6811_adcv()
{

  uint8_t cmd[4];
  uint16_t temp_pec;
  
  //1
  cmd[0] = ADCV[0]; //ADCV
  cmd[1] = ADCV[1];
  
  //2
  temp_pec = pec15_calc(2, ADCV);
  cmd[2] = (uint8_t)(temp_pec >> 8);
  cmd[3] = (uint8_t)(temp_pec);
  
  //3
  LTC6811_wakeup();
  
  //4
  CyDelay(1);
  spi_write(cmd, 4);
  CyDelay(1);
}

//reads one register from one chip
int8_t LTC6811_rdcv_ltc_reg(uint8_t reg, uint8_t * data, uint8_t addr){
    uint8_t cmd[4];
    uint16_t temp_pec;
    uint16_t data_pec;
    uint16_t received_pec;

    if (reg == 1) {
        cmd[1] = 0x04;
        cmd[0] = 0x00;
    }
    else if (reg == 2) {
        cmd[1] = 0x06;
        cmd[0] = 0x00;
    }
    else if (reg == 3) {
        cmd[1] = 0x08;
        cmd[0] = 0x00;
    }
    else if (reg == 4) {
        cmd[1] = 0x0A;
        cmd[0] = 0x00;
    }

    cmd[0] = 0x80 + (addr << 3);

    temp_pec = pec15_calc(2, cmd);
    cmd[2] = (uint8_t)(temp_pec >> 8);
    cmd[3] = (uint8_t)(temp_pec);

    LTC6811_wakeup();
    
    int num_tries = 0;
    int try_again = 0;
    
    
    do {
        spi_write_read(cmd, 4, &data[addr*8], 8);
        try_again = 0;
        
        // check for missing voltages in reg (6553's)
        for (int i = 0; i < 3; i+= 2) {
            if (data[addr*8 + i] == 0xFF && data[addr*8 + i + 1] == 0xFF)
                try_again = 1;
        }
        // check for pec Errors
        received_pec = (*(&data[addr*8] + 6) << 8) + *(&data[addr*8] + 7);
        data_pec = pec15_calc(6, &data[addr*8]);
        
        num_tries++;
        if (num_tries > 2)
            return -1;
        
    } while ((data_pec != received_pec) || try_again);
    
    return 0;
}
/* NOT WORKING
//read all voltages from one chip
int8_t LTC6811_rdcv_ltc(uint8_t addr, uint16_t voltages[CELLS_PER_LTC]){
    
    //bytes to recieve
    const uint8_t NUM_RX_BYT = 8;
    // bytes in a 6811 reg
    const uint8_t BYT_IN_REG = 6;
    // each cell gets 2 bytes in a reg
    const uint8_t CELL_IN_REG = 3;
    
    int8_t pec_error = 0;
    uint16_t parsed_cell;
    uint16_t received_pec;
    uint16_t data_pec;
    uint8_t data_counter=0; //data counter
    uint8_t cell_data[NUM_RX_BYT];
    
    for(uint8_t cell_reg = 1; cell_reg<5; cell_reg++)         			 //executes once for each of the LTC6804 cell voltage registers
    {
        data_counter = 0;
     
        LTC6811_rdcv_ltc_reg(cell_reg, cell_data, addr);

    	for(uint8_t current_cell = 0; current_cell<CELL_IN_REG; current_cell++)	   // This loop parses the read back data. 
        {														   		           // Loops once for each cell voltage in the register 
          parsed_cell = cell_data[data_counter] + (cell_data[data_counter + 1] << 8);
          voltages[current_cell  + ((cell_reg - 1) * CELL_IN_REG)] = parsed_cell;
          data_counter = data_counter + 2;
        }
        
        received_pec = (cell_data[data_counter] << 8) + cell_data[data_counter+1];
        data_pec = pec15_calc(BYT_IN_REG, cell_data);
        
        if(received_pec != data_pec)
        {
          pec_error = -1;
        }
    }
    
    return pec_error;
}
*/
/***********************************************//**
 \brief Read the raw data from the LTC6804 cell voltage register
 
 The function reads a single cell voltage register and stores the read data
 in the *data point as a byte array. This function is rarely used outside of 
 the LTC6804_rdcv() command. 
 
 @param[in] uint8_t reg; This controls which cell voltage register is read back. 
         
          1: Read back cell group A 
		  
          2: Read back cell group B 
		  
          3: Read back cell group C 
		  
          4: Read back cell group D 
		  
 @param[in] uint8_t total_ic; This is the number of ICs in the network
 
 @param[out] uint8_t *data; An array of the unparsed cell codes 
 *************************************************/
void LTC6811_rdcv_reg(uint8_t reg,
					  uint8_t total_ic, 
					  uint8_t *data
					  )
{
  uint8_t cmd[4];
  uint16_t temp_pec;
  
  //1
  if (reg == 1)
  {
    cmd[1] = 0x04;
    cmd[0] = 0x00;
  }
  else if(reg == 2)
  {
    cmd[1] = 0x06;
    cmd[0] = 0x00;
  } 
  else if(reg == 3)
  {
    cmd[1] = 0x08;
    cmd[0] = 0x00;
  } 
  else if(reg == 4)
  {
    cmd[1] = 0x0A;
    cmd[0] = 0x00;
  } 

  //2
 
  
  //3
  LTC6811_wakeup(); //This will guarantee that the LTC6804 isoSPI port is awake. This command can be removed.
  
  //4
  for(int current_ic = 0; current_ic<total_ic; current_ic++)
  {
	cmd[0] = 0x80 + (current_ic<<3); //Setting address
    
    temp_pec = pec15_calc(2, cmd);
	cmd[2] = (uint8_t)(temp_pec >> 8);
	cmd[3] = (uint8_t)(temp_pec); 
	CyDelay(1);
	spi_write_read(cmd,4,&data[current_ic*8],8);
    CyDelay(1);
  }
}

/*
  LTC6804_rdcv_reg Function Process:
  1. Determine Command and initialize command array
  2. Calculate Command PEC
  3. Wake up isoSPI, this step is optional
  4. Send Global Command to LTC6804 stack
*/

/***********************************************//**
 \brief Reads and parses the LTC6804 cell voltage registers.
 
 The function is used to read the cell codes of the LTC6804.
 This function will send the requested read commands parse the data
 and store the cell voltages in cell_codes variable. 
 
 
@param[in] uint8_t reg; This controls which cell voltage register is read back. 
 
          0: Read back all Cell registers 
		  
          1: Read back cell group A 
		  
          2: Read back cell group B 
		  
          3: Read back cell group C 
		  
          4: Read back cell group D 
 
@param[in] uint8_t total_ic; This is the number of ICs in the network
 

@param[out] uint16_t cell_codes[]; An array of the parsed cell codes from lowest to highest. The cell codes will
  be stored in the cell_codes[] array in the following format:
  |  cell_codes[0]| cell_codes[1] |  cell_codes[2]|    .....     |  cell_codes[11]|  cell_codes[12]| cell_codes[13] |  .....   |
  |---------------|----------------|--------------|--------------|----------------|----------------|----------------|----------|
  |IC1 Cell 1     |IC1 Cell 2      |IC1 Cell 3    |    .....     |  IC1 Cell 12   |IC2 Cell 1      |IC2 Cell 2      | .....    |
 
 @return int8_t, PEC Status.
 
	0: No PEC error detected
  
	-1: PEC error detected, retry read
 *************************************************/
int8_t LTC6811_rdcv(uint8_t reg,
					 uint8_t total_ic,
					 uint16_t cell_codes[][12]
					 )
{
  //bytes to recieve
  const uint8_t NUM_RX_BYT = 8;
  // bytes in a 6811 reg
  const uint8_t BYT_IN_REG = 6;
  // each cell gets 2 bytes in a reg
  const uint8_t CELL_IN_REG = 3;
  
  int8_t pec_error = 0;
  uint16_t parsed_cell;
  uint16_t received_pec;
  uint16_t data_pec;
  uint8_t data_counter=0; //data counter
  uint8_t cell_data[NUM_RX_BYT * total_ic];

  //1.a
  if (reg == 0)
  {
    //a.i
    for(uint8_t cell_reg = 1; cell_reg<5; cell_reg++)         			 //executes once for each of the LTC6804 cell voltage registers
    {
      data_counter = 0;
      //LTC6804_rdcv_reg(cell_reg, total_ic,cell_data);
    
      for (int ic_num = 0; ic_num < total_ic; ic_num++) {
         //LTC6804_rdcv_ltc_reg(cell_reg, cell_data, ic_num);
         LTC6811_rdcv_ltc_reg(cell_reg, cell_data, ic_num);
      }
    /*
      if (cell_reg == 1) {
          for (int ic_num = 0; ic_num < total_ic; ic_num += 3) {
             LTC6811_rdcv_ltc_reg(cell_reg, cell_data, ic_num);
             CyDelay(1);
             LTC6811_rdcv_ltc_reg(cell_reg, cell_data, ic_num);
             CyDelay(1);
          }
      }*/
    
      for (uint8_t current_ic = 0 ; current_ic < total_ic; current_ic++) // executes for every LTC6804 in the stack
      {																 	  // current_ic is used as an IC counter
        //a.ii
		for(uint8_t current_cell = 0; current_cell<CELL_IN_REG; current_cell++)	// This loop parses the read back data. Loops 
        {														   		  // once for each cell voltages in the register 
          parsed_cell = cell_data[data_counter] + (cell_data[data_counter + 1] << 8);
          cell_codes[current_ic][current_cell  + ((cell_reg - 1) * CELL_IN_REG)] = parsed_cell;
          data_counter = data_counter + 2;
        }
		//a.iii
        received_pec = (cell_data[data_counter] << 8) + cell_data[data_counter+1];
        data_pec = pec15_calc(BYT_IN_REG, &cell_data[current_ic * NUM_RX_BYT ]);
        if(received_pec != data_pec)
        {
          pec_error = -1;
        }
        data_counter=data_counter+2;
      }
    }
  }
 //1.b
  else
  {
	//b.i
	
    LTC6811_rdcv_reg(reg, total_ic,cell_data);
    for (uint8_t current_ic = 0 ; current_ic < total_ic; current_ic++) // executes for every LTC6804 in the stack
    {							   									// current_ic is used as an IC counter
		//b.ii
		for(uint8_t current_cell = 0; current_cell < CELL_IN_REG; current_cell++)   									// This loop parses the read back data. Loops 
		{						   									// once for each cell voltage in the register 
			parsed_cell = cell_data[data_counter] + (cell_data[data_counter+1]<<8);
			cell_codes[current_ic][current_cell + ((reg - 1) * CELL_IN_REG)] = 0x0000FFFF & parsed_cell;
			data_counter= data_counter + 2;
		}
		//b.iii
	    received_pec = (cell_data[data_counter] << 8 )+ cell_data[data_counter + 1];
        data_pec = pec15_calc(BYT_IN_REG, &cell_data[current_ic * NUM_RX_BYT * (reg-1)]);
		if(received_pec != data_pec)
		{
			pec_error--;//pec_error = -1;
		}
	}
  }

    CyDelay(1);
 //2
return(pec_error);
}
/*
	LTC6804_rdcv Sequence
	
	1. Switch Statement:
		a. Reg = 0
			i. Read cell voltage registers A-D for every IC in the stack
			ii. Parse raw cell voltage data in cell_codes array
			iii. Check the PEC of the data read back vs the calculated PEC for each read register command
		b. Reg != 0 
			i.Read single cell voltage register for all ICs in stack
			ii. Parse raw cell voltage data in cell_codes array
			iii. Check the PEC of the data read back vs the calculated PEC for each read register command
	2. Return pec_error flag
*/


/*!******************************************************************************************************
 \brief Start an GPIO Conversion
 
  Starts an ADC conversions of the LTC6804 GPIO inputs.
  The type of ADC conversion done is set using the associated global variables:
 |Variable|Function                                      | 
 |--------|----------------------------------------------|
 | MD     | Determines the filter corner of the ADC      |
 | CHG    | Determines which GPIO channels are converted |
 
*********************************************************************************************************/
void LTC6811_adax()
{
  uint8_t cmd[4];
  uint16_t temp_pec;
 
  cmd[0] = ADAX[0];
  cmd[1] = ADAX[1];
  temp_pec = pec15_calc(2, ADAX);
  cmd[2] = (uint8_t)(temp_pec >> 8);
  cmd[3] = (uint8_t)(temp_pec);
 
  LTC6811_wakeup(); //This will guarantee that the LTC6804 isoSPI port is awake. This command can be removed.
  spi_write(cmd, 4);
}
/*
  LTC6804_adax Function sequence:
  
  1. Load adax command into cmd array
  2. Calculate adax cmd PEC and load pec into cmd array
  3. wakeup isoSPI port, this step can be removed if isoSPI status is previously guaranteed
  4. send broadcast adax command to LTC6811 stack
*/




/*
 *  Read the voltage of a specific GPIO pin
 *  for reference:
 *  group 0 (auxa): set 0 = gpio1, set 1 = gpio2, set 2 = gpio2
 *  group 1 (auxb): set 0 = gpio4, set 1 = gpio5, set 2 = vref2
 *  group 2 (auxc): ?? look for in data sheet
 *  group 3 (auxd): ?? look for in data sheet
 */
int8_t LTC6811_rdaux_pin(uint8_t lt_addr, enum AuxPins pin, uint16_t *aux)
{
    uint8_t cmd[4];
    uint16_t cmd_pec;
    uint8_t rx_data[8];
    int8_t pec_error;
    uint16_t received_pec;
    uint16_t data_pec;
    
    cmd[0] = 128;
    cmd[0] = addressify_cmd(lt_addr, cmd[0]);
    
    uint8_t group = pin / 3;
    uint8_t set = 2* (pin % 3);
    
    switch(group) {
        case 0: cmd[1] = 12; break;     // GPIO 1 2 3 
        case 1: cmd[1] = 14; break;     // GPIO 4 5 VRef
        case 2: cmd[1] = 13; break;     // Different Register
        case 3: cmd[1] = 15; break;     // Different Register
    }
    
    cmd_pec = pec15_calc(2, cmd);
    cmd[2] = (cmd_pec >> 8);
    cmd[3] = cmd_pec;
    
    uint8_t num_tries = 0;
    
    do {
        LTC6811_wakeup();
        spi_write_read(cmd, 4, rx_data, 8);
            
        received_pec = (*(rx_data + 6) << 8) + *(rx_data + 7);
        data_pec = pec15_calc(6, rx_data);
        num_tries++;
        
        if (num_tries > 2) {
            num_tries = 0;
            *aux = 0xFFFF;
            return -1;
        }
    
    } while (data_pec != received_pec);
    
    *aux = rx_data[set] | (rx_data[set + 1] << 8);
    
    return 0;
}
//These are unused, but may be useful in the future. 
/*
int8_t LTC6811_rdaux(uint8_t reg,
					 uint8_t total_ic, 
					 uint16_t aux_codes[][6]
					 )
{


  const uint8_t NUM_RX_BYT = 8;
  const uint8_t BYT_IN_REG = 6;
  const uint8_t GPIO_IN_REG = 3;
  
  uint8_t data_counter = 0; 
  int8_t pec_error = 0;
  uint16_t received_pec;
  uint16_t data_pec;
  uint8_t data[NUM_RX_BYT * total_ic];
  //1.a
  if (reg == 0)
  {
	//a.i
    for(uint8_t gpio_reg = 1; gpio_reg<3; gpio_reg++)		 	   		 //executes once for each of the LTC6804 aux voltage registers
    {
      data_counter = 0;
      LTC6811_rdaux_reg(gpio_reg, total_ic,data);
      for (uint8_t current_ic = 0 ; current_ic < total_ic; current_ic++) // This loop executes once for each LTC6804
      {									  								 // current_ic is used as an IC counter
        //a.ii
		for(uint8_t current_gpio = 0; current_gpio< GPIO_IN_REG; current_gpio++)	// This loop parses GPIO voltages stored in the register
        {								   													
          
          aux_codes[current_ic][current_gpio +((gpio_reg-1)*GPIO_IN_REG)] = data[data_counter] + (data[data_counter+1]<<8);
          data_counter=data_counter+2;
		  
        }
		//a.iii
        received_pec = (data[data_counter]<<8)+ data[data_counter+1];
        data_pec = pec15_calc(BYT_IN_REG, &data[current_ic*NUM_RX_BYT*(gpio_reg-1)]);
        if(received_pec != data_pec)
        {
          pec_error = -1;
        }
       
        data_counter=data_counter+2;
      }
   

    }
  
  }
  else
  {
	//b.i
    LTC6811_rdaux_reg(reg, total_ic, data);
    for (int current_ic = 0 ; current_ic < total_ic; current_ic++) // executes for every LTC6804 in the stack
    {							   // current_ic is used as an IC counter
		//b.ii
		for(int current_gpio = 0; current_gpio<GPIO_IN_REG; current_gpio++)   // This loop parses the read back data. Loops 
		{						   // once for each aux voltage in the register 
			aux_codes[current_ic][current_gpio +((reg-1)*GPIO_IN_REG)] = 0x0000FFFF & (data[data_counter] + (data[data_counter+1]<<8));
			data_counter=data_counter+2;
		}
		//b.iii
		received_pec = (data[data_counter]<<8) + data[data_counter+1];
        data_pec = pec15_calc(6, &data[current_ic*8*(reg-1)]);
        if(received_pec != data_pec)
        {
          pec_error = -1;
        }
	}
  }
  return (pec_error);
}
*/

/*
	LTC6804_rdaux Sequence
	
	1. Switch Statement:
		a. Reg = 0
			i. Read GPIO voltage registers A-D for every IC in the stack
			ii. Parse raw GPIO voltage data in cell_codes array
			iii. Check the PEC of the data read back vs the calculated PEC for each read register command
		b. Reg != 0 
			i.Read single GPIO voltage register for all ICs in stack
			ii. Parse raw GPIO voltage data in cell_codes array
			iii. Check the PEC of the data read back vs the calculated PEC for each read register command
	2. Return pec_error flag
*/


/***********************************************//*
 \brief Read the raw data from the LTC6804 auxiliary register
 
 The function reads a single GPIO voltage register and stores thre read data
 in the *data point as a byte array. This function is rarely used outside of 
 the LTC6804_rdaux() command. 
 
 @param[in] uint8_t reg; This controls which GPIO voltage register is read back. 
		  
          1: Read back auxiliary group A
		  
          2: Read back auxiliary group B 

         
 @param[in] uint8_t total_ic; This is the number of ICs in the stack
 
 @param[out] uint8_t *data; An array of the unparsed aux codes 
 *************************************************/
/*
void LTC6811_rdaux_reg(uint8_t reg, 
					   uint8_t total_ic,
					   uint8_t *data
					   )
{
  uint8_t cmd[4];
  uint16_t cmd_pec;
  
  //1
  if (reg == 1)
  {
    cmd[1] = 0x0C;
    cmd[0] = 0x00;
  }
  else if(reg == 2)
  {
    cmd[1] = 0x0e;
    cmd[0] = 0x00;
  } 
  else
  {
     cmd[1] = 0x0C;
     cmd[0] = 0x00;
  }
  //2
  cmd_pec = pec15_calc(2, cmd);
  cmd[2] = (uint8_t)(cmd_pec >> 8);
  cmd[3] = (uint8_t)(cmd_pec);
  
  //3
  LTC6811_wakeup(); //This will guarantee that the LTC6804 isoSPI port is awake, this command can be removed.
  //4
   for(int current_ic = 0; current_ic<total_ic; current_ic++)
  {
	cmd[0] = 0x80 + (current_ic<<3); //Setting address
    cmd_pec = pec15_calc(2, cmd);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec); 
	spi_write_read(cmd,4,&data[current_ic*8],8);
  }
}
*/
/*
  LTC6804_rdaux_reg Function Process:
  1. Determine Command and initialize command array
  2. Calculate Command PEC
  3. Wake up isoSPI, this step is optional
  4. Send Global Command to LTC6804 stack
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
void spi_write(uint8_t data[], //Array of bytes to be written on the SPI port
               uint8_t len     //Option: Number of bytes to be written on the SPI port 
			   )
{ 
  uint8_t dummy[1];
  spi_write_read(data, len, dummy, 0);
    
  //SOMETHING WITH THIS WAS FUCKED I HAD TO RUN ADAX AND WRCFGA 5 TIMES TO GET THEM TO WORK FUCK YOU 
  /*CyDelay(1);
  for(uint8_t i = 0; i < len; i++)
  {
     SPI_WriteTxData((int8_t)data[i]);
  }
  CyDelay(1);*/ 
}


/*!
 \brief Writes and read a set number of bytes using the SPI port.

@param[in] uint8_t tx_data[] array of data to be written on the SPI port
@param[in] uint8_t tx_len length of the tx_data array
@param[out] uint8_t rx_data array that read data will be written too. 
@param[in] uint8_t rx_len number of bytes to be read from the SPI port.

//Returns 0 on success and -1 on fail

*/
int8_t spi_write_read(volatile uint8_t tx_Data[],//array of data to be written on SPI port 
					uint8_t tx_len, //length of the tx data arry
					uint8_t *rx_data,//Input: array that will store the data read by the SPI port
					uint8_t rx_len //Option: number of bytes to be read from the SPI port
					)
{
    
    SPI_ClearRxBuffer();
    
    uint8_t i = 0;
    uint8_t dummy_read;         //stores uneeded values
    volatile uint8_t receivedTx[10];
    
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
        receivedTx[i] = (uint8_t)SPI_ReadRxData();   //read out tx values from rx buffer
    }

    for(i = 0; i < rx_len; i++){
        rx_data[i] = (uint8_t)SPI_ReadRxData();   //read out rx values
    }

    CyDelayUs(200);
    return 0;
}


/* [] END OF FILE */
