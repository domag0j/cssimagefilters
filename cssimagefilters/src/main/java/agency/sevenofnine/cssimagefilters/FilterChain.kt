package agency.sevenofnine.cssimagefilters

import android.graphics.Bitmap
import android.graphics.drawable.Drawable

/**
 * Created by dk on 15/11/2017.
 */
class FilterChain(private val filters: List<Filter>, val name: String) {

    fun apply(image: Bitmap){
        filters.forEach { it.apply(image) }
    }

    class Builder{
        val filters: MutableList<Filter> = mutableListOf()
        var _name = "n/a"
        fun name(n: String) = apply { _name=n }
        fun add(filter: Filter) = apply { filters.add(filter) }
        fun brightness(amount: Float) = apply { filters.add(Brightness(amount)) }
        fun contrast(amount: Float) = apply { filters.add(Contrast(amount)) }
        fun invert(amount: Float) = apply { filters.add(Invert(amount)) }
        fun blendColor(porterDuff: PorterDuffMode, argb: Int) = apply { filters.add(BlendColor(porterDuff, argb)) }
        fun blendDrawable(porterDuff: PorterDuffMode,drawable: Drawable) = apply { filters.add(BlendDrawable(porterDuff, drawable)) }
        fun grayscale(amount: Float) = apply { filters.add(Grayscale(amount)) }
        fun sepia(amount: Float) = apply { filters.add(Sepia(amount)) }
        fun saturate(amount: Float) = apply { filters.add(Saturate(amount)) }
        fun hueRotate(degrees: Float) = apply { filters.add(HueRotate(degrees)) }
        fun build() = FilterChain(this)
    }

    private constructor(builder: Builder) : this(builder.filters, builder._name)
}