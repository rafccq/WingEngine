#pragma once

#include <stdlib.h>
#include <gccore.h>

// Generate a random number between [0..1)
inline float RandomNorm() {
    return rand() / (float)RAND_MAX;
}

inline float RandRange(float fMin, float fMax) {
    //if ( fMin > fMax ) std::swap( fMin, fMax );
    return ( RandomNorm() * ( fMax - fMin ) ) + fMin;
}

inline guVector RandUnitVec() {
    float x = ( RandomNorm() * 2.0f ) - 1.0f;
    float y = ( RandomNorm() * 2.0f ) - 1.0f;
    float z = ( RandomNorm() * 2.0f ) - 1.0f;

	guVector vec = {x,y,z};
    guVecNormalize(&vec);

    return vec;
}
