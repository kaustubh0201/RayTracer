#define HYPATIA_IMPLEMENTATION

#include <stdio.h>
#include <assert.h>
#include <tgmath.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <stdalign.h>

#include "outfile.h"
#include "util.h"
#include "camera.h"
#include "sphere.h"
#include "hitRecord.h"
#include "types.h"
#include "hypatiaINC.h"
#include "ray.h"
#include "material.h"
#include "color.h"
#include "poolAllocator.h"

RGBColorU8 writeColor(CFLOAT r, CFLOAT g, CFLOAT b, int sample_per_pixel){
    CFLOAT scale = 1.0/sample_per_pixel;

    r = sqrt(scale * r);
    g = sqrt(scale * g);
    b = sqrt(scale * b);

    return COLOR_U8CREATE(r, g, b);
}

HitRecord* hittableList(int n, Sphere sphere[n], Ray ray, PoolAlloc * restrict hrAlloc, CFLOAT t_min, CFLOAT t_max){
    HitRecord * r = (HitRecord *) alloc_poolAllocAllocate(hrAlloc);
    HitRecord * h = NULL;

    for(int i = 0; i < n; i++){
        hit(sphere + i, ray, t_min, t_max, r);

        if(r->valid){

            if(h == NULL ){
                h = r;
                r = (HitRecord *) alloc_poolAllocAllocate(hrAlloc);
            }else if(r->distanceFromOrigin < h->distanceFromOrigin){
                alloc_poolAllocFree(hrAlloc, h);
                h = r;
                r = (HitRecord *) alloc_poolAllocAllocate(hrAlloc);
            }
        }
    }    
    return h;
}


RGBColorF ray_c(Ray r, int n, Sphere sphere[n], int depth, PoolAlloc * restrict hrAlloc){
    if(depth <= 0){
        return (RGBColorF){0};
    }

    HitRecord *  rec = hittableList(n, sphere, r, hrAlloc, 0.00001, FLT_MAX);
    if(rec != NULL){
        Ray scattered = {0};
        RGBColorF attenuation = {0};
        
        if(mat_scatter(&r, rec, &attenuation, &scattered)){
            RGBColorF color = ray_c(scattered, n, sphere, depth - 1, hrAlloc);
            color = colorf_multiply(color, attenuation); 

            
            return color;
        }

        return (RGBColorF){0};
    }

    vec3 ud = r.direction;
    vector3_normalize(&ud);
    CFLOAT t = 0.5 * (ud.y + 1.0);
    vec3 inter4;
    vector3_setf3(&inter4, 1.0 - t, 1.0 - t, 1.0 - t);
    vec3 inter3;
    vector3_setf3(&inter3, 0.5 * t, 0.7 * t, 1.0 * t);
    vector3_add(&inter3, &inter4);
    
    return (RGBColorF){
        .r = inter3.x,
        .g = inter3.y,
        .b = inter3.z
    };
}
        
void printProgressBar(int i, int max){
    int p = (int)(100 * (CFLOAT)i/max);

    printf("|");
    for(int j = 0; j < p; j++){
        printf("=");
    }

    for(int j = p; j < 100; j++){
        printf("*");
    }

    if(p == 100){
        printf("|\n");
    }else{
        printf("|\r");
    }
}


