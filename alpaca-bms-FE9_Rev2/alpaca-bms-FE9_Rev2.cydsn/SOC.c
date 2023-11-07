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

#include "SOC.h"

extern volatile BAT_PACK_t bat_pack;

// mimiced in charger code
// so master board can send SOC back and forth
// 240 values taken from data sheet
// used to approximate state of charge
//int SOC_LUT[240] =  {
//    0, 5, 13, 22, 31, 39,
//    48, 57, 67, 76, 86, 
//    90, 106, 117, 127, 138,
//    150, 162, 174, 186, 199,
//    212, 226, 241, 256, 271, 
//    288, 205, 324, 345, 364,
//    386, 410, 436, 465, 497,
//    534, 577, 629, 695, 780,
//    881, 972, 1044, 1103, 1157,
//    1206, 1253, 1299, 1344, 1389, 
//    1434, 1479, 1527, 1576, 1628, 
//    1682, 1738, 1798, 1859, 1924,
//    1992, 2062, 2134, 2208, 2281, 
//    2424, 2492, 2557, 2620, 2681,
//    2743, 2804, 2868, 2903, 3003,
//    3078, 3161, 3253, 3354, 3467, 
//    3589, 3720, 3851, 3976, 4092, 
//    4200, 4303, 4404, 4504, 4603,
//    4700, 4792, 4878, 4958, 5032,
//    5101, 5166, 5228, 5289, 5347,
//    5405, 5462, 5518, 5573, 5628,
//    5680, 5731, 5780, 5826, 5869,
//    5911, 5951, 5988, 6024, 6059,
//    6092, 6124, 6156, 6187, 6217, 
//    6247, 6278, 6308, 6337, 6368,
//    6398, 6428, 6459, 6491, 6523,
//    6556, 6590, 6625, 6660, 6696,
//    6733, 6770, 6808, 6846, 6884,
//    6923, 6961, 7000, 7039, 7077,
//    7115, 7153, 7191, 7228, 7266, 
//    7303, 7340, 7376, 7413, 7449,
//    7484, 7520, 7555, 7590, 7625,
//    7659, 7694, 7728, 7762, 7796,
//    7830, 7864, 7898, 7932, 7966, 
//    8000, 8034, 8068, 8102, 8136,
//    8170, 8204, 8238, 8272, 8306,
//    8340, 8373, 8406, 8440, 8472,
//    8505, 8538, 8570, 8602, 8632,
//    8666, 8697, 8729, 8760, 8791,
//    8822, 8853, 8884, 8915, 8945, 
//    8976, 9006, 9036, 9067, 9097, 
//    9127, 9157, 9186, 9216, 9245,
//    9275, 9304, 9333, 9362, 9390,
//    9419, 9447, 9475, 9503, 9531,
//    9559, 9586, 9613, 9640, 9647, 
//    9693, 9720, 9746, 9772, 9797,
//    9823, 9848, 9873, 9898, 9923,
//    9947, 9971, 9995, 100000    
//};
//
//void update_soc(){
//    bat_pack.SOC_percent = SOC_LUT[((uint32_t)bat_pack.LO_voltage * 28 - 934000) / 1000] / 100;
//}

// Estimates the accumulator's SOC (state of charge) with respect to time
// based on simulated values for the circuit's V (voltage) and I (current).

#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<stdbool.h>

#include "matrix_mult.h"

#define RX_BUFFER_LENGTH 128

