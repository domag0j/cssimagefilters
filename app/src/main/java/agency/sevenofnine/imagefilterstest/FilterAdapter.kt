package agency.sevenofnine.imagefilterstest

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.support.v7.widget.RecyclerView
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import android.widget.TextView

/**
 * Created by dk on 16/11/2017.
 */
class FilterAdapter(val presenter: PresenterInterface): RecyclerView.Adapter<FilterViewHolder>() {
    private val demoFilters:MutableList<DemoFilter> = mutableListOf()
    override fun onCreateViewHolder(parent: ViewGroup?, viewType: Int): FilterViewHolder {
        val view = LayoutInflater.from(parent?.context).inflate(R.layout.filter_view, parent, false)
        return FilterViewHolder(view, presenter)
    }

    override fun onBindViewHolder(holder: FilterViewHolder?, position: Int) {
        holder?.setFilter(demoFilters[position], position)
    }

    fun setup(filters: List<DemoFilter>){
        demoFilters.clear()
        demoFilters.addAll(filters)
        notifyDataSetChanged()
    }

    override fun getItemCount(): Int = demoFilters.size
}

class FilterViewHolder(itemView: View?, val presenter: PresenterInterface) : RecyclerView.ViewHolder(itemView){
    val imageView: ImageView = itemView!!.findViewById(R.id.thumbnail)
    val textView: TextView = itemView!!.findViewById(R.id.filter_name)
    val parent = itemView
    var filterPosition: Int = 0

    fun setFilter(filter: DemoFilter, pos: Int){
        parent?.setOnClickListener() { presenter.filterSelected(pos)}
        filterPosition = pos
        val chain = filter.createChain(filter.defaultParam)
        textView.text = chain.name
        val bmp = BitmapFactory.decodeResource(textView.context.resources, R.drawable.photo)
        val scaledbmp = Bitmap.createScaledBitmap(bmp, 160, 160, false)
        chain.apply(scaledbmp)
        imageView.setImageBitmap(scaledbmp)
    }

}