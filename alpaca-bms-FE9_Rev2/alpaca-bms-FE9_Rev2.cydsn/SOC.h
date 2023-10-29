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
#ifndef SOC_H
#define SOC_H

#include "cell_interface.h"
#include "matrix_mult.h"

//void update_soc();
    
// SOC Constants
#define dt 0.01		// Sampling Period
#define R0 0.01
#define Rc 0.015
#define Cc 2400
#define Cbat 18000
#define Voc0 3.435
    
// Initializing probability matrices
Matrix Aprime;
Matrix Aprime_transpose;

Matrix Eprime;
Matrix Eprime_transpose; 

Matrix Cprime; // VOC(SOC)
Matrix Cprime_transpose;


// Coefficients for probability
float Rk;

// --------------------------------------------------------------------------
// At time k
// --------------------------------------------------------------------------

// Variables Qualitatively
// (Constant) VOC : Open circuit voltage
// (Time variant) Vc : Voltage across capacitor in battery circuit model
// (Time variant) V : Voltage we measure(output from battery circuit model)
// (Constant)Cc : Capacitor's capacitance
// (Constant)Rc : Resistor in parallel with capacitor
// (Constant)R0 : Ohmic resistance
// (Constant)Cbat : Capacity of battery in AmpHours ?

//// Initializing the matrixes we'll need to use
Matrix xhat;	//xhat is a 2-by-1 matrix, top value is: SOC_estimated, bottom is Vc_estimated
Matrix Qk1;
Matrix xhatk_1;
Matrix fk_;
Matrix xhatCorrected;
Matrix PCorrected;
Matrix Lk;
Matrix P;
Matrix Pk_1;
Matrix sub_mat_1;
Matrix sub_mat_2;
Matrix sub_mat_3;

// Variables subject to constant change
uint8_t i, j;
float actualSOC;
float Vc;
float I;
float Ik_1;
float V;		// current V


void init_SOC_vars();
void update_SOC_input(uint16_t voltage, int16_t current);
float VOC(float soc);
float hk(float SOC_val, float I);
void fk(Matrix* xhatk_1, float I, Matrix* fk_);
void EKF(Matrix* xhatk_1, Matrix* Pk_1, float I, float Ik_1, float V,
	float Rk, Matrix* Aprime, Matrix* Cprime, Matrix* Eprime,
	Matrix* fk_, Matrix* Qk1,
	Matrix* Aprime_transpose, Matrix* Eprime_transpose,
	Matrix* sub_mat_1, Matrix* sub_mat_2, Matrix* sub_mat_3, Matrix* xhat,
	Matrix* P, Matrix* Cprime_transpose, Matrix* Lk,
	Matrix* xhatCorrected, Matrix* PCorrected);

#endif
/* [] END OF FILE */
