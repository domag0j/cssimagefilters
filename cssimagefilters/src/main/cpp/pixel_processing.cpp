//
// Created by Domagoj Kovacevic on 13/11/2017.
//

#include <math.h>
#include <android/log.h>
#include <stdlib.h>
#include "pixel_processing.h"
#include "pixel_processing_neon.h"
#include <cpu-features.h>
#define  LOG_TAG    "cssimagefilters"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

//it is actualy kept as ABGR????
#define EXTRACT_B(pixel)  ((pixel >> 16) & 0xFF)
#define EXTRACT_G(pixel)  ((pixel >> 8) & 0xFF)
#define EXTRACT_R(pixel)  (pixel & 0xFF)
#define EXTRACT_A(pixel)  ((pixel >> 24) & 0xFF)
#define COMBINE_KEEP_ALPHA(pixel, r, g, b) ((pixel&0xFF000000)|((b<<16))|((g<<8))|(r & 0xFF))
#define TO_FLOAT(pixel) ((float)pixel)
#define TO_FLOAT_NORMALIZED(pixel) (((float)pixel)/255.0f)
#define TO_INT(fpixel) (int) (fpixel+0.5f)
#define TO_INT_DENORMALIZE(fpixel) ((int) (fpixel*255.f+0.5f))
#define LIMIT(pixel) (pixel>255? 255 : (pixel <0? 0 : pixel))


