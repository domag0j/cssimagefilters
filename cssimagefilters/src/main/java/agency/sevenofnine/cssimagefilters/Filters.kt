package agency.sevenofnine.cssimagefilters

import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.drawable.Drawable

/**
 * Created by dk on 14/11/2017.
 */

interface Filter{
    fun apply(image: Bitmap)
}

open class LinearFilter(val slopeR: Float=1.0f, val slopeG: Float=1.0f, val slopeB: Float=1.0f,
                   val interceptR: Float=0.0f, val interceptG: Float=0.0f, val interceptB: Float=0.0f): Filter{
    override fun apply(image: Bitmap) {
        CssImageFilters().applyLinear(image, this.slopeR, this.slopeG, this.slopeB,
                                        this.interceptR, this.interceptG, this.interceptB)
    }
}

open class LinearMonotone(slope: Float=1.0f, intercept: Float=0.0f): LinearFilter(slopeR = slope, slopeG = slope, slopeB = slope,
                                                                    interceptR = intercept, interceptG = intercept, interceptB = intercept);
class Brightness(brightness: Float): LinearMonotone(slope=brightness);
class Contrast(contrast: Float): LinearMonotone(slope=contrast, intercept=.5f - contrast/2.0f);
class Invert(invert: Float): LinearMonotone(slope=1-2*invert, intercept=invert);

class BlendColor(private val porterDuff:PorterDuffMode, private val argb: Int): Filter{
    override fun apply(image: Bitmap) {
        val red = Color.red(argb) / 255.0f;
        val green = Color.green(argb) / 255.0f;
        val blue = Color.blue(argb) / 255.0f;
        val alpha = Color.alpha(argb) / 255.0f;
        CssImageFilters().blendColor(image, red, green, blue, alpha, porterDuff.value);
    }
}

class BlendDrawable(private val porterDuff: PorterDuffMode, val drawable: Drawable): Filter{
    var db: Bitmap? = null
    override fun apply(image: Bitmap) {
        prepareDrawableBitmap(image.width, image.height)
        db?.let { CssImageFilters().blendBitmap(image, it, porterDuff.value) }
    }

    private fun prepareDrawableBitmap(width: Int, height: Int) {
        if (db == null || db?.width != width || db?.height != height) {
            db = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888)
            drawable.setBounds(0, 0, width, height)
            val canvas = Canvas(db)
            drawable.draw(canvas)
        }

    }
}


open class MatrixFilter(private val matrix: Array<FloatArray> = arrayOf(floatArrayOf(1.0f, 0.0f, 0.0f),
                                                           floatArrayOf(0.0f, 1.0f, 0.0f),
                                                           floatArrayOf(0.0f, 0.0f, 1.0f) )): Filter{
    override fun apply(image: Bitmap) {
       CssImageFilters().applyMatrix(image, Crr=matrix[0][0], Crg=matrix[0][1], Crb=matrix[0][2],
                                             Cgr=matrix[1][0], Cgg=matrix[1][1], Cgb=matrix[1][2],
                                             Cbr=matrix[2][0], Cbg=matrix[2][1], Cbb=matrix[2][2])

    }
}

class Grayscale(amount: Float): MatrixFilter(matrix=grayscaleMatrix(amount))
class Sepia(amount: Float): MatrixFilter(matrix= sepiaMatrix(amount))
class Saturate(amount: Float): MatrixFilter(matrix= saturateMatrix(amount))
class HueRotate(degrees: Float): MatrixFilter(matrix = hueRotateMatrix(degrees))


//see https://www.w3.org/TR/filter-effects/#element-attrdef-fecomponenttransfer-intercept
private fun grayscaleMatrix(amount: Float): Array<FloatArray> {
    val inv_amount = 1.0f - amount;
    return arrayOf(
            floatArrayOf(.2126f + .7874f*inv_amount, .7152f*amount, .0722f *amount),
            floatArrayOf(.2126f *amount, .7152f + .2848f*inv_amount, .0722f *amount),
            floatArrayOf(.2126f *amount, .7152f*amount, .0722f + .9278f *inv_amount)
    )
}

private fun sepiaMatrix(amount: Float): Array<FloatArray> {
    val inv_amount = 1.0f - amount;
    return arrayOf(
            floatArrayOf(.393f + .607f*inv_amount, .769f*amount, .189f *amount),
            floatArrayOf(.349f *amount, .686f + .314f*inv_amount, .168f *amount),
            floatArrayOf(.272f *amount, .534f*amount, .131f + .869f *inv_amount)
    )
}

private fun saturateMatrix(amount: Float): Array<FloatArray> {
    val inv_amount = 1.0f - amount;
    return arrayOf(
            floatArrayOf(.213f+.787f*amount, .715f*inv_amount, .072f*inv_amount),
            floatArrayOf(.213f*inv_amount, .715f+.285f*amount, .072f*inv_amount),
            floatArrayOf(.213f*inv_amount, .715f*inv_amount, .072f+.928f*amount)
    )
}

private fun hueRotateMatrix(degrees: Float): Array<FloatArray> {
    val rad = degrees *  Math.PI/ 180;
    val co: Float = Math.cos(rad).toFloat()
    val si: Float = Math.sin(rad).toFloat()

    return arrayOf(
            floatArrayOf(.213f+co*.787f-si*.213f, .715f-co*.715f-si*.715f, .072f-co*.072f+si*.928f),
            floatArrayOf(.213f-co*.213f+si*.143f, .715f+co*.285f+si*.140f, .072f-co*.072f-si*.283f),
            floatArrayOf(.213f-co*.213f-si*.787f, .715f-co*.715f+si*.715f, .072f+co*.928f+si*.072f)
    )
}







