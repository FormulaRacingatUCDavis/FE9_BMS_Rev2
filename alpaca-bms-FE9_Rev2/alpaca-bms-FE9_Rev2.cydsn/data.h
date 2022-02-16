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

#ifndef DATA_H
#define DATA_H
    
#define N_OF_SUBPACK (1u)    //number of subpacks
#define N_OF_BUSSES (1u)     //number of isoSPI busses
//Some code will need changing if switching back to multiple busses. 
//In cell_interface.c, get_cell_temps() is hard coded for bus 0. 
    
#define IC_PER_BUS (2u)      //6811 per bus. 
#define IC_PER_PACK (2u)     //6811 per subpack
#define CELLS_PER_LTC 12u    //number of cells per LTC
    
#define BOARD_TEMPS_PER_LTC 0u
#define CELL_TEMPS_PER_LTC 16u
    
#define CELL_TEMPS_PER_PACK 32u
#define BOARD_TEMPS_PER_PACK 0u
#define HUMIDITY_SENSORS_PER_PACK 0u
    
#define N_OF_LTC (IC_PER_PACK * N_OF_SUBPACK)                     //total number of LTC6811s
#define N_OF_CELL (N_OF_LTC * CELLS_PER_LTC)                      //total number of cells
#define CELLS_PER_SUBPACK (N_OF_CELL / N_OF_SUBPACK)              //cells per subpack
#define TEMPS_PER_LTC (CELL_TEMPS_PER_LTC + BOARD_TEMPS_PER_LTC)  //total number of temps per LTC
#define N_OF_TEMP_CELL (CELL_TEMPS_PER_LTC * N_OF_LTC)                 //total number of temps
#define N_OF_TEMP_BOARD (BOARD_TEMPS_PER_LTC * N_OF_LTC)          //total number of board temps
#define N_OF_TEMP (N_OF_TEMP_CELL + N_OF_TEMP_BOARD)

    
//MUX Indexes:   
//First LTC on Node: 
#define IC1_ADDRESSES [0, 2, 4, 6, 8]  //addresses
#define CELL_TEMP_INDEXES_IC1 [0, 1, 2, 3, 4, 5, 6, 7, 11, 12, 13, 14, 15]  //cell thermistors
#define BOARD_TEMP_INDEXES_IC1 [8, 10, 11]                                  //board thermistors
#define HUMIDITY_INDEXES_IC1 [9]                                            //humidity sensors

//Second LTC on Node: 
#define IC2_ADDRESSES [1, 3, 5, 7, 9]  //addresses
#define CELL_TEMP_INDEXES_IC2 [0, 1, 2, 3, 8, 9, 10, 11, 12, 13, 14, 15]  //cell thermistors
#define BOARD_TEMP_INDEXES_IC2 [4, 5, 6, 7]                               //board thermistors
#define HUMIDITY_INDEXES_IC2 []                                           //humidity sensors

#endif 


/* [] END OF FILE */
