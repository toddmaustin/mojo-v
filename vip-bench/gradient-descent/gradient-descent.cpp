/** Gradient Descent for linear regression.
* The formula for linear regression is : Y= wX + b
* Cost Function: 1/n⅀(Yi - (wXi + b))^2 [i starts from 0 → n]
*Functions Description:
* 1) derivateWRTWeight(VIP_ENCDOUBLE weight, VIP_ENCDOUBLE bias):
* 	Is the partial derivative of the cost function with respect to weight
* 2) derivateWRTBias(VIP_ENCDOUBLE weight, VIP_ENCDOUBLE bias):
* 	Is the partial derivative of the cost function with respect to bias
* 3) gradientDescent(VIP_ENCDOUBLE weight, VIP_ENCDOUBLE bias):
*	Represents the gradient descent function. It is used to manage the value of the weight
*	and the bias of the linear function.
*/
#include <iostream>

#include "../config.h"

using namespace std;
#define M  106
VIP_ENCDOUBLE  L = 0.001;

// VIP_ENCDOUBLE X[M] = {0, 1, 2, 3, 4, 5, 6, 7};
// VIP_ENCDOUBLE Y[M] = {1, 2, 3, 4, 5, 6, 7, 8};
//VIP_ENCDOUBLE X[M] = {-5, -4.7, -3, -2.5, -1, 1.5, 2, 2.3, 3, 3.9};
//VIP_ENCDOUBLE Y[M] = {3, 1.7, 2, 1.5, 4, -0.3, -1.5, -2, -4.4, -6};

//Number of homicides by firearm
VIP_ENCDOUBLE Y[M] = {56, 20, 1198, 9,30,18,11,52,1456,8,
12,70,68,18,34678,0,51,187,173,353,
12539,248,201,17,27,5,20,15,1618,1790,
453,2446,41,3,24,35,24,158,29,5009,
85,5201,7,0,3093,21,6,417,1080,11,
26,210,14,28,5,31,17,6,3,25,
64,5,0,1,11309,8,3,84,55,7,
338,5,2,569,466,757,7349,35,44,1,
5,45,128,1,10,2,0,8319,90,291,
37,57,128,15,365,535,5,280,100,9146,
93,11115,834,105,28,598}; //6

//Average firearms per 100 people
VIP_ENCDOUBLE X[M] = {8.6, 7.6,10.2,12.5,15,30.4,3.5,5.3,0.5,7.8,
7.3,17.2,10,17.3,8,1.4,6.2,4.3,30.8,10.7,
5.9,1.4,9.9,21.7,4.8,36.4,16.3,12,5.1,1.3,
3.5,5.8,6.2,9.2,45.3,7.3,30.3,22.5,13.1,14.6,
6.2,5.5,30.3,4.2,8.6,7.3,11.9,8.1,0.6,11.5,
1.3,1.1,0.9,19,21,1.6,0.7,15.3,24.1,1.5,
6.5,11.9,14.7,15,7.1,1.9,0.8,3.9,22.6,7.7,
21.9,31.3,21.7,17,18.8,4.7,1.3,8.5,19.2,0.7,
37.8,0.6,0.5,8.3,13.5,0.4,12.7,10.4,1.5,31.6,
45.7,4.4,1,1.6,12.5,3.8,1.4,6.6,88.8,31.8,
10.7,1.7,3.4,8.9,4.4,8.6};


VIP_ENCDOUBLE derivateWRTWeight(VIP_ENCDOUBLE weight, VIP_ENCDOUBLE bias){
	VIP_ENCDOUBLE sum = 0;
	
	for(int i = 0; i<M; i++){
		sum = (VIP_ENCDOUBLE)(sum + (X[i]*(Y[i] - (weight * X[i] + bias))));
	}
	
	return ((VIP_ENCDOUBLE)-2.0 * sum)/M;
}

VIP_ENCDOUBLE derivateWRTBias(VIP_ENCDOUBLE weight, VIP_ENCDOUBLE bias){
	VIP_ENCDOUBLE sum = 0;
	
	for(int i = 0; i<M; i++){
		sum = (VIP_ENCDOUBLE)(sum + (Y[i] - (weight * X[i] + bias)));
	}
	
	return ((VIP_ENCDOUBLE)-2.0 * sum)/M;
}

void gradientDescent(VIP_ENCDOUBLE &weight, VIP_ENCDOUBLE &bias){
	for(unsigned i = 0; i<10000; i++){
		weight = (VIP_ENCDOUBLE)(weight - (L*derivateWRTWeight(weight, bias)));
		bias =(VIP_ENCDOUBLE)(bias - (L*derivateWRTBias(weight, bias)));		
	}
}
int main() {
	VIP_ENCDOUBLE weight = 0;
	VIP_ENCDOUBLE bias = 0;
  {
    Stopwatch s("VIP_Bench Runtime");

	  gradientDescent(weight, bias);
  }
	cout << "The function is: " << VIP_DEC(weight) << "x + " << VIP_DEC(bias) <<endl;

	return 0;
}