//#pragma warning (disable:4996)

 
void init_SOC_vars() {
    // Aprime
    mat_init(2, 2, &Aprime);
    mat_set(1, 1, 1.0, &Aprime);
    mat_set(2, 2, exp(-dt / (Cc * Rc)), &Aprime);
    // Aprime_transpose
    mat_transpose(&Aprime, &Aprime_transpose);
    
    // Eprime
    mat_init(2, 2, &Eprime);
    mat_set(1, 1, 1.0, &Eprime);
    mat_set(2, 2, 1.0, &Eprime);
    //Eprime_transpose
    mat_transpose(&Eprime, &Eprime_transpose);  
    
    
    // Cprime
    mat_init(2, 1, &Cprime);
    mat_set(1, 1, 0.007, &Cprime);
    mat_set(2, 1, -1.0, &Cprime);
    // Cprime_transpose
    mat_init(2, 1, &Cprime_transpose);
    mat_set(1, 1, mat_get(1, 1, &Cprime), &Cprime_transpose);
    mat_set(2, 1, mat_get(1, 2, &Cprime), &Cprime_transpose);
    
    // Rk
    Rk = pow(10, -4);
    
    // xhat
    mat_init(2, 1, &xhat);
    
    // Qk1
    mat_init(2, 2, &Qk1);
    mat_set(1, 1, 2.5 * pow(10, -7), &Qk1);
    
    // xhatk_1
    mat_init(2, 1, &xhatk_1);
    
    // fk_
    mat_init(2, 1, &fk_);
    
    mat_init(2, 1, &xhatCorrected);
 
    mat_init(2, 2, &PCorrected);

    mat_init(2, 1, &Lk);

    mat_init(2, 2, &P);
    mat_set(1, 1, Rk, &P);
    mat_set(2, 2, Rk, &P);

    mat_init(2, 2, &Pk_1);
    mat_set(1, 1, Rk, &Pk_1);
    mat_set(2, 2, Rk, &Pk_1);

    mat_init(2, 2, &sub_mat_1);
    mat_init(2, 2, &sub_mat_2);
    mat_init(2, 2, &sub_mat_3);
    
    
    mat_set(1, 1, actualSOC, &xhatk_1);
    mat_set(2, 1, Vc, &xhatk_1);
    
    actualSOC = 1.0;
    Vc = 0.0;
    I = 0.0;
    Ik_1 = 0.0;
    V = 0.0;		// current V
}

void update_SOC_input(uint16_t voltage, int16_t current) {
    V = ((float)voltage) / 100.0f / 120.0f;  // 120 cells series
    I = ((float)current) / 10.0f / 4.0f;   // 4 cells parallel
}


//------------------------------- SMALLER CALCULATION FUNCTIONS --------------------------------

// Functions we need to calculate
float VOC(float soc) {
  int8_t soc_int = (int8_t)soc;

  const uint16_t voc_lt[101] = {
      28930, 29662, 30239, 30689, 31101, 31510, 31915, 32304, 32661, 32960,
      33245, 33466, 33703, 33931, 34100, 34159, 34222, 34313, 34372, 34451,
      34539, 34650, 34781, 34921, 35038, 35172, 35276, 35374, 35472, 35560,
      35646, 35744, 35838, 35940, 36028, 36094, 36150, 36248, 36320, 36395,
      36467, 36539, 36624, 36726, 36788, 36889, 36987, 37043, 37145, 37249,
      37331, 37426, 37511, 37615, 37678, 37782, 37854, 37962, 38053, 38155,
      38259, 38364, 38475, 38573, 38667, 38769, 38854, 38923, 39030, 39103,
      39188, 39289, 39390, 39501, 39609, 39710, 39824, 39971, 40078, 40209,
      40320, 40405, 40408, 40496, 40539, 40582, 40600, 40639, 40657, 40694,
      40700, 40714, 40757, 40794, 40837, 40906, 40968, 41070, 41213, 41427,
      41668};

  if (soc_int < 0) {
    return 4.2f;
  } else if (soc_int > 100) {
    return 2.5f;
  } else {
    return ((float)voc_lt[soc_int]) / 10000;
  }
}

float hk(float SOC_val, float I) {
	return VOC(SOC_val) - (R0 * I);		// hk_ is calculated voltage?
}

void fk(Matrix* xhatk_1, float I, Matrix* fk_) {
    float array_mat[2][1] = { {(-1) * I / Cbat}, {(I / Cc) - (mat_get(2, 1, xhatk_1) / (Cc * Rc))} };
   
    array_mat[0][0] *= dt;
    mat_set(1, 1, mat_get(1, 1, xhatk_1) + array_mat[0][0], fk_);
    array_mat[1][0] *= dt;
    mat_set(2, 1, mat_get(2, 1, xhatk_1) + array_mat[1][0], fk_);
    
    
	// xhat = previous xhat + change in xhat in time (dt)
	// xhat is a derivative
}


