//
// Created by Domagoj Kovacevic on 13/11/2017.
//

#ifndef CSSIMAGEFILTERS_PIXEL_PROCESSING_CPP_H
#define CSSIMAGEFILTERS_PIXEL_PROCESSING_CPP_H

#include <jni.h>


#define PORTER_DUFF_NORMAL 0
#define PORTER_DUFF_MULTIPLY 1
#define PORTER_DUFF_SCREEN 2
#define PORTER_DUFF_OVERLAY 3
#define PORTER_DUFF_DARKEN 4
#define PORTER_DUFF_LIGHTEN 5
#define PORTER_DUFF_COLOR_DODGE 6
#define PORTER_DUFF_COLOR_BURN 7
#define PORTER_DUFF_HARD_LIGHT 8
#define PORTER_DUFF_SOFT_LIGHT 9
#define PORTER_DUFF_DIFFERENCE 10
#define PORTER_DUFF_EXCLUSION 11

/**
 *  Linear function P=P*slope+intercept, where P is R, G, B
 * @param pixels - a pointer to array of BGR 24 bit pixels
 * @param nr - number of pixels
 * @param slopeR Red slope
 * @param slopeG Green slope
 * @param slopeB Blue slope
 * @param interceptR Red intercept
 * @param interceptG Green intercept
 * @param interceptB Blue intercept
 */
void apply_linear_function(int *pixels, int nr, float slopeR, float slopeG, float slopeB,
                           float interceptR, float interceptG, float interceptB);

/**
 *
 matrix operations - R = R*Crr + G*Crg + B*Crb etc..
 */
void apply_matrix_function(int *pixels, int nr, float Crr, float Crg, float Crb,
                                                float Cgr, float Cgg, float Cgb,
                                                float Cbr, float Cbg, float Cbb);

void apply_blend_color(int *pixels, int nr, float redBackdrop, float greenBackdrop, float blueBackdrop, float alpha, int porterDuffMode);
void apply_blend_bitmap(int *pixels_target, int *pixels_to_blend, int nr, int porterDuffMode);

#endif //CSSIMAGEFILTERS_PIXEL_PROCESSING_CPP_H
