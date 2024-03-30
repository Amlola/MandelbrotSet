#pragma once


#include <SFML/Graphics.hpp>
#include <immintrin.h>
#include "stdlib.h"
#include <x86intrin.h>


#define RUN_TEST(FUNC)                                                      \
    uint64_t start_ticks = __rdtsc();                                       \
        for (size_t i = 0; i < ITERATIONS; i++)                             \
            {                                                               \
            FUNC(set);                                                      \
            }                                                               \
    uint64_t end_ticks   = __rdtsc();                                       \
    printf("CPU ticks = %ld \n", (end_ticks - start_ticks) / ITERATIONS);   \


const int   MAX_ITERATIONS = 256;
const float MAX_RADIUS     = 100.f;
const int   WIDTH          = 800;
const int   HEIGHT         = 600;
const float D_X            = 1 / (float)WIDTH;
const float D_Y            = 1 / (float)WIDTH;
const float HALF_WIDTH     = (float)WIDTH  / 2;
const float HALF_HEIGHT    = (float)HEIGHT / 2;
const int   VECTOR_SIZE    = 8;
const size_t ITERATIONS    = 80;


enum Mode 
    {
    BY_PIXELS  =  0,
    BY_VECTOR  =  1,
    BY_SIMD    =  2
    };


struct MandelBrot 
    {
    float scale;
    float x_offset;
    float y_offset;
    Mode mode;
    uint32_t* pixels_array;
    };


void BuildingImageByNaive(MandelBrot* set);

void GetMandelBrotSet(MandelBrot* set);

void MandelBrotInit(MandelBrot* set); 

inline int KeyboardStatus(MandelBrot* set, sf::Event &event); 

inline void GetPixels(MandelBrot* set, size_t index, size_t count);

void BuildingImageByArray(MandelBrot* set);

void BuildingImageBySimd(MandelBrot* set);

inline void GetFPS(sf::Clock &clock, sf::Text &text, MandelBrot* set);

void ChooseRunTest(MandelBrot* set);