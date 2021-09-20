#include <stdlib.h>
#include <stdbool.h>

#include "util.h"

CFLOAT util_ldClamp(CFLOAT c, CFLOAT lower, CFLOAT upper){
    if(c <= lower){
        return lower;
    }

    if(c >= upper){
        return upper;
    }

    return c;
}

/*
int randomRange(int lower, int upper){
    return (rand() % (upper - lower + 1)) + lower;
}
*/

vec3 util_randomInUnitSphere(){
    CFLOAT x, y, z;
    while(true){
        x = util_randomLD(0.0, 1.0);
        y = util_randomLD(0.0, 1.0);
        z = util_randomLD(0.0, 1.0);

        if(x*x + y*y + z*z >= 1){
            continue;
        }else{
            break;
        }
    }

    return (vec3){
        .x = x,
        .y = y,
        .z = z
    };
}

CFLOAT util_randomLD(CFLOAT lower, CFLOAT upper){
    CFLOAT scale = rand() / (CFLOAT) RAND_MAX;
    return scale * (upper - lower) + lower;
}
