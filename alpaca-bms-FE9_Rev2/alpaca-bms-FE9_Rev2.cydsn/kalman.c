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

#include "cell_interface.h"
#include "kalman.h"
#include "project.h"

//------------------------------- SMALLER CALCULATION FUNCTIONS --------------------------------

extern volatile BAT_PACK_t bat_pack;

// Functions we need to calculate
double VOC(double SOC, double Voc0) {
	double VOC_ = 0.007 * SOC + Voc0;
	return VOC_;
}

double hk(double SOC_val, double I, double Voc0, double R0) {
	//double SOC_val = (*xhat)[0][0];
	double hk_ = VOC(SOC_val, Voc0) - (R0 * I);		// hk_ is calculated voltage?
	return hk_;
}

void fk(volatile double**** xhatk_1, double I, double dt, double Cbat, double Cc, double Rc, volatile double**** fk_) {
	double my_mat[2][1] = { {(-1) * I / Cbat}, {(I / Cc) - ((**xhatk_1)[1][0] / (Cc * Rc))} };
	for (int i = 0; i < 2; i++) {
		my_mat[i][0] *= dt;
		(**fk_)[i][0] = (**xhatk_1)[i][0] + my_mat[i][0];
	}

	// xhat = previous xhat + change in xhat in time (dt)
	// xhat is a derivative
}


//------------------------------------ MATRIX FUNCTIONS -------------------------------------

// Function to allocate memory for a 2x1 or 2x2 matrix
volatile double** memory_allocate(int num_columns) {
	int i, j;
	// memory allocation
	volatile double** xhat;
	xhat = (volatile double**)malloc((2) * sizeof(volatile double*));
	for (i = 0; i < 2; i++) {
		xhat[i] = (volatile double*)malloc((num_columns) * sizeof(volatile double));
	}
	// Initialize values to 0.0
	for (i = 0; i < 2; i++) {
		for (j = 0; j < num_columns; j++) {
			xhat[i][j] = 0.0;
		}
	}
	return xhat;
}

// Function to multiply 2 matrixes
void mat_multiply(volatile double**** Aprime, volatile double**** Pk_1, volatile double**** sub_mat_1, int num_cols_of_second_matrix) {
	int row, column, index;
	double temp_val = 0.0;
	for (row = 0; row < 2; row++) {
		for (column = 0; column < num_cols_of_second_matrix; column++) {
			for (index = 0; index < 2; index++) {
				temp_val += ((**Aprime)[row][index]) * ((**Pk_1)[index][column]);
			}
			(**sub_mat_1)[row][column] = temp_val;
			temp_val = 0;
		}
	}
}

// Function to transpose a 2x2 matrix
void transpose(volatile double*** Aprime, volatile double*** Aprime_transpose) {
	(*Aprime_transpose)[0][1] = (*Aprime)[1][0];
	(*Aprime_transpose)[0][0] = (*Aprime)[0][0];
	(*Aprime_transpose)[1][0] = (*Aprime)[0][1];
	(*Aprime_transpose)[1][1] = (*Aprime)[1][1];
}


//------------------------------------ EKF CALCULATOR FUNCTION -------------------------------------

// EKF Calculator function (to compute values of xhatCorrected and PCorrected continuously)
void EKF(volatile double*** xhatk_1, volatile double*** Pk_1, double I, double Ik_1, double V, double Voc0,
	double Rk, volatile double*** Aprime, volatile double** Cprime, volatile double*** Eprime, volatile double*** Fprime,
	volatile double*** fk_, double dt, double Cbat, double Cc, double Rc, volatile double*** Qk1,
	volatile double*** Aprime_transpose, volatile double*** Eprime_transpose,
	volatile double*** sub_mat_1, volatile double*** sub_mat_2, volatile double*** sub_mat_3, volatile double*** xhat,
	volatile double*** P, volatile double*** Cprime_transpose, volatile double*** Lk, double R0,
	volatile double*** xhatCorrected, volatile double*** PCorrected) {

	int row, col, index;

	//----------------- CALCULATIONS FOR xhat -----------------
	fk(&xhatk_1, I, dt, Cbat, Cc, Rc, &fk_);
	for (row = 0; row < 2; row++) {
		(*xhat)[row][0] = (*fk_)[row][0];
	}

	//----------------- CALCULATIONS FOR P -----------------
	// Multiplying Aprime * Pk_1 * Aprime_transpose
	mat_multiply(&Aprime, &Pk_1, &sub_mat_1, 2);
	mat_multiply(&sub_mat_1, &Aprime_transpose, &sub_mat_2, 2);

	// Multiplying Eprime * Qk1 * Eprime_transpose
	mat_multiply(&Eprime, &Qk1, &sub_mat_1, 2);
	mat_multiply(&sub_mat_1, &Eprime_transpose, &sub_mat_3, 2);

	// P = (Aprime * Pk_1 * Aprime_transpose) + (Eprime * Qk1 * Eprime_transpose)
	for (row = 0; row < 2; row++) {
		for (col = 0; col < 2; col++) {
			(*P)[row][col] = (*sub_mat_2)[row][col] + (*sub_mat_3)[row][col];
		}
	}

	//----------------- CALCULATIONS FOR Lk -----------------
	// error_estimate = (Cprime * P * Cprime_tranpose)
	mat_multiply(&P, &Cprime_transpose, &sub_mat_1, 1);
	double error_estimate = 0.0, combined_error = 0.0;
	for (row = 0; row < 2; row++) {
		error_estimate += (*sub_mat_1)[row][0] * (*Cprime)[row];
	}
	// combined_error = [(Cprime * P * Cprime_tranpose) + Rk]^(-1)
	combined_error = error_estimate + Rk;
	combined_error = pow(combined_error, (-1));

	// Lk = combined_error * (P * Cprime_transpose)
	for (row = 0; row < 2; row++) {
		(*Lk)[row][0] = (*sub_mat_1)[row][0] * combined_error;
	}

	//--------------- CALCULATIONS FOR xhatCorrected ---------------
	// temp_val = (SOC + Vc) - hk(100*SOC, I, Voc0, R0)
	double temp_val = 0.0;
	temp_val = (V + (*xhat)[1][0]) - hk(100 * (*xhat)[0][0], I, Voc0, R0);
	
	// xhatCorrected = (Lk * temp_val) * xhat
	for (row = 0; row < 2; row++) {
		(*sub_mat_1)[row][0] = (*Lk)[row][0] * temp_val;
		(*xhatCorrected)[row][0] = (*xhat)[row][0] + (*sub_mat_1)[row][0];
	}

	//----------------- CALCULATIONS FOR PCorrected -----------------
	// FINDING: Lk * Cprime
	for (row = 0; row < 2; row++) {
		for (index = 0; index < 2; index++) {
			(*sub_mat_1)[row][index] = (*Lk)[row][0] * (*Cprime)[index];
		}
	}
	// FINDING: (Lk * Cprime) * P
	mat_multiply(&sub_mat_1, &P, &sub_mat_2, 2);

	// PCorrected = P - [(Lk * Cprime) * P]
	for (row = 0; row < 2; row++) {
		for (col = 0; col < 2; col++) {
			(*PCorrected)[row][col] = (*P)[row][col] - (*sub_mat_2)[row][col];
		}
	}

}
  
