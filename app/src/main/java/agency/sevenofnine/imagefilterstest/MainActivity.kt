package agency.sevenofnine.imagefilterstest

import agency.sevenofnine.cssimagefilters.FilterChain
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.os.Bundle
import android.renderscript.Allocation
import android.renderscript.Element
import android.renderscript.RenderScript
import android.renderscript.ScriptIntrinsicLUT
import android.support.v7.app.AppCompatActivity
import android.support.v7.widget.LinearLayoutManager
import android.util.Log
import android.widget.SeekBar
import android.widget.SeekBar.OnSeekBarChangeListener
import kotlinx.android.synthetic.main.activity_main.*






class MainActivity : AppCompatActivity(), ViewInterface {
    override fun setAppTitle(t: String) {
       title = t
    }

    var paramMin = 0.0f;
    var paramMax = 0.0f;
    private val picture  by lazy { BitmapFactory.decodeResource(application.resources, R.drawable.photo) }

    val rs by lazy  { RenderScript.create(applicationContext)}
    val script by lazy { ScriptIntrinsicLUT.create(rs, Element.U8_4(rs))}
    val presenter = Presenter(this)

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        //picture = BitmapFactory.decodeResource(application.resources, R.drawable.photo)
        setContentView(R.layout.activity_main)
        val layoutManager = LinearLayoutManager(this)
        layoutManager.orientation = LinearLayoutManager.HORIZONTAL
        layoutManager.scrollToPosition(0)
        filtersView.layoutManager = layoutManager
        filtersView.setHasFixedSize(true)
        filtersView.adapter = FilterAdapter(presenter)
        slider.setOnSeekBarChangeListener(object: OnSeekBarChangeListener{
            override fun onStartTrackingTouch(seekBar: SeekBar?){}
            override fun onStopTrackingTouch(seekBar: SeekBar?){}
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                val pr = progress/100.0f *(paramMax-paramMin) + paramMin
                presenter.parameterValueSet(pr)
            }
        })
        reset.setOnClickListener {presenter.paramReset()}
        imageView.setOnClickListener {presenter.filtersToggle()}
    }

    override fun onResume() {
        super.onResume()
        presenter.start()
    }

    override fun onPause() {
        presenter.stop()
        super.onPause()
    }


    override fun setSliderRange(min: Float, max: Float, initialValue: Float) {
      if (min==max) {
          slidersGroup.isEnabled = true
      }else{
          slidersGroup.isEnabled = false
          this.paramMax = max
          this.paramMin = min
          slider.progress = (100 * (initialValue-min)/(max-min)+.5).toInt()
      }


    }
    fun testRenderScript(bm: Bitmap) {
        val now = System.currentTimeMillis()

        val allocation = Allocation.createFromBitmap(rs, bm)


        for (i in 0..255){
            script.setAlpha(i, i)
            script.setRed(i, i/2)
            script.setGreen(i, i/2)
            script.setBlue(i, i/2)
        }

        script.forEach(allocation, allocation)
        allocation.copyTo(bm);

        //Destroy everything to free memory
        allocation.destroy();
        //lutAllocation.destroy();
        //script.destroy();
        //t.destroy();
        //rs.destroy();
        Log.d("XXX", "Rs in ${System.currentTimeMillis()-now}")
    }

    override fun applyFilter(filterChain: FilterChain) {

        val to_filter = Bitmap.createScaledBitmap(picture, 600, 600, false)
        val t1 = System.currentTimeMillis()
        //testRenderScript(to_filter)
        filterChain.apply(to_filter)
        val t2 = System.currentTimeMillis()
        Log.d("XXX", "in ${t2-t1}")

        imageView.setImageBitmap(to_filter)
    }

    override fun clearFilter() {
        imageView.setImageBitmap(picture)
    }

    override fun setupFilters(filters: List<DemoFilter>) {
       val adapter = filtersView.adapter as? FilterAdapter ?: return
        adapter.setup(filters)
    }




}
