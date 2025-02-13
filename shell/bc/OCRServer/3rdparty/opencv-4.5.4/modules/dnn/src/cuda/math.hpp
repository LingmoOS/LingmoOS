// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.

#ifndef OPENCV_DNN_SRC_CUDA_MATH_HPP
#define OPENCV_DNN_SRC_CUDA_MATH_HPP

#include <cuda_runtime.h>
#include <cuda_fp16.h>

namespace cv { namespace dnn { namespace cuda4dnn { namespace csl { namespace device {

    template <class T> __device__ T abs(T val) { return (val < T(0) ? -val : val); }
    template <> inline __device__ float abs(float val) { return fabsf(val); }
    template <> inline __device__ double abs(double val) { return fabs(val); }

    template <class T> __device__ T exp(T val);
#if !defined(__CUDA_ARCH__) || (__CUDA_ARCH__ >= 530)
    template <> inline __device__ __half exp(__half val) { return hexp(val); }
#endif
    template <> inline __device__ float exp(float val) { return expf(val); }
    template <> inline __device__ double exp(double val) { return ::exp(val); }

    template <class T> __device__ T expm1(T val);
#if !defined(__CUDA_ARCH__) || (__CUDA_ARCH__ >= 530)
    template <> inline __device__ __half expm1(__half val) { return hexp(val) - __half(1); }
#endif
    template <> inline __device__ float expm1(float val) { return expm1f(val); }
    template <> inline __device__ double expm1(double val) { return ::expm1(val); }

    template <class T> __device__ T max(T x, T y) { return (x > y ? x : y); }
    template <> inline __device__ float max(float x, float y) { return fmaxf(x, y); }
    template <> inline __device__ double max(double x, double y) { return fmax(x, y); }

    template <class T> __device__ T min(T x, T y) { return (x > y ? y : x); }
    template <> inline __device__ float min(float x, float y) { return fminf(x, y); }
    template <> inline __device__ double min(double x, double y) { return fmin(x, y); }

    template <class T> __device__ T log1p(T val);
#if !defined(__CUDA_ARCH__) || (__CUDA_ARCH__ >= 530)
    template <> inline __device__ __half log1p(__half val) { return hlog(__half(1) + val); }
#endif
    template <> inline __device__ float log1p(float val) { return log1pf(val); }

    template <class T> __device__ T log1pexp(T val);
#if !defined(__CUDA_ARCH__) || (__CUDA_ARCH__ >= 530)
    template <> inline __device__ __half log1pexp(__half val) {
        if (val <= __half(-4.0))
            return exp(val);
        else if (val <= __half(8.0))
            return log1p(exp(val));
        else if (val <= __half(8.7))
            return val + exp(-val);
        else
            return val;
    }
#endif
    template <> inline __device__ float log1pexp(float val) {
        if (val <= -20)
            return expf(val);
        else if (val <= 9.0)
            return log1pf(expf(val));
        else if (val <= 14.6)
            return val + exp(-val);
        else
            return val;
    }
    template <> inline __device__ double log1pexp(double val) {
        if (val <= -37)
            return exp(val);
        else if (val <= 18)
            return log1p(exp(val));
        else if (val <= 33.3)
            return val + exp(-val);
        else
            return val;
    }

    template <class T> __device__ T tanh(T val);
#if !defined(__CUDA_ARCH__) || (__CUDA_ARCH__ >= 530)
    template <> inline __device__ __half tanh(__half val) { return tanhf(val); }
#endif
    template <> inline __device__ float tanh(float val) { return tanhf(val); }
    template <> inline __device__ double tanh(double val) { return ::tanh(val); }

    template <class T> __device__ T pow(T val, T exp);
#if !defined(__CUDA_ARCH__) || (__CUDA_ARCH__ >= 530)
    template <> inline __device__ __half pow(__half val, __half exp) { return powf(val, exp); }
#endif
    template <> inline __device__ float pow(float val, float exp) { return powf(val, exp); }
    template <> inline __device__ double pow(double val, double exp) { return ::pow(val, exp); }

    template <class T> __device__ T sqrt(T val);
#if !defined(__CUDA_ARCH__) || (__CUDA_ARCH__ >= 530)
    template <> inline __device__ __half sqrt(__half val) { return hsqrt(val); }
#endif
    template <> inline __device__ float sqrt(float val) { return sqrtf(val); }
    template <> inline __device__ double sqrt(double val) { return ::sqrt(val); }

    template <class T> __device__ T rsqrt(T val);
#if !defined(__CUDA_ARCH__) || (__CUDA_ARCH__ >= 530)
    template <> inline __device__ __half rsqrt(__half val) { return hrsqrt(val); }
#endif
    template <> inline __device__ float rsqrt(float val) { return rsqrtf(val); }
    template <> inline __device__ double rsqrt(double val) { return ::rsqrt(val); }

    template <class T> __device__ T sigmoid(T val) { return T(1) / (T(1) + exp(-val)); }

    template <class T> __device__ T clamp(T value, T lower, T upper) { return min(max(value, lower), upper); }

    template <class T> __device__ long lround(T value);
    template <> inline __device__ long lround(double value) { return ::lround(value); }
    template <> inline __device__ long lround(float value) { return lroundf(value); }

    template <class T> __device__ T round(T value);
    template <> inline __device__ double round(double value) { return ::round(value); }
    template <> inline __device__ float round(float value) { return roundf(value); }
#if !defined(__CUDA_ARCH__) || (__CUDA_ARCH__ >= 530)
    template <> inline __device__ __half round(__half value) { return hrint(value); }
#endif

    template <class T> __device__ T ceil(T value);
    template <> inline __device__ double ceil(double value) { return ::ceil(value); }
    template <> inline __device__ float ceil(float value) { return ceilf(value); }
#if !defined(__CUDA_ARCH__) || (__CUDA_ARCH__ >= 530)
    template <> inline __device__ __half ceil(__half value) { return hceil(value); }
#endif

    template <class T> __device__ T mul_ftz(T x, T y) { return x * y; }
    template <> inline __device__ float mul_ftz(float x, float y) {
        float result;
        asm("mul.ftz.f32 %0, %1, %2;" : "=f"(result) : "f"(x), "f"(y));
        return result;
    }

    template <class T> __device__ T fast_divide(T x, T y) { return x / y; }
    template <> inline __device__ float fast_divide(float x, float y) { return __fdividef(x, y); }

    template <class T> __device__ T fast_divide_ftz(T x, T y) { return fast_divide(x, y); }
    template <> inline __device__ float fast_divide_ftz(float x, float y) {
        float result;
        asm("div.approx.ftz.f32 %0, %1, %2;" : "=f"(result) : "f"(x), "f"(y));
        return result;
    }

    template <class T> __device__ T fast_exp(T value) { return exp(value); }
    template <> inline __device__ float fast_exp(float value) { return __expf(value); }

    template <class T> __device__ T fast_sigmoid(T value) { return sigmoid(value); }
    template <> inline __device__ float fast_sigmoid(float value) { return __fdividef(1, 1 + __expf(-value)); }

}}}}} /* namespace cv::dnn::cuda4dnn::csl::device */

#endif /* OPENCV_DNN_SRC_CUDA_MATH_HPP */
