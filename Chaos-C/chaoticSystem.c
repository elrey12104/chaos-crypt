#include<stdlib.h>
#include<stdint.h>
#include<stdio.h>
#include<stdbool.h>
#include"vector.h"

static void heunsPlusOne(matrix* params, vector* current, float stepSize) {
	vector xvec = { current->x,0.0,0.0 };
	vector uxv = cprod(xvec, *current);
	vector Ax = mprod(*params, *current);

	*current = vsum(*current, sprod(stepSize / 2, vsum(*current, sprod(stepSize + 1, vsum(Ax, uxv)))));
	
}

static void initParams(matrix* params, float rho, float sigma, float beta) {
	vector v0 = {
		-1*sigma,
		rho,
		0.0
	};
	vector v1 = {
		sigma,
		-1,
		0
	};

	vector v2 = {
		0,
		0,
		-1*beta
	};
	params->v0=v0;
	params->v1=v1;
	params->v2=v2;
}

void numericSolve(FILE* input, FILE* output, size_t messageLength, bool isCoupled) {

	vector vec = { 1.0,1.0,1.0 };
	
	matrix paramsVal={vec,vec,vec};
	matrix* params = &paramsVal;
	
	initParams(params,28,10,8/3);
	
	if(input==NULL) {
		for (int i=0; i<messageLength; i++) {
		heunsPlusOne(params, &vec, 1);
		fprintf(output, "%.4F,%.4F,%.4F\n", vec.x, vec.y, vec.z);
		} 
	} else if(isCoupled) {
		for(int i=0; i<messageLength; i++){
			fscanf(input, "%F\n", &(vec.x));
			heunsPlusOne(params, &vec, 1);
			fprintf(output, "%.4F,%.4F,%.4F\n", vec.x, vec.y, vec.z);
		}
	} else {
			char drho,dsigma,dbeta;
		for(int i=0; i<messageLength; i++){
			drho = fgetc(input);
			dsigma = fgetc(input);
			dbeta = fgetc(input);
			for (int j=0; j<8; j++) {
				initParams(params,28+28*((drho >> j) & 1),10+10*((dsigma >> j) & 1),8/3+8/3*((dbeta >> j) & 1));
				heunsPlusOne(params, &vec, 1);
				fprintf(output, "%.4F\n", vec.x);
			}


			

		}
	}
}