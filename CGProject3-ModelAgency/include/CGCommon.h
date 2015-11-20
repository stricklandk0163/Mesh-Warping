/*
 * CGCommon.h
 *
 * A general header file for use with computer graphics projects that
 * includes and defines a lot of commonly used stuff.
 */

#ifndef CGCOMMON_H_
#define CGCOMMON_H_

// Ensure PI and other constants get defined on microsoft compiler
#ifdef _MSC_VER
    #define _USE_MATH_DEFINES
#endif

// Needed throughout most C/C++ programs
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <csignal>

// STL includes needed pretty much everywhere
#include <string>

// Fixed precision integer types (and the macros for use with printf)
#ifndef __STDC_FORMAT_MACROS
    #define __STDC_FORMAT_MACROS
#endif

#ifdef _MSC_VER
    // Need a special header for Microsoft compiler
    #include "msinttypes.h"
#else
    #include <inttypes.h>
#endif

// This must happen before glu on windows (both MS and MinGW)
#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
#endif

// Some handy utilities for OpenGL
#ifdef __APPLE__
    #include <OpenGL/glu.h>
#else
    #include <GL/glu.h>
#endif

// Because msvc is silly
#ifndef GL_MULTISAMPLE
    #define GL_MULTISAMPLE                    0x809D
#endif

// Because these should exist (but don't on many systems)
#ifndef MAX
#define MAX(X, Y) ((X)>(Y)?(X):(Y))
#endif

#ifndef MIN
#define MIN(X, Y) ((X)<(Y)?(X):(Y))
#endif

// A nice default Field-of-view for projections
// (Corresponds to 35mm focal length on a 32mm film size)
#define GENERAL_FOV      49.13

// Easy macro for checking and printing the OpenGL error state
// (and possibly aborting when one has occured)
#define CG_BREAK_ON_GL_ERROR   false
#define CG_GL_ERROR_CHECK   { \
        int lGLErrorCode = glGetError(); \
        if(lGLErrorCode != GL_NO_ERROR) \
        { \
            printf("OpenGL Error @ %s(%d): %s\n", __FILE__, __LINE__, gluErrorString(lGLErrorCode)); \
            fflush(stdout); \
            if(CG_BREAK_ON_GL_ERROR) ::raise(SIGABRT); \
        } \
    }

/* In-line vector math */
// Implementation of basic 3-element vector math operations as macros for efficiency and easy use.
// All UPPER-CASE letters should be pre-allocated, 3-element vectors.  Lower-case letters are scalers.
// These macros should work with any floating point types (best if all parameters are same type).

// Compute the difference of two 3-component arrays (points or vectors) C = A - B
#define VEC_DIFF(A, B, C)	((C)[0] = (A)[0] - (B)[0]); \
                            ((C)[1] = (A)[1] - (B)[1]); \
                            ((C)[2] = (A)[2] - (B)[2])

// 'returns' length of 3-element vector A
#define VEC_LEN_SQ(A)		((A)[0]*(A)[0] + (A)[1]*(A)[1] + (A)[2]*(A)[2])

// Divides components of 3-element vector B by scaler l ... normally l=sqrt(VEC_LEN_SQ(B))
#define NORMALIZE(B, l)     ((B)[0]/=(l)); ((B)[1]/=(l)); ((B)[2]/=(l))

// An easier to use (and safer) but potentially less efficient form of NORMALIZE
#define SAFE_NORMALIZE(B) {\
        double myLen = sqrt(VEC_LEN_SQ((B))); \
        if(myLen > 1e-10) { (B)[0]/=(myLen); (B)[1]/=(myLen); (B)[2]/=(myLen); } \
        else { printf("Trying to normalize a zero-length vector: (%.4f, %.4f, %.4f), len = %.8f\n", \
                      (B)[0], (B)[1], (B)[2], myLen); fflush(stdout); } \
    }

// 'returns' scalar dot product of 3-element vectors A and B
#define DOTPROD(A, B)		((A)[0]*(B)[0] + (A)[1]*(B)[1] + (A)[2]*(B)[2])

// Computes cross-product of 3-element vectors A and B and stores in 3-element vector C
#define CROSSPROD(A, B, C)	((C)[0] = (A)[1]*(B)[2] - (A)[2]*(B)[1]); \
                            ((C)[1] = (A)[2]*(B)[0] - (A)[0]*(B)[2]); \
                            ((C)[2] = (A)[0]*(B)[1] - (A)[1]*(B)[0])

#endif /* CGCOMMON_H_ */
