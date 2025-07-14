#pragma once


#define FFTW_API __declspec(dllexport)



extern "C" {
	FFTW_API void fftw(int n, int fs, double* in, double* frequence, double* amplitude);
}

