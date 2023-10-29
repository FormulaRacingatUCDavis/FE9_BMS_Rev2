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

#ifndef MATRIX_MULT
#define MATRIX_MULT  

// Constant(s)

#define MAX_ELEMENTS 4

// Type(s)

typedef struct MatrixStruct {
    unsigned char rows;
    unsigned char columns;
    float elements[MAX_ELEMENTS];
} Matrix;

// Function declarations

void mat_init(unsigned char rows, unsigned char columns, Matrix * matrix);
void mat_set(unsigned char row, unsigned char column, float element, Matrix * matrix);
float mat_get(unsigned char row, unsigned char column, Matrix *);
void mat_fill_zero(Matrix *);
void mat_multiply(Matrix *matrixA, Matrix *matrixB, Matrix *matrixC);
void mat_transpose(Matrix* matrix, Matrix* result); // Function to transpose a 2x2 matrix
                                                    //                         ^^^

#endif 


/* [] END OF FILE */
