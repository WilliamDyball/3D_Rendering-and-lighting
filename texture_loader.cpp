
#include "texture_loader.h"


// Windows Imaging Component factory class (singleton)
static IWICImagingFactory			*wicFactory = NULL;


// safe release COM interfaces
template <class T>
inline void SafeRelease(T **comInterface)
{
    if (*comInterface)
    {
        (*comInterface)->Release();
        *comInterface = NULL;
    }
}

HRESULT initCOM(void) {

	return CoInitialize(NULL);
}

void shutdownCOM(void) {

	CoUninitialize(); // Notify COM we're done
}


HRESULT getWICFormatConverter(IWICFormatConverter **formatConverter) {
	
	if (!formatConverter || !wicFactory)
		return E_FAIL;
	else
		return wicFactory->CreateFormatConverter(formatConverter);
}


// Load and return an IWICBitmap interface representing the image loaded from path.  No format conversion is done here - this is left to the caller so each delegate can apply the loaded image data as needed.
HRESULT loadWICBitmap(LPCWSTR path, IWICBitmap **bitmap) {

	if (!bitmap || !wicFactory)
		return E_FAIL;

	IWICBitmapDecoder *bitmapDecoder = NULL;
	IWICBitmapFrameDecode *imageFrame = NULL;
	IWICFormatConverter *formatConverter = NULL;
	
	*bitmap = NULL;

	// create image decoder
	HRESULT hr = wicFactory->CreateDecoderFromFilename(path, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &bitmapDecoder);
	
	// validate number of frames
	
	UINT numFrames = 0;

	if (SUCCEEDED(hr)) {

		hr = bitmapDecoder->GetFrameCount(&numFrames);
	}

	if (SUCCEEDED(hr) && numFrames>0) {

		// decode first image frame (default to first frame - for animated types add parameters to select frame later!)
		hr = bitmapDecoder->GetFrame(0, &imageFrame);
	}

	if (SUCCEEDED(hr)) {

		hr = wicFactory->CreateFormatConverter(&formatConverter);
	}

	WICPixelFormatGUID pixelFormat;

	if (SUCCEEDED(hr)) {

		// check we can convert to the required format GUID_WICPixelFormat32bppPBGRA			
		hr = imageFrame->GetPixelFormat(&pixelFormat);
	}

	BOOL canConvert = FALSE;

	if (SUCCEEDED(hr)) {

		hr = formatConverter->CanConvert(pixelFormat, GUID_WICPixelFormat32bppPBGRA, &canConvert);
	}

	if (SUCCEEDED(hr) && canConvert==TRUE) {
		
		hr = formatConverter->Initialize(
		imageFrame,						// Input source to convert
		GUID_WICPixelFormat32bppPBGRA,	// Destination pixel format
		WICBitmapDitherTypeNone,		// Specified dither pattern
		NULL,							// Specify a particular palette 
		0.f,							// Alpha threshold
		WICBitmapPaletteTypeCustom		// Palette translation type
		);
	}

	if (SUCCEEDED(hr)) {

		// convert and create bitmap from converter
		hr = wicFactory->CreateBitmapFromSource(formatConverter, WICBitmapCacheOnDemand, bitmap);
	}


	// cleanup
	SafeRelease(&formatConverter);
	SafeRelease(&imageFrame);
	SafeRelease(&bitmapDecoder);

	// return result
	return hr;
}


GLuint loadTexture(const std::wstring& filename) {

	HRESULT		hr;

	// on first call instantiate WIC factory class
	if (!wicFactory) {

		hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory));

		// validate wicFactory before proceeding
		if (!SUCCEEDED(hr))
			return 0;
	}


	IWICBitmap			*textureBitmap = NULL;
	IWICBitmapLock		*lock = NULL;
	GLuint				newTexture = 0;

	hr = loadWICBitmap(filename.c_str(), &textureBitmap);

	UINT w = 0, h = 0;

	if (SUCCEEDED(hr))
		hr = textureBitmap->GetSize(&w, &h);

	WICRect rect = {0, 0, w, h};

	if (SUCCEEDED(hr))
		hr = textureBitmap->Lock(&rect, WICBitmapLockRead, &lock);

	UINT bufferSize = 0;
	BYTE *buffer = NULL;

	if (SUCCEEDED(hr))
		hr = lock->GetDataPointer(&bufferSize, &buffer);

	if (SUCCEEDED(hr)) {
		
		glGenTextures(1, &newTexture);
		glBindTexture(GL_TEXTURE_2D, newTexture);

		// note: GL_BGRA format used - input image format converted to GUID_WICPixelFormat32bppPBGRA for consistent interface with OpenGL texture setup
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, buffer);
	}

	SafeRelease(&lock);
	SafeRelease(&textureBitmap);


	// setup default texture properties
	if (newTexture) {

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	return newTexture;
}

