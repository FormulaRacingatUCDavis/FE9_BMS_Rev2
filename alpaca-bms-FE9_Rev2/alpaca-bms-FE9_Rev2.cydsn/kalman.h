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
#ifndef KALMAN_H_
#define KALMAN_H_
    
#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<stdbool.h>

// Constants
extern volatile double dt;		// Sampling Period - From TopDesign.cysch 
extern volatile double R0;
extern volatile double Rc;
extern volatile int Cc;
extern volatile int Cbat;
extern volatile double Voc0;
extern volatile double alp;


volatile double** Aprime;
volatile double** Aprime_transpose;

volatile double** Eprime;
volatile double** Eprime_transpose;

volatile double** Fprime;

volatile double* Cprime;
volatile double** Cprime_transpose;

// Coefficients for probability
volatile double Rk;

// --------------------------------------------------------------------------
// At time k
// --------------------------------------------------------------------------

// Variables Qualitatively
// (Constant)VOC: Open circuit voltage
// (Time variant) Vc : Voltage across capacitor in battery circuit model
// (Time variant) V : Voltage we measure(output from battery circuit model)
// (Constant)Cc : Capacitor's capacitance
// (Constant)Rc : Resistor in parallel with capacitor
// (Constant)R0 : Ohmic resistance
// (Constant)Cbat : Capacity of battery in AmpHours ?


//// Initializing the matrixes we'll need to use
volatile double** xhat;	//xhat is a 2-by-1 matrix, top value is: SOC_estimated, bottom is Vc_estimated
volatile double** Qk1;

volatile double** xhatk_1;
volatile double** fk_;
volatile double** xhatCorrected;
volatile double** PCorrected;
volatile double** Lk;
volatile double** P;
volatile double** Pk_1;

volatile double** sub_mat_1;
volatile double** sub_mat_2;
volatile double** sub_mat_3;

// Variables subject to constant change
volatile int i, j;
volatile double t;
volatile double actualSOC;
volatile double Vc;
volatile double I;
volatile double Ik_1;
volatile double V;		// current 
    

// Function declarations
double VOC(double SOC, double Voc0);
double hk(double SOC_val, double I, double Voc0, double R0);
void fk(volatile double**** xhatk_1, double I, double dt, double Cbat, double Cc, double Rc, volatile double**** fk_);
void mat_multiply(volatile double**** Aprime, volatile double**** Pk_1, volatile double**** sub_mat_1, int num_cols_of_second_matrix);
void transpose(volatile double*** Aprime, volatile double*** Aprime_transpose);
void EKF(volatile double*** xhatk_1, volatile double*** Pk_1, double I, double Ik_1, double V, double Voc0,
	double Rk, volatile double*** Aprime, volatile double** Cprime, volatile double*** Eprime, volatile double*** Fprime,
	volatile double*** fk_, double dt, double Cbat, double Cc, double Rc, volatile double*** Qk1,
	volatile double*** Aprime_transpose, volatile double*** Eprime_transpose,
	volatile double*** sub_mat_1, volatile double*** sub_mat_2, volatile double*** sub_mat_3, volatile double*** xhat,
	volatile double*** P, volatile double*** Cprime_transpose, volatile double*** Lk, double R0,
	volatile double*** xhatCorrected, volatile double*** PCorrected);
volatile double** memory_allocate(int num_columns);

void init_kalman();
void KalmanFilt_Int_Interrupt_InterruptCallback();

#endif //KALMAN_H
/* [] END OF FILE */
