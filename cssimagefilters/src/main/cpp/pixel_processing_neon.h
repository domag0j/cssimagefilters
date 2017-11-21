//
// Created by Domagoj Kovacevic on 18/11/2017.
//

#ifndef CSSIMAGEFILTERS_PIXEL_PROCESSING_NEON_H
#define CSSIMAGEFILTERS_PIXEL_PROCESSING_NEON_H
void apply_linear_function_neon(int *pixels, int nr, float slopeR, float slopeG, float slopeB,
                           float interceptR, float interceptG, float interceptB);
#endif //CSSIMAGEFILTERS_PIXEL_PROCESSING_NEON_H
