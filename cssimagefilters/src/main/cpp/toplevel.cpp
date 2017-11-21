#include <jni.h>
#include <string>
#include <thread>
#include <cpu-features.h>
#include <android/log.h>
#include <android/bitmap.h>
#include "pixel_processing.h"


#define  LOG_TAG    "cssimagefilters"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)




int check_format(JNIEnv *env, jobject &bitmap, AndroidBitmapInfo *info){
    int r = AndroidBitmap_getInfo(env, bitmap, info);
    if (r!=0){
        LOGE("Failed to get bitmap info, error=%d", r);
        return r;
    }
    if (info->format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        LOGE("Bitmap format is not RGBA_8888 !");
        return -1;
    }
    return 0;
}

int optimal_thread_count(){
    int nr_cpus = android_getCpuCount();
    if(nr_cpus < 1) return 1;
    if(nr_cpus>16) return 16;
    return nr_cpus;
}

typedef struct {
    int * start_address_p1;
    int * start_address_p2;
    int nr_pixels;
} work_unit;



work_unit* split_work(int nr_units, int total_pixels, int *p1, int *p2=0)
{
    int pixels_per_thread = total_pixels/nr_units;
    work_unit *work_partitions = new work_unit[nr_units];
    int current_position =0 ;
    for (int partition=0; partition<nr_units; partition++){
        int nr_pixels = pixels_per_thread;
        if (nr_pixels+current_position >total_pixels){
            nr_pixels = total_pixels - current_position;
        }
        work_partitions[partition].nr_pixels = nr_pixels;
        work_partitions[partition].start_address_p1 = p1 +current_position;
        if(p2!= nullptr){
            work_partitions[partition].start_address_p2 = p2 +current_position;
        }
        current_position += nr_pixels;
    }
    return work_partitions;
}


////////////////// Top level functions that split work among threads

void linear_function(int *pixels,  int nr, float slopeR, float slopeG, float slopeB,
                    float interceptR, float interceptG, float interceptB)
{
    int nr_threads = optimal_thread_count();
    if (nr<100){
        nr_threads = 1; //silly to start thread for couple of pixels
    }
    std::thread t[nr_threads];
    work_unit *work = split_work(nr_threads, nr, pixels, nullptr);

    for (int i=1; i< nr_threads; i++){
                        t[i-1] = std::thread(apply_linear_function, work[i].start_address_p1, work[i].nr_pixels,
                                           slopeR, slopeG, slopeB, interceptR, interceptG, interceptB);
                    }

    apply_linear_function(work[0].start_address_p1, work[0].nr_pixels, slopeR, slopeG, slopeB, interceptR, interceptG, interceptB);
    for (int i=1; i< nr_threads; i++){
        t[i-1].join();
    }
    delete []work;

}

void matrix_function(int *pixels,  int nr, float Crr, float Crg, float Crb,
                                           float Cgr, float Cgg, float Cgb,
                                           float Cbr, float Cbg, float Cbb)
{
    int nr_threads = optimal_thread_count();
    if (nr<100){
        nr_threads = 1; //silly to start thread for couple of pixels
    }
    std::thread t[nr_threads];
    work_unit *work = split_work(nr_threads, nr, pixels, nullptr);

    for (int i=1; i< nr_threads; i++){
        t[i-1] = std::thread(apply_matrix_function, work[i].start_address_p1, work[i].nr_pixels,
                             Crr, Crg, Crb, Cgr, Cgg, Cgb, Cbr, Cbg, Cbb);
    }

    apply_matrix_function(work[0].start_address_p1, work[0].nr_pixels, Crr, Crg, Crb, Cgr, Cgg, Cgb, Cbr, Cbg, Cbb);
    for (int i=1; i< nr_threads; i++){
        t[i-1].join();
    }
    delete []work;

}


void blend_color(int *pixels, int32_t nr, float red, float green, float blue, float alpha, int porterDuff)
{
    int nr_threads = optimal_thread_count();
    if (nr<100){
        nr_threads = 1; //silly to start thread for couple of pixels
    }
    std::thread t[nr_threads];
    work_unit *work = split_work(nr_threads, nr, pixels, nullptr);
    for (int i=1; i< nr_threads; i++){
        t[i-1] = std::thread(apply_blend_color, work[i].start_address_p1, work[i].nr_pixels,
                             red, green, blue, alpha, porterDuff);
    }
    apply_blend_color(work[0].start_address_p1, work[0].nr_pixels,
            red, green, blue, alpha, porterDuff);
    for (int i=1; i< nr_threads; i++){
        t[i-1].join();
    }
    delete []work;
}



