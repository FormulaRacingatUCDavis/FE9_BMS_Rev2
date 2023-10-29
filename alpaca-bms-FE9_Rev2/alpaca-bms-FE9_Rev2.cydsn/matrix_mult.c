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

#include "matrix_mult.h"

void mat_init(unsigned char rows, unsigned char columns, Matrix* matrix) {

    matrix->rows = rows;
    matrix->columns = columns;
    mat_fill_zero(matrix);

    return;
}

void mat_set(unsigned char row, unsigned char column, float element, Matrix* matrix) {

    matrix->elements[(row - 1) * matrix->columns + (column - 1)] = element;

    return;
}

float mat_get(unsigned char row, unsigned char column, Matrix* matrix) {
    return matrix->elements[(row - 1) * matrix->columns + (column - 1)];
}

void mat_fill_zero(Matrix *matrix) {

    for (int i = 0; i < MAX_ELEMENTS; i++) {
        matrix->elements[i] = 0;
    }

    return;
}

void mat_multiply(Matrix* matrixA, Matrix* matrixB, Matrix* matrixC) {

    // matrixA.columns != matrixB.rows should yield an error 
    // BUT it should not check that in this application 

    // set the result matrix's rows and columns numbers and initialize its elements to zero
    mat_init(matrixA->rows, matrixB->columns, matrixC);
    mat_fill_zero(matrixC);

    for (int i = 1; i <= matrixC->rows; i++) {
        for (int j = 1; j <= matrixC->columns; j++) {
            for (int k = 1; k <= matrixA->columns; k++) {
                mat_set(i, j, mat_get(i, j, matrixC) + mat_get(i, k, matrixA) * mat_get(k, j, matrixB), matrixC);
            }
        }
    }

}

// Function to transpose a 2x2 matrix
//                         ^^^
void mat_transpose(Matrix* matrix, Matrix* result) {
//	(*Aprime_transpose)[0][1] = (*Aprime)[1][0];
//	(*Aprime_transpose)[0][0] = (*Aprime)[0][0];
//	(*Aprime_transpose)[1][0] = (*Aprime)[0][1];
//	(*Aprime_transpose)[1][1] = (*Aprime)[1][1];
    mat_init(2, 2, result);
    
    float matrix_1_2 = mat_get(1, 2, matrix);
    mat_set(2, 1, matrix_1_2, result);
    
    float matrix_1_1 = mat_get(1, 1, matrix);
    mat_set(1, 1, matrix_1_1, result);
    
    float matrix_2_1 = mat_get(2, 1, matrix);
    mat_set(1, 2, matrix_2_1, result);
    
    float matrix_2_2 = mat_get(2, 2, matrix);
    mat_set(2, 2, matrix_2_2, result);
}


/* [] END OF FILE */
