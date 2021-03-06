#include<stdlib.h>
#include<stdint.h>
#include<stdio.h>
#include<stdbool.h>
#include<math.h>
#include"vector.h"
#include"chaoticSystem.h"

/*
It uses Eulers's method to solve the Lorenz-Stenflo Chaotic System.
Each time this function is called, it generates the i+1th chaotic vector
this vector is unpredictable unless the initial parameters are known, making them perfect
for generating a pseudorandom mask
*/
void eulerPlusOne(matrix* params, vector* current, float stepSize) {
	vector xi = *current;
	vector fxi = LorenzStenflo(params, &xi);
	*current = vsum(xi, sprod(stepSize, fxi));

}

vector LorenzStenflo(matrix* params, vector* current) {
	vector uxv = { 0, -1 * current->x * current->z, current->x * current->y, 0 };
	return vsum(mprod(*params, *current), uxv);
}

//Initialises parameter matrix, used in the eulerPlusOne function to step to generate the numerical solution at the proceeding t value
static void initParams(matrix* params, float rho, float sigma, float beta, float gamma) {
	vector v0 = {
		-1*sigma,
		rho,
		0.0,
		-1
	};
	vector v1 = {
		sigma,
		-1,
		0,
		0
	};

	vector v2 = {
		0,
		0,
		-1*beta,
		0
	};
	vector v3 = {
		gamma,
		0,
		0,
		sigma
	};

	params->v0=v0;
	params->v1=v1;
	params->v2=v2;
	params->v3=v3;
}


//Generates a numeric solution to the Lorenz-Stenflo Equations
void GenNumeric(FILE* input, FILE* output, size_t messageLength, float init[], uint32_t windUp) {
	vector vec = { 5, 5, 5, 5 };
	
	matrix paramsVal={vec,vec,vec,vec};
	matrix* params = &paramsVal;

	initParams(params,40,10,8/3,50);
	for(int i = 0; i<messageLength; i++){
		eulerPlusOne(params, &vec, 0.1);
		fprintf(output, "%F,", vec.x);
		fprintf(output, "%F,", vec.y);
		fprintf(output, "%F,", vec.x);
		fprintf(output, "%F\n", vec.w);
	}
}
/*
For each 128-bit block of data, XOR mask every byte with a byte generated by the Lorenz-Stenflo System
*/
void numericSolve(FILE* input, FILE* output, size_t messageLength, float init[], uint32_t windUp) {

	//unpacks parameters
	vector vec = { fmod(init[0],280/3),fmod(init[1],280/3),fmod(init[2],280/3),fmod(init[3],280/3) };

	float rho = init[4];
	float sigma = init[5];
	float beta = init[6];
	float gamma = init[7];

	uint8_t bytestream[4];

	
	matrix paramsVal={vec,vec,vec,vec};
	matrix* params = &paramsVal;

	initParams(params,rho,sigma,beta,gamma);
	for(int i = 0; i<windUp + 1024; i++){
		eulerPlusOne(params, &vec, 0.1);
	}
	for(int i=0; i<messageLength/16; i++) {
		eulerPlusOne(params, &vec, 0.1);
		
		bytestream[0] = fgetc(input);
		bytestream[1] = fgetc(input);
		bytestream[2] = fgetc(input);
		bytestream[3] = fgetc(input);
		//turns data and float bytes into two 32-bit lengths of binary and XOR gates them together
		*(int32_t*) bytestream ^= *(int32_t*) &vec.x;
		fwrite(bytestream, 1, sizeof(int32_t), output);
		//XORing the encrypted data with the same chaotic system vector restores the data back to it's unencryypted state
		bytestream[0] = fgetc(input);
		bytestream[1] = fgetc(input);
		bytestream[2] = fgetc(input);
		bytestream[3] = fgetc(input);

		*(int32_t*) bytestream ^= *(int32_t*) &vec.y;
		fwrite(bytestream, 1, sizeof(int32_t), output);

		bytestream[0] = fgetc(input);
		bytestream[1] = fgetc(input);
		bytestream[2] = fgetc(input);
		bytestream[3] = fgetc(input);

		*(int32_t*) bytestream ^= *(int32_t*) &vec.z;
		fwrite(bytestream, 1, sizeof(int32_t), output);

		bytestream[0] = fgetc(input);
		bytestream[1] = fgetc(input);
		bytestream[2] = fgetc(input);
		bytestream[3] = fgetc(input);

		*(int32_t*) bytestream ^= *(int32_t*) &vec.w;
		fwrite(bytestream, 1, sizeof(int32_t), output);
	}

}