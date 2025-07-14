#pragma once

#define FFT_API __declspec(dllexport)

extern "C" {
	FFT_API
		void fft_radix2(
		double* Re, 
		double* Im, 
		size_t fft_len, 
		double* Re_o, 
		double* Im_o);
}

