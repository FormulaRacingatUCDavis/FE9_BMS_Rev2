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
    
#include <stdint.h>
    
#define N_OF_SUBPACK (5u)    //number of subpacks
#define N_OF_BUSSES (1u)     //number of isoSPI busses
//Some code will need changing if switching back to multiple busses. 
//In cell_interface.c, get_cell_temps() is hard coded for bus 0. 
    
#define IC_PER_BUS (10u)      //6811 per bus. 
#define IC_PER_PACK (2u)     //6811 per subpack
#define CELLS_PER_LTC 12u    //number of cells per LTC
    
//#define BOARD_TEMPS_PER_LTC 0u
//#define CELL_TEMPS_PER_LTC 16u
    
#define CELL_TEMPS_PER_PACK 24u
#define BOARD_TEMPS_PER_PACK 7u
#define HUMIDITY_SENSORS_PER_PACK 1u
    
#define TEMP_LOOP_DIVISION 4   //spread out temp collection over this many loops
    
#define N_OF_LTC (IC_PER_PACK * N_OF_SUBPACK)                     //total number of LTC6811s
#define N_OF_CELL (N_OF_LTC * CELLS_PER_LTC)                      //total number of cells
#define CELLS_PER_SUBPACK (N_OF_CELL / N_OF_SUBPACK)              //cells per subpack
#define TEMPS_PER_LTC 16u                                          //total number of temps per LTC
#define N_OF_TEMP_CELL (CELL_TEMPS_PER_PACK * N_OF_SUBPACK)                 //total number of temps
#define N_OF_TEMP_BOARD (BOARD_TEMPS_PER_PACK * N_OF_SUBPACK)          //total number of board temps
#define N_OF_TEMP (N_OF_TEMP_CELL + N_OF_TEMP_BOARD)

#define TEMPS_PER_LOOP (TEMPS_PER_LTC/TEMP_LOOP_DIVISION)
    


#endif 


/* [] END OF FILE */
