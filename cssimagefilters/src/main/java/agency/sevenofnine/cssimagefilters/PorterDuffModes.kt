package agency.sevenofnine.cssimagefilters

/**
 * Created by dk on 14/11/2017.
 */

enum class PorterDuffMode(val value: Int){
    PORTER_DUFF_NORMAL(0){override fun toString()= "Normal"},
    PORTER_DUFF_MULTIPLY(1){override fun toString()= "Multiply"},
    PORTER_DUFF_SCREEN(2){override fun toString()= "Screen"},
    PORTER_DUFF_OVERLAY(3){override fun toString()= "Overlay"},
    PORTER_DUFF_DARKEN(4){override fun toString()= "Darken"},
    PORTER_DUFF_LIGHTEN(5){override fun toString()= "Lighten"},
    PORTER_DUFF_COLOR_DODGE(6){override fun toString()= "Dodge"},
    PORTER_DUFF_COLOR_BURN(7){override fun toString()= "Color Burn"},
    PORTER_DUFF_HARD_LIGHT(8){override fun toString()= "Hard Light"},
    PORTER_DUFF_SOFT_LIGHT(9){override fun toString()= "Soft Light"},
    PORTER_DUFF_DIFFERENCE(10){override fun toString()= "Difference"},
    PORTER_DUFF_EXCLUSION(11){override fun toString()= "Exclusion"}
}