//------------------------------------ EKF CALCULATOR FUNCTION -------------------------------------

// EKF Calculator function (to compute values of xhatCorrected and PCorrected continuously)
void EKF(Matrix* xhatk_1, Matrix* Pk_1, float I, float V, 
	float Rk, Matrix* Aprime, Matrix* Cprime, Matrix* Eprime, 
	Matrix* fk_, Matrix* Qk1,
	Matrix* Aprime_transpose, Matrix* Eprime_transpose,
	Matrix* sub_mat_1, Matrix* sub_mat_2, Matrix* sub_mat_3, Matrix* xhat,
	Matrix* P, Matrix* Cprime_transpose, Matrix* Lk, 
	Matrix* xhatCorrected, Matrix* PCorrected) {

	uint8_t row, col, index; // used for loops

	//----------------- CALCULATIONS FOR xhat -----------------
	fk(xhatk_1, I, fk_);
    for (row = 1; row <= 2; row++) {
        mat_set(row, 1, mat_get(row, 1, fk_), xhat);
    }

	//----------------- CALCULATIONS FOR P -----------------
	// Multiplying Aprime * Pk_1 * Aprime_transpose
	mat_multiply(Aprime, Pk_1, sub_mat_1);
	mat_multiply(sub_mat_1, Aprime_transpose, sub_mat_2);

	// Multiplying Eprime * Qk1 * Eprime_transpose
	mat_multiply(Eprime, Qk1, sub_mat_1);
	mat_multiply(sub_mat_1, Eprime_transpose, sub_mat_3);

	// P = (Aprime * Pk_1 * Aprime_transpose) + (Eprime * Qk1 * Eprime_transpose)
    for (row = 1; row <= 2; row++) {
		for (col = 1; col <= 2; col++) {
            mat_set(row, col, mat_get(row, col, sub_mat_2) + mat_get(row, col, sub_mat_3), P);
		}
	}

	//----------------- CALCULATIONS FOR Lk -----------------
    // error_estimate = (Cprime * P * Cprime_tranpose)
	mat_multiply(P, Cprime_transpose, sub_mat_1);
	float error_estimate = 0.0, combined_error = 0.0;
    for (row = 1; row <= 2; row++) {
        error_estimate += mat_get(row, 1, sub_mat_1) * mat_get(row, 1, Cprime);
	}
	// combined_error = [(Cprime * P * Cprime_tranpose) + Rk]^(-1)
	combined_error = error_estimate + Rk;
	combined_error = pow(combined_error, (-1));

	// Lk = combined_error * (P * Cprime_transpose)
    for (row = 1; row <= 2; row++) {
		mat_set(row, 1, mat_get(row, 1, sub_mat_1) * combined_error, Lk);
	}

	//--------------- CALCULATIONS FOR xhatCorrected ---------------
	// temp_val = (SOC + Vc) - hk(100*SOC, I, Voc0, R0)
	float temp_val = 0.0;
    temp_val = (V + mat_get(2, 1, xhat)) - hk(100 * mat_get(1, 1, xhat), I);
	
	// xhatCorrected = (Lk * temp_val) * xhat
	for (row = 1; row <= 2; row++) {
		mat_set(row, 1, mat_get(row, 1, Lk) * temp_val, sub_mat_1);
		mat_set(row, 1, mat_get(row, 1, xhat) + mat_get(row, 1, sub_mat_1), xhatCorrected);
	}  

	//----------------- CALCULATIONS FOR PCorrected -----------------
	// FINDING: Lk * Cprime
    for (row = 1; row <= 2; row++) {
		for (index = 1; index <= 2; index++) {
			mat_set(row, index, mat_get(row, 1, Lk) * mat_get(index, 1, Cprime), sub_mat_1); // sussy, check later
		}
	}
	// FINDING: (Lk * Cprime) * P
	mat_multiply(sub_mat_1, P, sub_mat_2);

	// PCorrected = P - [(Lk * Cprime) * P]
    for (row = 1; row <= 2; row++) {
		for (col = 1; col <= 2; col++) {
			mat_set(row, col, mat_get(row, col, P) - mat_get(row, col, sub_mat_2), PCorrected);
		}
	}

}


/* [] END OF FILE */
