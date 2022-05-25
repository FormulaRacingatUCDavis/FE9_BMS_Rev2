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

#include <math.h>

double dt = 0.01;		// Sampling Period
int totalTime = 100;		// total #seconds
double R0 = 0.01;
double Rc = 0.015;
int Cc = 2400;
int Cbat = 18000;
double Voc0 = 3.435;
double alp = 0.007;

double Aprime[2][2];
double Aprime_transpose[2][2];

double Eprime[2][2];
double Eprime_transpose[2][2];

double Fprime[2][2];
double Cprime[2];
double Cprime_transpose[2][1];

// Coefficients for probability
double Rk;


//// Initializing the matrixes we'll need to use
double xhat[2][1];	//xhat is a 2-by-1 matrix, top value is: SOC_estimated, bottom is Vc_estimated
double Qk1[2][2];
double xhatk_1[2][1];
double fk[2][1];
double xhatCorrected[2][1];
double PCorrected[2][2];
double Lk[2][1];
double P[2][2];
double Pk_1[2][2];
double sub_mat_1[2][2];
double sub_mat_2[2][2];
double sub_mat_3[2][2];

// Variables subject to constant change
int i, j;
double t = 0.0, actualSOC = 1.0, Vc = 0.0;
double I = 0.0;
double Ik_1 = 0.0;
double V = 0.0;		// current V





double VOC(double SOC, double Voc0);
double hk(double SOC_val, double I, double Voc0, double R0);
void fk(double**** xhatk_1, double I, double dt, double Cbat, double Cc, double Rc, double**** fk_);
void mat_multiply(double**** Aprime, double**** Pk_1, double**** sub_mat_1, int num_cols_of_second_matrix);
void transpose(double*** Aprime, double*** Aprime_transpose);
void EKF();

// Function to transpose a 2x2 matrix
void transpose(double* Aprime[2][2], double* Aprime_transpose[2][2]) {
	(*Aprime_transpose)[0][1] = (*Aprime)[1][0];
	(*Aprime_transpose)[0][0] = (*Aprime)[0][0];
	(*Aprime_transpose)[1][0] = (*Aprime)[0][1];
	(*Aprime_transpose)[1][1] = (*Aprime)[1][1];
}

void init_kalman() {
    Aprime[0][0] = 1.0;
    Aprime[1][1] = exp(-dt / (Cc * Rc));
    transpose(&Aprime, &Aprime_transpose);
    
    Eprime[0][0] = 1.0;
    Eprime[1][1] = 1.0;
    transpose(&Eprime, &Eprime_transpose);
    
    Fprime[0][0] = 1.0;
    Fprime[1][1] = 1.0;
    
    Cprime[0] = 0.007;
    Cprime[1] = -1.0;
    
    (Cprime_transpose)[0][0] = Cprime[0];
    (Cprime_transpose)[1][0] = Cprime[1];
    Rk = pow(10, -4);
    
    
    Qk1[0][0] = 2.5 * pow(10, -7);
    
    P[0][0] = Rk;
    P[1][1] = Rk;
    Pk_1[0][0] = Rk;
    Pk_1[1][1] = Rk;
    
    (xhatk_1)[0][0] = actualSOC;
    (xhatk_1)[1][0] = Vc;
 }

void mat_multiply(double**** Aprime, double**** Pk_1, double**** sub_mat_1, int num_cols_of_second_matrix) {
	int row, column, index;
	double temp_val = 0.0;
	for (row = 0; row < 2; row++) {
		for (column = 0; column < num_cols_of_second_matrix; column++) {
			for (index = 0; index < 2; index++) {
				temp_val += ((*Aprime)[row][index]) * ((**Pk_1)[index][column]);
			}
			(**sub_mat_1)[row][column] = temp_val;
			temp_val = 0;
		}
	}
}

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

void fk(double**** xhatk_1, double I, double dt, double Cbat, double Cc, double Rc, double**** fk_) {
	double my_mat[2][1] = { {(-1) * I / Cbat}, {(I / Cc) - ((**xhatk_1)[1][0] / (Cc * Rc))} };
	for (int i = 0; i < 2; i++) {
		my_mat[i][0] *= dt;
		(**fk_)[i][0] = (**xhatk_1)[i][0] + my_mat[i][0];
	}

	// xhat = previous xhat + change in xhat in time (dt)
	// xhat is a derivative
}

// EKF Calculator function (to compute values of xhatCorrected and PCorrected continuously)
void EKF(double*** xhatk_1, double*** Pk_1, double I, double Ik_1, double V, double Voc0,
	double Rk,
	double*** fk_, double dt, double Cbat, double Cc, double Rc, double*** Qk1,
	double*** sub_mat_1, double*** sub_mat_2, double*** sub_mat_3, double*** xhat,
	double*** P, double*** Cprime_transpose, double*** Lk, double R0,
	double*** xhatCorrected, double*** PCorrected, double* actualSOC) {

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
		error_estimate += (*sub_mat_1)[row][0] * (Cprime)[row];
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
			(*sub_mat_1)[row][index] = (*Lk)[row][0] * (Cprime)[index];
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
  
void KalmanFilt_Int_Interrupt_InterruptCallback();

/* [] END OF FILE */