void blend_bitmap(int *pixels_target, int *pixels_to_blend, int32_t nr, int porterDuff)
{

    int nr_threads = optimal_thread_count();
    if (nr<100) {
        nr_threads = 1; //silly to start thread for couple of pixels
    }
    std::thread t[nr_threads];
    work_unit *work = split_work(nr_threads, nr, pixels_target, pixels_to_blend);

    for (int i=1; i< nr_threads; i++){

        t[i-1] = std::thread(apply_blend_bitmap, work[i].start_address_p1,
                             work[i].start_address_p2, work[i].nr_pixels, porterDuff);
    }
    apply_blend_bitmap(work[0].start_address_p1, work[0].start_address_p2, work[0].nr_pixels, porterDuff);

    for (int i=1; i< nr_threads; i++){
        t[i-1].join();
    }
    delete []work;
}

//////////////////// JNI API


extern "C" JNIEXPORT  jint JNICALL Java_agency_sevenofnine_cssimagefilters_CssImageFilters_blendColor( JNIEnv *env,
                                                                                             jobject thiz, jobject bitmap,
                                                                                             float red, float green, float blue,
                                                                                             float alpha, int porterDuff)
{

    AndroidBitmapInfo info;
    int r = check_format(env, bitmap, &info);
    if (r) return r;

    void* pixels;
    if ((r = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", r);
        return r;
    }
    uint32_t nr_pixels = info.width * info.height;
    blend_color((int*)pixels, nr_pixels, red, green, blue, alpha, porterDuff);
    AndroidBitmap_unlockPixels(env, bitmap);
    return 0;
}


extern "C" JNIEXPORT  jint JNICALL Java_agency_sevenofnine_cssimagefilters_CssImageFilters_blendBitmap( JNIEnv *env,
                                                                                             jobject thiz, jobject target,
                                                                                             jobject to_blend, int porterDuff)
{

    AndroidBitmapInfo info;
    int r = check_format(env, to_blend, &info);
    if (r) return r;
    r = check_format(env, target, &info);
    if (r) return r;

    void* pixels_target, *pixels_to_blend;
    if ((r = AndroidBitmap_lockPixels(env, target, &pixels_target)) < 0) {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", r);
        return -1;
    }
    if ((r = AndroidBitmap_lockPixels(env, to_blend, &pixels_to_blend)) < 0) {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", r);
        AndroidBitmap_unlockPixels(env, target);
        return -1;
    }

    uint32_t nr_pixels = info.width * info.height;
    blend_bitmap((int*)pixels_target, (int*)pixels_to_blend, nr_pixels, porterDuff);
    AndroidBitmap_unlockPixels(env, to_blend);
    AndroidBitmap_unlockPixels(env, target);

    return 0;
}



extern "C" JNIEXPORT jint JNICALL Java_agency_sevenofnine_cssimagefilters_CssImageFilters_applyMatrix( JNIEnv *env,
                                                                                             jobject thiz, jobject bitmap,
                                                                                             float Crr, float Crg, float Crb,
                                                                                             float Cgr, float Cgg, float Cgb,
                                                                                             float Cbr, float Cbg, float Cbb)
{

    AndroidBitmapInfo info;
    int r = check_format(env, bitmap, &info);
    if (r) return r;

    void* pixels;
    if ((r = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", r);
        return r;
    }
    uint32_t nr_pixels = info.width * info.height;
    matrix_function((int*)pixels, nr_pixels, Crr, Crg, Crb, Cgr, Cgg, Cgb, Cbr, Cbg, Cbb);
    AndroidBitmap_unlockPixels(env, bitmap);
    return 0;
}



extern "C" JNIEXPORT jint JNICALL Java_agency_sevenofnine_cssimagefilters_CssImageFilters_applyLinear( JNIEnv *env,
                                                                               jobject thiz, jobject bitmap,
                                                                                float slopeR, float slopeG, float slopeB,
                                                                                float interceptR, float interceptG, float interceptB)
{

    AndroidBitmapInfo info;
    int r = check_format(env, bitmap, &info);
    if (r) return r;

    void* pixels;
    if ((r = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", r);
        return r;
    }
    uint32_t nr_pixels = info.width * info.height;
    linear_function((int*)pixels, nr_pixels,slopeR, slopeG, slopeB, interceptR, interceptG, interceptB);
    AndroidBitmap_unlockPixels(env, bitmap);
    return 0;
}



extern "C" JNIEXPORT jstring JNICALL
Java_agency_sevenofnine_cssimagefilters_CssImageFilters_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}