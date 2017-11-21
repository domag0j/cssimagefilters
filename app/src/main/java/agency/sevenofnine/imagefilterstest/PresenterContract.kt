package agency.sevenofnine.imagefilterstest

import agency.sevenofnine.cssimagefilters.FilterChain

/**
 * Created by dk on 16/11/2017.
 */


interface ViewInterface{
    fun setSliderRange(min: Float, max:Float, initialValue: Float)
    fun applyFilter(filterChain: FilterChain)
    fun clearFilter()
    fun setupFilters(filters: List<DemoFilter>)
    fun setAppTitle(title: String)
}


interface PresenterInterface {
    fun start()
    fun stop()

    fun filterSelected(index: Int)
    fun parameterValueSet(value: Float)
    fun filtersToggle()
    fun paramReset()
}