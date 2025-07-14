#include <iostream>
#include <complex>
#include <vector>
#include <chrono>
#include "fft.h"

using Complex = std::complex<double>; 
constexpr double HINT_PI = 3.1415926535897932384626433832795;
constexpr double HINT_2PI = HINT_PI * 2;

// ����������
template <typename T>
void binary_inverse_swap(T& ary, size_t len)
{
    size_t i = 0;
    for (size_t j = 1; j < len - 1; j++)
    {
        size_t k = len >> 1;
        i ^= k;
        while (k > i)
        {
            k >>= 1;
            i ^= k;
        };
        if (j < i)
        {
            std::swap(ary[i], ary[j]);
        }
    }
}


void fft_radix2_rec(std::vector<Complex>& input)
{
    size_t len = input.size();
    if (len <= 1)
    {
        return;
    }
    if (len == 2)
    {
        Complex tmp1 = input[0];
        Complex tmp2 = input[1];
        input[0] = tmp1 + tmp2;
        input[1] = tmp1 - tmp2;
        return;
    }
    size_t half_len = len / 2;
    std::vector<Complex> ary1(half_len); // ��һ�������У��洢ż���±����
    std::vector<Complex> ary2(half_len); // �ڶ��������У��洢�����±����
    for (size_t i = 0; i < half_len; i++)
    {
        ary1[i] = input[i * 2];
        ary2[i] = input[i * 2 + 1];
    }
    fft_radix2_rec(ary1);
    fft_radix2_rec(ary2);
    Complex omega(1, 0);
    Complex unit = std::polar<double>(1.0, -HINT_2PI / len); // ��λ��
    for (size_t i = 0; i < half_len; i++)
    {
        Complex tmp1 = ary1[i];
        Complex tmp2 = ary2[i] * omega;
        input[i] = tmp1 + tmp2;
        input[i + half_len] = tmp1 - tmp2;

        omega *= unit;
    }
} 

// ��2����fft
void fft_radix2(Complex* ary, size_t fft_len)
{
    binary_inverse_swap(ary, fft_len); // �Ƚ��ж��������򽻻�
    for (size_t rank = 1, gap; rank < fft_len; rank *= 2)
    {
        gap = rank * 2;                                                               // rank������һ��fft�ĳ���,gapΪ����fft����
        Complex unit_omega = std::polar<double>(1.0, (- 1) * HINT_2PI / gap);         // ��λ��
        for (size_t begin = 0; begin < fft_len; begin += gap)                         // beginÿ����Ծ�ĳ���,������Ϊgap
        {
            Complex omega(1, 0);
            for (size_t pos = begin; pos < begin + rank; pos++)
            {
                Complex tmp1 = ary[pos];
                Complex tmp2 = ary[pos + rank] * omega;
                ary[pos] = tmp1 + tmp2;
                ary[pos + rank] = tmp1 - tmp2;
                omega *= unit_omega;
            }
        }
    }
} 

void fft_radix2(double* Re, double* Im, size_t fft_len, double* Re_o, double* Im_o)
{
    Complex* ary = new Complex[fft_len];
    for (size_t i = 0; i < fft_len; i++)
    {
        ary[i] = std::complex <double>(Re[i], Im[i]);
       /* std::cout << ary[i] << std::endl;*/
    }

    binary_inverse_swap(ary, fft_len); // �Ƚ��ж��������򽻻�
    for (size_t rank = 1, gap; rank < fft_len; rank *= 2)
    {
        gap = rank * 2;                                                               // rank������һ��fft�ĳ���,gapΪ����fft����
        Complex unit_omega = std::polar<double>(1.0, (-1) * HINT_2PI / gap);         // ��λ��
        for (size_t begin = 0; begin < fft_len; begin += gap)                         // beginÿ����Ծ�ĳ���,������Ϊgap
        {
            Complex omega(1, 0);
            for (size_t pos = begin; pos < begin + rank; pos++)
            {
                Complex tmp1 = ary[pos];
                Complex tmp2 = ary[pos + rank] * omega;
                ary[pos] = tmp1 + tmp2;
                ary[pos + rank] = tmp1 - tmp2;
                omega *= unit_omega;
            }
        }
    }
    for (size_t i = 0; i < fft_len; i++)
    {
        //std::cout << ary[i] << std::endl;
        Re_o[i] = ary[i].real();
        Im_o[i] = ary[i].imag();
    }
    /*delete[] ary;*/
}

// 
//void DIT_FFT(double* Re, double* Im, size_t fft_len, double* Re_o, double* Im_o ,size_t M)
//{
//    Complex* ary = new Complex[fft_len];
//    for (size_t i = 0; i < fft_len; i++)
//    {
//        ary[i] = std::complex <double>(Re[i], Im[i]);
//        /* std::cout << ary[i] << std::endl;*/
//    }
//    binary_inverse_swap(ary, fft_len); // �Ƚ��ж��������򽻻�
//
//    for (int L = 1; L <= M; L++)
//    {
//        int B = pow(2, L - 1);
//        for (int J = 0; J <= B - 1; J++)
//        {
//            
//            for (int k = J; k < fft_len - 1; k = k + pow(2, L))
//            {
//                Complex tmp1 = ary[k];
//                Complex tmp2 = ary[k + B] * omega;
//                ary[k] = tmp1 + tmp2;
//                ary[k + B] = tmp1 - tmp2;
//            }
//        }
//    }
//
//}

int main()
{
    size_t fft_len = 2046; // �任����Ϊ2^n
    double Re[2046];
    double Im[2046] = { 0 };
    for (int i = 0; i < fft_len; i++)          //i�൱�ڱ��n = 0,1,....,7
    {
        Re[i] = sin((2 * HINT_PI * i) / 2046) + 0.5 * sin((2 * HINT_PI * 2 * i) / 2046 + 3 * HINT_PI / 4);
        //��ʱ���x(n) i���൱��n  1000��2000
    }
    double Re_o[2046] = { 0 };
    double Im_o[2046] = { 0 };

    auto start = std::chrono::high_resolution_clock::now();  //��¼��ʼ����ʱ��
    fft_radix2(Re, Im, 2046, Re_o, Im_o);
    auto end = std::chrono::high_resolution_clock::now(); //��¼����ʱ��
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "��������ʱ�䣺" << duration.count() << " ����" << std::endl;

    //for (size_t i = 0; i < fft_len; i++)
    //{
    //    std::cout << "(" << Re_o[i] << "," << Im_o[i] << ")" << std::endl;
    //}
    return 0;
}
