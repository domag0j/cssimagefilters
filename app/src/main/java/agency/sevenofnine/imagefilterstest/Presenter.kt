package agency.sevenofnine.imagefilterstest



/**
 * Created by dk on 16/11/2017.
 */
class Presenter(val view: ViewInterface): PresenterInterface{


    private val filters = createDemoFilters()
    private var currentFilter: Int = 0
    private var paramValue: Float = 0.0f
    private var enabled: Boolean = true

    override fun start() {
       view.setupFilters(filters)
       setup(currentFilter)
        if (enabled) {
            applyFilter(currentFilter, paramValue)
        }else{
            view.clearFilter()
            view.setAppTitle("OFF")
        }
    }


    override fun stop() {

    }

    private fun setup(current: Int) {
        val c = filters[current]
        view.setSliderRange(c.minParam, c.maxParam, c.defaultParam)
        paramValue = c.defaultParam
    }

    override fun paramReset() {
        enabled = true
        val c = filters[currentFilter]
        view.setSliderRange(c.minParam, c.maxParam, c.inactiveParam)
        paramValue = c.inactiveParam
        applyFilter(currentFilter, paramValue)
    }


    override fun filterSelected(index: Int) {
        enabled = true
        if (index!=currentFilter){
            currentFilter = index
            setup(currentFilter)
        }


        applyFilter(currentFilter, paramValue)
    }

    override fun parameterValueSet(value: Float) {
        if (paramValue==value) return
        paramValue = value
        applyFilter(currentFilter, paramValue)
    }

    override fun filtersToggle() {

       enabled = !enabled
        if (enabled){
            applyFilter(currentFilter, paramValue)
        }else {
            view.clearFilter()
            view.setAppTitle("OFF")
        }
    }

    private fun applyFilter(filterIndex: Int, value: Float){
        val c = filters[filterIndex]
        val chain = c.createChain(value)
        view.applyFilter(chain)
        val s = "%.2f".format(value)
        view.setAppTitle("${chain.name}: $s")
    }
}