int main(int argc, char *argv[]){
    srand(1);

    Camera cam = {0};

    vec3 ip_lookfrom = {
        .x = 3.0,
        .y = 3.0, 
        .z = 2.0
    };

    vec3 ip_lookat = {
        .x = 0.0, 
        .y = 0.0, 
        .z = -1.0
    };

    vec3 ip_up = {
        .x = 0.0, 
        .y = 1.0, 
        .z = 0.0
    };

    CFLOAT ip_vfov = 10.0;
    CFLOAT ip_aspect_ratio = 16.0/9.0;
    CFLOAT ip_aperture = 2.0;
    CFLOAT ip_focusDist = 5.19615242271;
        
    cam_setLookAtCamera(&cam, ip_lookfrom, ip_lookat, ip_up, ip_vfov, ip_aspect_ratio, ip_aperture, ip_focusDist);

    
    //vector3_multiplyf(&randOnDist, cam.lensRadius);


    // ------ testing --------


    if(argc < 2){
        printf("FATAL ERROR: Output file name not provided.\n");
        printf("EXITING ...\n");
        return 0;
    }

    srand(time(NULL));

    printf("Using Hypatia Version:%s\n", HYPATIA_VERSION);

    const CFLOAT aspectRatio = 3.0 / 2.0;
    const int WIDTH = 500;
    const int HEIGHT = (int)(WIDTH/aspectRatio);
    const int SAMPLES_PER_PIXEL = 100;
    const int MAX_DEPTH = 50;
    
    LambertianMat materialGround = {
        .albedo = {.r = 0.8, .g = 0.8, .b = 0.0}
    };

    LambertianMat materialCenter = {
        .albedo = {.r = 0.1, .g = 0.2, .b = 0.5}
    }; 

    DielectricMat materialLeft = {
        .ir = 1.5
    };

    MetalMat materialRight = {
        .albedo = {.r = 0.8, .g = 0.6,.b = 0.2},
        .fuzz = 0.0
    }; 

#define numSpheres 5

    Sphere s[numSpheres] = {
        {
        .center = { .x = 0.0, .y = -100.5, .z = -1.0},
        .radius = 100.0,
        .sphMat = {.matLamb = &materialGround, .matType = LAMBERTIAN },
        }, 
         
        {
        .center = { .x = 0.0, .y = 0.0, .z = -1.0},
        .radius = 0.5,
        .sphMat = {.matLamb = &materialCenter, .matType = LAMBERTIAN },
        }, 
        
        {
        .center = {.x = -1.0, .y = 0.0, .z = -1.0},
        .radius = 0.5,
        .sphMat = {.matDielectric = &materialLeft, .matType = DIELECTRIC },
        },

        {
        .center = {.x = -1.0, .y = 0.0, .z = -1.0},
        .radius = -0.45,
        .sphMat = {.matDielectric = &materialLeft, .matType = DIELECTRIC },
        },

        {
        .center = {.x = 1.0, .y = 0.0, .z = -1.0},
        .radius = 0.5,
        .sphMat = {.matMetal = &materialRight, .matType = METAL },
        }
    };
    

    vec3 lookFrom = {.x = 3.0, .y = 3.0, .z = 2.0};
    vec3 lookAt = {.x = 0.0, .y = 0.0, .z = -1.0};
    vec3 up = {.x = 0.0, .y = 1.0, .z = 0.0};

    CFLOAT distToFocus = 5.19615242271;
    CFLOAT aperture = 2.0;

    Camera c;
    cam_setLookAtCamera(&c, lookFrom, lookAt, up, 20, aspectRatio, aperture, distToFocus);

    Ray r;
    RGBColorF temp;
    
    RGBColorU8* image = (RGBColorU8*) malloc(sizeof(RGBColorF) * HEIGHT * WIDTH);
    PoolAlloc * hrpa = alloc_createPoolAllocator(sizeof(HitRecord) * MAX_DEPTH * SAMPLES_PER_PIXEL * 2, alignof(HitRecord), sizeof(HitRecord)); 

    CFLOAT pcR, pcG, pcB;

    for (int j = HEIGHT - 1; j >= 0; j--){
        for (int i = 0; i < WIDTH; i++){
            
            pcR = pcG = pcB = 0.0;

            for(int k = 0; k < SAMPLES_PER_PIXEL; k++){
                CFLOAT u = ((CFLOAT)i + util_randomFloat(0.0, 1.0)) / (WIDTH - 1);
                CFLOAT v = ((CFLOAT)j + util_randomFloat(0.0, 1.0)) / (HEIGHT - 1);
                r = cam_getRay(&c, u, v);
              
                temp = ray_c(r, numSpheres, s, MAX_DEPTH, hrpa);

                pcR += temp.r;
                pcG += temp.g;
                pcB += temp.b;   
            }

            image[i + WIDTH * (HEIGHT - 1 - j)] = writeColor(pcR, pcG, pcB, SAMPLES_PER_PIXEL);
            alloc_poolAllocFreeAll(hrpa);
        }

        printProgressBar(HEIGHT - 1 - j, HEIGHT - 1);
    }

    writeToPPM(argv[1], WIDTH, HEIGHT, image);
    free(image);
    alloc_freePoolAllocator(hrpa);

    return 0;
}