void init_kalman() {
    // Initializing probability matricies
	Aprime = memory_allocate(2);
	Aprime_transpose = memory_allocate(2);
	Aprime[0][0] = 1.0;
	Aprime[1][1] = exp(-dt / (Cc * Rc));
	transpose(&Aprime, &Aprime_transpose);
	
	Eprime = memory_allocate(2);
	Eprime_transpose = memory_allocate(2);
	Eprime[0][0] = 1.0;
	Eprime[1][1] = 1.0;
	transpose(&Eprime, &Eprime_transpose);

	Fprime = memory_allocate(2);
	Fprime[0][0] = 1.0;
	Fprime[1][1] = 1.0;
    
	// VOC(SOC)
	Cprime = (double*)malloc(2 * sizeof(double));
	Cprime[0] = 0.007;
	Cprime[1] = -1.0;

	Cprime_transpose = memory_allocate(1);
	(Cprime_transpose)[0][0] = Cprime[0];
	(Cprime_transpose)[1][0] = Cprime[1];

	// Coefficients for probability
	Rk = pow(10, -4);


	//// Initializing the matrixes we'll need to use
	xhat = memory_allocate(1);	//xhat is a 2-by-1 matrix, top value is: SOC_estimated, bottom is Vc_estimated
	Qk1 = memory_allocate(2);
	Qk1[0][0] = 2.5 * pow(10, -7);
	xhatk_1 = memory_allocate(1);
	fk_ = memory_allocate(1);
	xhatCorrected = memory_allocate(1);
	PCorrected = memory_allocate(2);
	Lk = memory_allocate(1);
	P = memory_allocate(2);
	Pk_1 = memory_allocate(2);
	P[0][0] = Rk;
	P[1][1] = Rk;
	Pk_1[0][0] = Rk;
	Pk_1[1][1] = Rk;
	sub_mat_1 = memory_allocate(2);
	sub_mat_2 = memory_allocate(2);
	sub_mat_3 = memory_allocate(2);

	// Variables subject to constant change
	(xhatk_1)[0][0] = actualSOC;
	(xhatk_1)[1][0] = Vc;
 }


void KalmanFilt_Int_Interrupt_InterruptCallback() {
    CyGlobalIntDisable;
    // FUNCTION TO CALCULATE xhatCorrected & PCorrected
	EKF(&xhatk_1, &Pk_1, I, Ik_1, V, Voc0, Rk, &Aprime, &Cprime, &Eprime,
		&Fprime, &fk_, dt, Cbat, Cc, Rc, &Qk1,
		&Aprime_transpose, &Eprime_transpose, &sub_mat_1, &sub_mat_2,
		&sub_mat_3, &xhat, &P, &Cprime_transpose, &Lk, R0, &xhatCorrected,
		&PCorrected);


	// Setting the xhatk_1 and Pk_1 values
	for (i = 0; i < 2; i++) {
		// set values of xhatk_1 to xhatCorrected
		xhatk_1[i][0] = xhatCorrected[i][0];
		// For each column of second matrix
		for (j = 0; j < 2; j++) {
			// set values of Pk_1 to PCorrected
			Pk_1[i][j] = PCorrected[i][j];
		}
	}
    
    //Update SOC percentage to be the most recently calculated value
    bat_pack.SOC_percent = xhatk_1[0][0];
    
    CyGlobalIntEnable
}

/* [] END OF FILE */
