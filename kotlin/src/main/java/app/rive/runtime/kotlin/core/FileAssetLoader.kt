package app.rive.runtime.kotlin.core

import android.content.Context
import android.util.Log
import androidx.annotation.VisibleForTesting
import app.rive.runtime.kotlin.RiveAnimationView
import com.android.volley.NetworkResponse
import com.android.volley.ParseError
import com.android.volley.Request
import com.android.volley.Response
import com.android.volley.toolbox.HttpHeaderParser
import com.android.volley.toolbox.Volley

abstract class FileAssetLoader : NativeObject(NULL_POINTER) {
    init {
        // Make the corresponding C++ object.
        cppPointer = constructor()
        refs.incrementAndGet()
        assert(cppPointer != NULL_POINTER)
    }

    /* C++ constructor */
    protected external fun constructor(): Long

    /* Destructor gets called on [dispose()] */
    external override fun cppDelete(pointer: Long)

    external fun cppRef(pointer: Long)

    private external fun cppSetRendererType(pointer: Long, rendererType: Int)

    /** Override this method to customize the asset loading process. */
    abstract fun loadContents(asset: FileAsset, inBandBytes: ByteArray): Boolean

    fun setRendererType(rendererType: RendererType) =
        cppSetRendererType(cppPointer, rendererType.value)

    override fun acquire(): Int {
        cppRef(cppPointer)
        return super.acquire()
    }
}

abstract class ContextAssetLoader(protected val context: Context) : FileAssetLoader()

class FallbackAssetLoader(
    context: Context,
    loadCDNAssets: Boolean = true,
    loader: FileAssetLoader? = null,
) : FileAssetLoader() {
    @VisibleForTesting(otherwise = VisibleForTesting.PACKAGE_PRIVATE)
    val loaders = mutableListOf<FileAssetLoader>()

    init {
        loader?.let { appendLoader(it) }
        // Exodus hardening:
        // Never append CDN loader. If someone tries to pass loadCDNAssets=true,
        // fail fast in debug builds to avoid accidental enablement.
        check(!loadCDNAssets) {
            "Remote/CDN asset loading is disabled in the Exodus fork"
        }
    }

    fun appendLoader(loader: FileAssetLoader) {
        loaders.add(loader)
        // Make sure everything is disposed.
        dependencies.add(loader)
    }

    fun prependLoader(loader: FileAssetLoader) {
        loaders.add(0, loader)
        // Make sure everything is disposed.
        dependencies.add(loader)
    }

    override fun loadContents(asset: FileAsset, inBandBytes: ByteArray) =
        loaders.any { it.loadContents(asset, inBandBytes) }

    private fun resetCDNLoader(needsCDNLoader: Boolean, context: Context) {
        // Exodus hardening: ensure CDN loader is never present, and assert if requested.
        val cdnLoaderIndex = loaders.indexOfFirst { it is CDNAssetLoader }
        if (cdnLoaderIndex >= 0) {
            loaders.removeAt(cdnLoaderIndex).let {
                dependencies.remove(it)
                it.release()
            }
        }
        check(!needsCDNLoader) {
            "Remote/CDN asset loading is disabled in the Exodus fork"
        }
    }

    /**
     * Resets the state of the asset loader when building RiveAnimationView with the secondary
     * constructor.
     */
    internal fun resetWith(builder: RiveAnimationView.Builder) {
        // First, try setting up a custom loader.
        builder.assetLoader?.let {
            // Prepend loader to make sure custom always executes first.
            prependLoader(it)
        }
        resetCDNLoader(builder.shouldLoadCDNAssets, builder.context.applicationContext)
    }
}

@Deprecated("Disabled in Exodus fork: remote/CDN asset loading is not allowed")
open class CDNAssetLoader(context: Context) : FileAssetLoader() {
    private val tag = javaClass.simpleName

    private val queue by lazy { Volley.newRequestQueue(context) }

    override fun loadContents(asset: FileAsset, inBandBytes: ByteArray): Boolean {
        // Exodus hardening: hard-fail any attempt to fetch remote resources.
        error("Remote/CDN asset loading is disabled in the Exodus fork")
        @Suppress("UNREACHABLE_CODE")
        return false
    }
}

class BytesRequest(
    url: String,
    private val onResponse: (bytes: ByteArray) -> Unit,
    errorListener: Response.ErrorListener,
) : Request<ByteArray>(Method.GET, url, errorListener) {
    init {
        // Exodus hardening: remote HTTP(S) requests must never be created.
        error("BytesRequest (remote fetch) is disabled in the Exodus fork")
    }

    override fun deliverResponse(response: ByteArray) {
        // This should never run
        check(false) { "BytesRequest.deliverResponse invoked unexpectedly" }
    }

    override fun parseNetworkResponse(response: NetworkResponse?): Response<ByteArray> {
        // This should never run
        check(false) { "BytesRequest.parseNetworkResponse invoked unexpectedly" }
        return Response.error(ParseError(Exception("Disabled")))
    }
}
