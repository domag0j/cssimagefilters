package agency.sevenofnine.imagefilterstest

import agency.sevenofnine.cssimagefilters.FilterChain
import agency.sevenofnine.cssimagefilters.PorterDuffMode
import android.graphics.Bitmap
import android.graphics.Color
import android.graphics.drawable.GradientDrawable

/**
 * Created by dk on 16/11/2017.
 */
abstract class DemoFilter(val minParam: Float = 0.0f, val maxParam: Float = 0.0f, val inactiveParam: Float = 0.0f, val defaultParam: Float=0.0f) {

    var paramValue: Float = inactiveParam
    open fun apply(bm : Bitmap){
        val f = createChain(paramValue);
        f.apply(bm)

    }
    abstract open fun createChain(value: Float): FilterChain
}

class Brightness: DemoFilter(minParam = 0.0f, maxParam = 2.0f, inactiveParam = 1.0f, defaultParam = 1.5f){
    override fun createChain(value: Float): FilterChain = FilterChain.Builder().name("Brightness").
                                                            brightness(value).build();
}

class Contrast: DemoFilter(minParam = 0.0f, maxParam = 2.0f, inactiveParam = 1.0f, defaultParam = 1.5f){
    override fun createChain(value: Float): FilterChain = FilterChain.Builder().name("Contrast").
            contrast(value).build();
}

class Invert: DemoFilter(minParam = 0.0f, maxParam = 1.0f, inactiveParam = 0.0f, defaultParam = 1.0f){
    override fun createChain(value: Float): FilterChain = FilterChain.Builder().name("invert").
            invert(value).build();
}

class Grayscale: DemoFilter(minParam = 0.0f, maxParam = 1.0f, inactiveParam = 0.0f, defaultParam = 1.0f){
    override fun createChain(value: Float): FilterChain = FilterChain.Builder().name("Grayscale").
            grayscale(value).build()
}

class Sepia: DemoFilter(minParam = 0.0f, maxParam = 1.0f, inactiveParam = 0.0f, defaultParam = 1.0f){
    override fun createChain(value: Float): FilterChain = FilterChain.Builder().name("Sepia").
            sepia(value).build()
}

class HueRotate: DemoFilter(minParam = -180f, maxParam = 180f, inactiveParam = 0.0f, defaultParam = 36f){
    override fun createChain(value: Float): FilterChain = FilterChain.Builder().name("Hue rotate").
            hueRotate(value).build()
}

class Saturation: DemoFilter(minParam = 0.0f, maxParam = 2.0f, inactiveParam = 1.0f, defaultParam = 1.4f){
    override fun createChain(value: Float): FilterChain = FilterChain.Builder().name("Saturate").
            saturate(value).build()
}

open class BlendBitmap (val pdMode: PorterDuffMode): DemoFilter(minParam = 0.0f, maxParam = 1.0f, inactiveParam = 0.0f, defaultParam = 1.0f){
    override fun createChain(value: Float): FilterChain {
        val intAlpha = (value * 255.0f).toInt()
        val colors: IntArray = intArrayOf(Color.argb(intAlpha, 127, 40, 120), Color.argb(0, 127, 0, 0))
        val g = GradientDrawable(GradientDrawable.Orientation.TL_BR, colors)
        return FilterChain.Builder().name(pdMode.toString()).
                blendDrawable(pdMode, g).build()
    }
}

class Dbg: DemoFilter(minParam = 0.0f, maxParam = 1.0f, inactiveParam = 0.0f, defaultParam = 1.0f){
    override fun createChain(value: Float): FilterChain = FilterChain.Builder().name("Sepia").
            sepia(value).build()
}


class Aden: DemoFilter(minParam = 1.0f, maxParam = 1.0f, inactiveParam = 1.0f, defaultParam = 1.0f){
    override fun createChain(value: Float): FilterChain {
        val colors: IntArray = intArrayOf(Color.argb(0, 0, 0, 0),
                                        Color.argb(51, 66, 10, 14))
        val g = GradientDrawable(GradientDrawable.Orientation.LEFT_RIGHT, colors)

        val builder =  FilterChain.Builder().name("Aden").hueRotate(-20.0f).contrast(0.9f)
                .saturate(.85f).brightness(1.2f).
                blendDrawable(PorterDuffMode.PORTER_DUFF_DARKEN, g)
        return builder.build()

    }
}

class Valencia: DemoFilter(minParam = 1.0f, maxParam = 1.0f, inactiveParam = 1.0f, defaultParam = 1.0f){
    override fun createChain(value: Float): FilterChain {
        val builder =  FilterChain.Builder().name("Valencia").contrast(1.08f)
               .brightness(1.08f).sepia(0.08f).
                blendColor(PorterDuffMode.PORTER_DUFF_EXCLUSION, Color.argb(127, 58, 3, 57))
        return builder.build()

    }
}


class Gingham: DemoFilter(minParam = 1.0f, maxParam = 1.0f, inactiveParam = 1.0f, defaultParam = 1.0f){
    override fun createChain(value: Float): FilterChain {
        val builder =  FilterChain.Builder().name("Gingham")
                .brightness(1.05f).hueRotate(-10f).
                blendColor(PorterDuffMode.PORTER_DUFF_SOFT_LIGHT, Color.argb(127, 230, 230, 250))
        return builder.build()

    }
}

class Clarendon: DemoFilter(minParam = 1.0f, maxParam = 1.0f, inactiveParam = 1.0f, defaultParam = 1.0f){
    override fun createChain(value: Float): FilterChain {
        val builder =  FilterChain.Builder().name("clarendon").
                blendColor(PorterDuffMode.PORTER_DUFF_OVERLAY, Color.argb(51, 127, 187, 227))
                .contrast(1.2f).saturate(1.35f)

        return builder.build()

    }
}

class Slumber: DemoFilter(minParam = 1.0f, maxParam = 1.0f, inactiveParam = 1.0f, defaultParam = 1.0f){
    override fun createChain(value: Float): FilterChain {
        val builder =  FilterChain.Builder().name("Slumber").
                blendColor(PorterDuffMode.PORTER_DUFF_LIGHTEN, Color.argb(102, 69, 41, 12))
                .saturate(.66f).brightness(1.05f).
                blendColor(PorterDuffMode.PORTER_DUFF_SOFT_LIGHT, Color.argb(64, 125, 105, 24))


        return builder.build()

    }
}





fun createDemoFilters(): List<DemoFilter> = listOf(Brightness(), Contrast(), Invert(), Grayscale(),
        Sepia(), HueRotate(), Saturation()) +  PorterDuffMode.values().map { BlendBitmap(it) } +
        listOf(Aden(), Valencia(), Gingham(), Clarendon(), Slumber())




