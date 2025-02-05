//
// Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//


#pragma once

#include <cuda.h>
#include <cuda_runtime.h>

#include <iostream>
#include <sstream>
#include <stdexcept>

#include <OptiXToolkit/CuOmmBaking/CuOmmBaking.h>

namespace cuOmmBaking {

class Exception : public std::runtime_error
{
  public:
    explicit Exception( Result result, const char* msg )
        : std::runtime_error( msg )
        , m_result( result )
    {
    }

    explicit Exception( Result result, const std::string& msg )
        : std::runtime_error( msg )
        , m_result( result )
    {
    }

    Result getResult() const
    {
        return m_result;
    }

  private:
    Result m_result = Result::SUCCESS;
};


//------------------------------------------------------------------------------
//
// Assertions
//
//------------------------------------------------------------------------------

#define OMM_ASSERT( cond )                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        if( !( cond ) )                                                                                                \
        {                                                                                                              \
            std::stringstream ss;                                                                                      \
            ss << __FILE__ << " (" << __LINE__ << "): " << #cond;                                                      \
            throw Exception( ss.str().c_str() );                                                                       \
        }                                                                                                              \
    } while( 0 )


#define OMM_ASSERT_MSG( cond, msg )                                                                                    \
    do                                                                                                                 \
    {                                                                                                                  \
        if( !( cond ) )                                                                                                \
        {                                                                                                              \
            std::stringstream ss;                                                                                      \
            ss << ( msg ) << ": " << __FILE__ << " (" << __LINE__ << "): " << #cond;                                   \
            throw Exception( ss.str().c_str() );                                                                       \
        }                                                                                                              \
    } while( 0 )


//------------------------------------------------------------------------------
//
// CUDA error-checking
//
//------------------------------------------------------------------------------

inline void checkCudaError( cudaError_t error, const char* expr, const char* /*file*/, unsigned int /*line*/ )
{
    if( error != cudaSuccess )
    {
        std::stringstream ss;
        ss << "CUDA call (" << expr << " ) failed with error: '" << cudaGetErrorString( error ) << "' (" __FILE__ << ":"
           << __LINE__ << ")\n";
        throw Exception( Result::ERROR_CUDA, ss.str().c_str() );
    }
}

// A non-throwing variant for use in destructors.
inline void checkCudaErrorNoThrow( cudaError_t error, const char* expr, const char* file, unsigned int line ) noexcept
{
    if( error != cudaSuccess )
    {
        std::cerr << "CUDA call (" << expr << " ) failed with error: '" << cudaGetErrorString( error ) << "' (" << file
                  << ":" << line << ")\n";
        std::terminate();
    }
}

inline void checkCudaError( CUresult result, const char* expr, const char* file, unsigned int line )
{
    if( result != CUDA_SUCCESS )
    {
        const char* errorStr;
        cuGetErrorString( result, &errorStr );
        std::stringstream ss;
        ss << "CUDA call (" << expr << " ) failed with error: '" << errorStr << "' (" << file << ":" << line << ")\n";
        throw Exception( Result::ERROR_CUDA, ss.str().c_str() );
    }
}

// A non-throwing variant for use in destructors.
inline void checkCudaErrorNoThrow( CUresult result, const char* expr, const char* file, unsigned int line ) noexcept
{
    if( result != CUDA_SUCCESS )
    {
        const char* errorStr;
        cuGetErrorString( result, &errorStr );
        std::cerr << "CUDA call (" << expr << " ) failed with error: '" << errorStr << "' (" << file << ":" << line << ")\n";
        std::terminate();
    }
}

#ifndef NDEBUG

#define OMM_CUDA_CHECK( call )                                                              \
    do {                                                                                    \
        cuOmmBaking::checkCudaError( call, #call, __FILE__, __LINE__ );                       \
        cuOmmBaking::checkCudaError( cudaDeviceSynchronize() , #call, __FILE__, __LINE__ );   \
    } while(false);

#define OMM_CUDA_CHECK_NOTHROW( call )                                                      \
    do {                                                                                    \
        cuOmmBaking::checkCudaErrorNoThrow( call, #call, __FILE__, __LINE__ );                \
        cuOmmBaking::checkCudaError( cudaDeviceSynchronize() , #call, __FILE__, __LINE__ );   \
    } while(false);

#else

#define OMM_CUDA_CHECK( call ) cuOmmBaking::checkCudaError( call, #call, __FILE__, __LINE__ )
#define OMM_CUDA_CHECK_NOTHROW( call ) cuOmmBaking::checkCudaErrorNoThrow( call, #call, __FILE__, __LINE__ )

#endif

}  // end namespace cuOmmBaking
