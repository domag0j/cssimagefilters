package agency.sevenofnine.cssimagefilters

import android.graphics.Bitmap

/**
 * Created by dk on 13/11/2017.
 */



class CssImageFilters {
    external fun stringFromJNI(): String
    external fun applyLinear(bitmap: Bitmap, slopeR: Float, slopeG: Float, slopeB: Float,
                              interceptR: Float, interceptG: Float, interceptB: Float): Int
    external fun applyMatrix(bitmap: Bitmap, Crr: Float, Crg: Float, Crb: Float,
                                              Cgr: Float, Cgg: Float, Cgb: Float,
                                              Cbr: Float, Cbg: Float, Cbb: Float): Int
    external fun blendColor(target: Bitmap, red: Float, green: Float, blue: Float, alpha: Float, porterDuff: Int): Int
    external fun blendBitmap(target: Bitmap, to_blend: Bitmap, porterDuff: Int): Int
    companion object {

        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("cssimagefilters-lib");

        }


    }
}