bool has_NEON(){
    if (android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM &&
        (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
        return true;
    } else {
        return false;
    }
}

typedef int (*pd_func)(int, int);

int pd_normal(int Cb, int Cs){
    return Cs;
}

int pd_multiply(int Cb, int Cs){
    return (Cb * Cs)>>8;
}

int pd_screen(int Cb, int Cs){
    int r = Cb + Cs - ((Cb * Cs)>>8);
    if (r>255) r = 255;
    if (r<0) r = 0;
    return r;
}

int pd_darken(int Cb, int Cs){
    return Cb < Cs? Cb : Cs;
}

int pd_lighten(int Cb, int Cs){
    return Cb > Cs? Cb : Cs;
}


int pd_color_dodge(int Cbi, int Csi){
    if (Cbi==0) return 0;
    if (Csi==255) return 255;

    float Cb = Cbi / 255.0f;
    float Cs = Csi / 255.0f;

    float r = Cb / (1.0f - Cs);
    r = r >1.0f ? 1.0f : r;
    return (int)(r*255.0f+.5f);
}



int pd_color_burn(int Cbi, int Csi){
    if (Csi==0) return 0;
    if (Cbi==255) return 255;

    float Cb = Cbi / 255.0f;
    float Cs = Csi / 255.0f;
    float r = (1.0f -Cb) / Cs;
    if(r>1.0f) r = 1.0f;
    r = 1 - r;
    return (int)(r*255.0f+.5f);
}


int pd_hard_light(int Cb, int Cs){
    if (Cs <= 127){
        int r = (2 * Cb * Cs)>>8;
        if (r>255) r = 255;
        return r;
    }else{
        int r = 2 * Cb  + 2* Cs - ((2 * Cb * Cs)>>8) -255;
        if (r>255) r = 255;
        if (r<0) r = 0;
        return r;
    }
}

int pd_overlay(int Cb, int Cs){
    return pd_hard_light(Cs, Cb);
}

int pd_soft_light(int Cbi, int Csi){
    float Cb = Cbi / 255.0f;
    float Cs = Csi / 255.0f;
    float r;
    if (Cs <= 0.5f){
        r = Cb - (1.f - 2.f * Cs) * Cb * (1.f - Cb);
    }else{
        float d;
        if (Cb < .25f){
            d = ((16.f * Cb - 12.f) * Cb + 4.f) * Cb;
        }else{
            d = sqrtf(Cb);
        }
        r =   Cb + (2.f * Cs - 1) * (d - Cb);
    }

    int ri = (int)(r * 255.0f+.5f);
    if (ri>255) ri = 255;
    if (ri<0) ri = 0;
    return ri;
}

int pd_difference(int Cb, int Cs){
    return abs(Cb-Cs);
}

int pd_exclusion(int Cb, int Cs){
    int r = Cb + Cs - ((2 * Cb * Cs)>>8);
    if (r>255) r = 255;
    if (r<0) r = 0;
    return r;
}




pd_func get_pd_function(int pdMode){
    switch(pdMode){
        case PORTER_DUFF_NORMAL:
            return pd_normal;
        case PORTER_DUFF_MULTIPLY:
            return pd_multiply;
        case PORTER_DUFF_SCREEN:
            return pd_screen;
        case PORTER_DUFF_OVERLAY:
            return pd_overlay;
        case PORTER_DUFF_DARKEN:
            return pd_darken;
        case PORTER_DUFF_LIGHTEN:
            return pd_lighten;
        case PORTER_DUFF_COLOR_DODGE:
            return pd_color_dodge;
        case PORTER_DUFF_COLOR_BURN:
            return pd_color_burn;
        case PORTER_DUFF_HARD_LIGHT:
            return pd_hard_light;
        case PORTER_DUFF_SOFT_LIGHT:
            return pd_soft_light;
        case PORTER_DUFF_DIFFERENCE:
            return pd_difference;
        case PORTER_DUFF_EXCLUSION:
            return pd_exclusion;
        default:
            return nullptr;
    }
}



void apply_blend_color(int *pixels, int nr, float redBackdrop, float greenBackdrop, float blueBackdrop, float alpha, int porterDuffMode)
{
    auto pixel_function = get_pd_function(porterDuffMode);
    if (pixel_function== nullptr) return;
    int alphai = (int)(255.0f *alpha +.5f);
    auto inv_alpha = 255- alphai;
    int redBackdropI = (int)(255.0f *redBackdrop +.5f);
    int greenBackdropI = (int)(255.0f *greenBackdrop +.5f);
    int blueBackdropI = (int)(255.0f *blueBackdrop +.5f);

    while (nr--) {
        int pixel = *pixels;
        int r = EXTRACT_R(pixel);
        int g = EXTRACT_G(pixel);
        int b = EXTRACT_B(pixel);
        r = r * inv_alpha + alphai * pixel_function(redBackdropI, r);
        g = g * inv_alpha + alphai * pixel_function(greenBackdropI, g);
        b = b * inv_alpha + alphai * pixel_function(blueBackdropI, b);
        int ri =r>>8;
        int gi = g>>8;
        int bi = b>>8;
        *pixels = COMBINE_KEEP_ALPHA(pixel, ri, gi, bi);
        pixels ++;

    }
}

void apply_blend_bitmap(int *pixels_target, int *pixels_to_blend, int nr, int porterDuffMode)
{
    auto pixel_function = get_pd_function(porterDuffMode);
    if (pixel_function== nullptr) return;
    while (nr--) {
        int pixel = *pixels_target;
        int pixel_to_blend = *pixels_to_blend;
        int r = EXTRACT_R(pixel);
        int g = EXTRACT_G(pixel);
        int b = EXTRACT_B(pixel);

        int redBackdrop = EXTRACT_R(pixel_to_blend);
        int greenBackdrop = EXTRACT_G(pixel_to_blend);
        int blueBackdrop = EXTRACT_B(pixel_to_blend);
        int alpha = EXTRACT_A(pixel_to_blend);
        int inv_alpha = 255 - alpha;

        r = r * inv_alpha + alpha * pixel_function(redBackdrop, r);
        g = g * inv_alpha + alpha * pixel_function(greenBackdrop, g);
        b = b * inv_alpha + alpha * pixel_function(blueBackdrop, b);

        int ri = r>>8;
        int gi = g>>8;
        int bi = b>>8;
        *pixels_target = COMBINE_KEEP_ALPHA(pixel, ri, gi, bi);
        pixels_target ++;
        pixels_to_blend++;

    }
}

void apply_matrix_function_f(int *pixels, int nr, float Crr, float Crg, float Crb,
                           float Cgr, float Cgg, float Cgb,
                           float Cbr, float Cbg, float Cbb) {

    while (nr--) {
        int pixel = *pixels;
        float r = TO_FLOAT(EXTRACT_R(pixel));
        float g = TO_FLOAT(EXTRACT_G(pixel));
        float b = TO_FLOAT(EXTRACT_B(pixel));

        r = r * Crr + g * Crg + b * Crb;
        g = r * Cgr + g * Cgg + b * Cgb;
        b = r * Cbr + g * Cbg + b * Cbb;

        int ri = LIMIT(TO_INT(r));
        int gi = LIMIT(TO_INT(g));
        int bi = LIMIT(TO_INT(b));
        *pixels = COMBINE_KEEP_ALPHA(pixel, ri, gi, bi);
        pixels++;

    }
}

void apply_matrix_function(int *pixels, int nr, float Crr, float Crg, float Crb,
                             float Cgr, float Cgg, float Cgb,
                             float Cbr, float Cbg, float Cbb) {

    int crr = (int) (Crr * 256.0f + .5f);
    int crg = (int) (Crg * 256.0f + .5f);
    int crb = (int) (Crb * 256.0f + .5f);
    int cgr = (int) (Cgr * 256.0f + .5f);
    int cgg = (int) (Cgg * 256.0f + .5f);
    int cgb = (int) (Cgb * 256.0f + .5f);
    int cbr = (int) (Cbr * 256.0f + .5f);
    int cbg = (int) (Cbg * 256.0f + .5f);
    int cbb = (int) (Cbb * 256.0f + .5f);
    while (nr--) {
        int pixel = *pixels;
        int r = EXTRACT_R(pixel);
        int g = EXTRACT_G(pixel);
        int b = EXTRACT_B(pixel);
        int rout = r * crr + g * crg + b * crb;
        int gout = r * cgr + g * cgg + b * cgb;
        int bout = r * cbr + g * cbg + b * cbb;
        //LOGI("r1out=%d, g1out=%d, b1out=%d", rout, gout, bout);
        rout >>= 8;
        gout >>= 8;
        bout >>= 8;
        //LOGI("r2out=%d, g2out=%d, b2out=%d", rout, gout, bout);
        int ri = LIMIT(rout);
        int gi = LIMIT(gout);
        int bi = LIMIT(bout);
        //LOGI("r3out=%d, g3out=%d, b3out=%d, pixel=%x", ri, gi, bi, pixel);
        pixel = COMBINE_KEEP_ALPHA(pixel, ri, gi, bi);
        //LOGI("pout=%x", pixel);
        *pixels = pixel;
        pixels++;

    }
}


typedef struct ABGR_PIXEL{
    uint8_t a;
    uint8_t b;
    uint8_t g;
    uint8_t r;

};

void apply_linear_function(int *pixels, int nr, float slopeR, float slopeG, float slopeB,
                           float interceptR, float interceptG, float interceptB)
{

    if (has_NEON){
       // todo
      //  apply_linear_function_neon(pixels, nr, slopeR, slopeG, slopeB, interceptR, interceptG, interceptB);
      //  return;
    }
    #define  ONE  256
    int ir = (int)(ONE * interceptR + .5f);
    int ig = (int)(ONE * interceptG + .5f);
    int ib = (int)(ONE * interceptB + .5f);

    int sr = (int)(ONE * slopeR + .5f);
    int sg = (int)(ONE * slopeG + .5f);
    int sb = (int)(ONE * slopeB + .5f);


    while (nr--) {
        int pixel = *pixels;
        int r = EXTRACT_R(*pixels);
        int g = EXTRACT_G(*pixels);
        int b = EXTRACT_B(*pixels);
        r = ((r * sr) >>8) + ir;
        g = ((g * sg) >>8) + ig;
        b = ((b * sb) >>8) + ib;
        int ri = LIMIT(r);
        int gi = LIMIT(g);
        int bi = LIMIT(b);
        *pixels = COMBINE_KEEP_ALPHA(pixel, ri, gi, bi);
        pixels ++;
    }

}
