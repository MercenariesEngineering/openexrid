/*

             *
            ***
           *****
   *********************       Mercenaries Engineering SARL
     *****************              Copyright (C) 2016
       *************
         *********        http://www.mercenaries-engineering.com
        ***********
       ****     ****
      **           **

*/

#include <set>
#include <openexrid/Mask.h>
#include <openexrid/Query.h>
#include <ImfDeepScanLineInputFile.h>
#include <re2/set.h>
#include <assert.h>
#include "instance.h"
#include "ofxUtilities.h"

extern OfxStatus onInstanceChanged(OfxImageEffectHandle effect, OfxPropertySetHandle inArgs);
extern std::string computeFinalName (OfxPropertySetHandle inArgs, Instance *instance, bool &black);
extern bool isFileAnimated (OfxPropertySetHandle inArgs, Instance *instance);


using namespace Imf;
using namespace Imath;

#if defined __APPLE__ || defined linux || defined __FreeBSD__
#  define EXPORT __attribute__((visibility("default")))
#elif defined _WIN32
#  define EXPORT OfxExport
#else
#  error Not building on your operating system quite yet
#endif

// pointers64 to various bits of the host
OfxHost					*gHost = NULL;
OfxImageEffectSuiteV1	*gEffectHost = NULL;
OfxPropertySuiteV1		*gPropHost = NULL;
OfxParameterSuiteV1		*gParamHost = NULL;
OfxMultiThreadSuiteV1	*gThreadHost = NULL;
OfxInteractSuiteV1		*gInteractHost = NULL;
OfxMemorySuiteV1		*gMemoryHost = NULL;
OfxMessageSuiteV1		*gMessageSuite = NULL;

// Remove the - sign
const char *removeNeg (const string &s)
{
	if (!s.empty () && s[0] == '-')
		return s.c_str ()+1;
	return s.c_str ();
}

// Convinience wrapper to get private data 
static Instance *getInstanceData(OfxImageEffectHandle effect)
{
	Instance *instance = (Instance *) ofxuGetEffectInstanceData(effect);
	return instance;
}

//  instance construction
static OfxStatus createInstance(OfxImageEffectHandle effect)
{
	// get a pointer to the effect properties
	OfxPropertySetHandle effectProps;
	gEffectHost->getPropertySet(effect, &effectProps);

	// get a pointer to the effect's parameter set
	OfxParamSetHandle paramSet;
	gEffectHost->getParamSet(effect, &paramSet);

	// make my private instance data
	Instance *instance = new Instance;

	// cache away param handles
	gParamHost->paramGetHandle(paramSet, "file", &instance->File, 0);

	gParamHost->paramGetHandle(paramSet, "firstFrame", &instance->FirstFrame, 0);
	gParamHost->paramGetHandle(paramSet, "lastFrame", &instance->LastFrame, 0);
	gParamHost->paramGetHandle(paramSet, "before", &instance->Before, 0);
	gParamHost->paramGetHandle(paramSet, "after", &instance->After, 0);
	gParamHost->paramGetHandle(paramSet, "frame", &instance->Frame, 0);
	gParamHost->paramGetHandle(paramSet, "offset", &instance->Offset, 0);
	gParamHost->paramGetHandle(paramSet, "missingFrames", &instance->MissingFrames, 0);

	gParamHost->paramGetHandle(paramSet, "pattern", &instance->Pattern, 0);
	gParamHost->paramGetHandle(paramSet, "colors", &instance->Colors, 0);
	gParamHost->paramGetHandle(paramSet, "invert", &instance->Invert, 0);
	gParamHost->paramGetHandle(paramSet, "alpha", &instance->Alpha, 0);

	// cache away clip handles
	gEffectHost->clipGetHandle(effect, kOfxImageEffectOutputClipName, &instance->OutputClip, 0);

	// set my private instance data
	gPropHost->propSetPointer(effectProps, kOfxPropInstanceData, 0, (void *) instance);

	// get the host name
	char *returnedHostName;
	gPropHost->propGetString(gHost->host, kOfxPropName, 0, &returnedHostName);

	return kOfxStatOK;
}

// instance destruction
static OfxStatus destroyInstance(OfxImageEffectHandle effect)
{
	// get my instance data
	Instance *instance = getInstanceData(effect);

	// and delete it
	if(instance)
		delete instance;
	return kOfxStatOK;
}

// tells the host what region we are capable of filling
OfxStatus getSpatialRoD(OfxImageEffectHandle effect, OfxPropertySetHandle inArgs, OfxPropertySetHandle outArgs)
{
	OfxStatus status = kOfxStatOK;

	// retrieve any instance data associated with this effect
	Instance *instance = getInstanceData(effect);

	// Read the mas now
	bool black;
	const std::string finalName = computeFinalName (inArgs, instance, black);

	try 
	{
		DeepScanLineInputFile file (finalName.c_str ());
		const Header& header = file.header();
		const Box2i displayWindow = header.displayWindow();
		const Box2i dataWindow = header.dataWindow();
		const int w = dataWindow.max.x+1-dataWindow.min.x;
		const int h = dataWindow.max.y+1-dataWindow.min.y;
		const int imageH = displayWindow.max.y+1;
		const int xMin = dataWindow.min.x;
		const int yMin = imageH-dataWindow.max.y-1;
		const double res[] = {(double)xMin, (double)yMin, (double)(xMin+w), (double)(yMin+h)};
		gPropHost->propSetDoubleN(outArgs, kOfxImageEffectPropRegionOfDefinition, 4, res);
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what () << std::endl;
		status = kOfxStatFailed;
	}

	return status;
}

// look up a pixel in the image, does bounds checking to see if it is in the image rectangle
template <class PIX> 
inline PIX *pixelAddress(PIX *img, OfxRectI rect, int x, int y, int bytesPerLine)
{  
	if(x < rect.x1 || x >= rect.x2 || y < rect.y1 || y > rect.y2)
		return 0;
	PIX *pix = (PIX *) (((char *) img) + (y - rect.y1) * bytesPerLine);
	pix += x - rect.x1;  
	return pix;
}

// base class to process images with
class Processor 
{
public :
	Processor(OfxImageEffectHandle eff, OfxPointD rs, void *dst, 
		OfxRectI dRect, int dBytesPerLine, OfxRectI  win, openexrid::Query &query, bool colors, bool black, bool alpha)
		: effect(eff)
		, renderScale(rs)
		, dstV(dst)
		, dstRect(dRect)
		, window(win)
		, dstBytesPerLine(dBytesPerLine)
		, Query (query)
		, Colors (colors)
		, Black (black)
		, Alpha (alpha)
	{}

	static void multiThreadProcessing(unsigned int threadId, unsigned int nThreads, void *arg);
	void doProcessing(OfxRectI window);
	void process(void);

protected :
	OfxImageEffectHandle effect;
	OfxPointD		renderScale;
	void *dstV; 
	OfxRectI dstRect;
	OfxRectI  window;
	int dstBytesPerLine;
	openexrid::Query &Query;
	bool	Colors, Black, Alpha;
};

// function call once for each thread by the host
void Processor::multiThreadProcessing(unsigned int threadId, unsigned int nThreads, void *arg)
{	
	Processor *proc = (Processor *) arg;

	// slice the y range into the number of threads it has
	unsigned int dy = proc->window.y2 - proc->window.y1;
  
	unsigned int y1 = proc->window.y1 + threadId * dy/nThreads;
	unsigned int y2 = proc->window.y1 + std::min((threadId + 1) * dy/nThreads, dy);

	OfxRectI win = proc->window;
	win.y1 = y1; win.y2 = y2;

	// and render that thread on each
	proc->doProcessing(win);
}

// function to kick off rendering across multiple CPUs
void Processor::process(void)
{
	unsigned int nThreads;
	gThreadHost->multiThreadNumCPUs(&nThreads);
	gThreadHost->multiThread(multiThreadProcessing, nThreads, (void *) this);
}

inline float halton (float base, int id)
{
	float result = 0;
	float f = 1;
	float i = (float)id;
	while (i > 0)
	{
		f = f / base;
		result = result + f * fmodf (i, base);
		i = floorf (i / base);
	}
	return result;
}

inline OfxRGBColourF haltonColors (int id)
{
	return {halton (2, id), halton (3, id), halton (5, id)};
}

void Processor::doProcessing(OfxRectI procWindow)
{
	OfxRGBAColourF *dst = (OfxRGBAColourF*)dstV;
	openexrid::Sample sample;
	const int R = Alpha ? -1 : Query.TheMask->findSlice ("R");
	const int G = Alpha ? -1 : Query.TheMask->findSlice ("G");
	const int B = Alpha ? -1 : Query.TheMask->findSlice ("B");
	const int A = Query.TheMask->findSlice ("A");
	std::vector<float> data;

	for(int y = procWindow.y1; y < procWindow.y2; y++) 
	{
		// Convert to top-bottom
		const std::pair<int,int> size = Query.TheMask->getSize ();
		const int _y = size.second-(int)((double)y/renderScale.y)-1;
		if(gEffectHost->abort(effect)) break;

		OfxRGBAColourF *dstPix = pixelAddress(dst, dstRect, procWindow.x1, y, dstBytesPerLine);

		for(int x = procWindow.x1; x < procWindow.x2; x++)
		{
			const int _x = (int)((double)x/renderScale.x);
			dstPix->r = dstPix->g = dstPix->b = dstPix->a = 0;
			if (Black)
				continue;

			if (Colors)
			{
				// False colors
				const int n = Query.TheMask->getSampleN (_x, _y);
				for (int s = 0; s < n; ++s)
				{
					Query.TheMask->getSample (_x, _y, s, sample);
					const float a = A == -1 ? 1.f : float (sample.Values[A]);
					const OfxRGBColourF h = haltonColors (sample.Id);
					const bool selected = Query.isSelected (sample.Id);
					const OfxRGBColourF c = {
						R == -1 ? 1.f : float (sample.Values[R]), 
						G == -1 ? 1.f : float (sample.Values[G]),
						B == -1 ? 1.f : float (sample.Values[B])};
					dstPix->r += selected ? c.r : powf (h.r, 1.f/0.3f)*a;
					dstPix->g += selected ? c.g : powf (h.g, 1.f/0.3f)*a;
					dstPix->b += selected ? c.b : powf (h.b, 1.f/0.3f)*a;
					dstPix->a += a;
				}
			}
			else
			{
				Query.getSliceData (_x, _y, data);
				const float a = A == -1 ? 1.f : data[A];
				dstPix->r = R == -1 ? a : data[R];
				dstPix->g = G == -1 ? a : data[G];
				dstPix->b = B == -1 ? a : data[B];
				dstPix->a = a;
			}
			dstPix++;
		}
	}
}

static OfxStatus purgeCaches(OfxImageEffectHandle effect)
{
	Instance *instance = getInstanceData(effect);
	instance->LastMaskFilename = "";
	return kOfxStatOK;
}

// the process code  that the host sees
static OfxStatus render(OfxImageEffectHandle effect,
						OfxPropertySetHandle inArgs,
						OfxPropertySetHandle /*outArgs*/)
{
	// get the render window and the time from the inArgs
	OfxTime time;
	OfxRectI renderWindow;
	OfxStatus status = kOfxStatOK;

	gPropHost->propGetDouble(inArgs, kOfxPropTime, 0, &time);
	gPropHost->propGetIntN(inArgs, kOfxImageEffectPropRenderWindow, 4, &renderWindow.x1);

	// retrieve any instance data associated with this effect
	Instance *instance = getInstanceData(effect);

	// property handles and members of each image
	// in reality, we would put this in a struct as the C++ support layer does
	OfxPropertySetHandle outputImg = NULL;
	int dstRowBytes, dstBitDepth;
	bool dstIsAlpha;
	OfxRectI dstRect;
	void *dst;

	try 
	{
		outputImg = ofxuGetImage(instance->OutputClip, time, dstRowBytes, dstBitDepth, dstIsAlpha, dstRect, dst);
		if(outputImg == NULL)
			throw OfxuNoImageException();

		// get the render scale
		OfxPointD renderScale;
		gPropHost->propGetDoubleN(inArgs, kOfxImageEffectPropRenderScale, 2, &renderScale.x);
		bool black;
		const std::string finalName = computeFinalName (inArgs, instance, black);
		const char *pattern;
		gParamHost->paramGetValue(instance->Pattern, &pattern);
		int colors;
		gParamHost->paramGetValue(instance->Colors, &colors);
		int _invert;
		gParamHost->paramGetValue(instance->Invert, &_invert);
		const bool invert = _invert != 0;
		int alpha;
		gParamHost->paramGetValue(instance->Alpha, &alpha);

		// Split the string in strings
		std::vector<std::string> patterns;
		std::string line;
		while (true)
		{
			const char c = *(pattern++);
			if (c == '\n' || c == '\r' || c == '\0')
			{
				if (!line.empty())
				{
					patterns.push_back (line);
					line.clear ();
				}
			}
			else
				line += c;
			if (c == '\0')
				break;
		}

		try
		{
			// Same file ?
			if (instance->LastMaskFilename != finalName)
			{
				// Already in cache
				instance->Mask.read (finalName.c_str());
				instance->LastMaskFilename = finalName;
			}

			// Check the size
			std::pair<int,int> size = instance->Mask.getSize ();
			if (size.first < renderWindow.x2 || renderWindow.x1 < 0 ||
				size.second < renderWindow.y2 || renderWindow.y1 < 0)
				return kOfxStatFailed;

			// Initialize re2
			re2::RE2::Options			options;
			re2::RE2::Anchor			anchor = re2::RE2::UNANCHORED;
			re2::RE2::Set set (options, anchor);
			for (const auto &pattern : patterns)
				if (set.Add (removeNeg (pattern), NULL) < 0)
					throw std::runtime_error ("Bad regular expression");

			if (!patterns.empty () && !set.Compile ())
					throw std::runtime_error ("Bad regular expression");
			std::vector<int> matched;

			auto match = [&set,&patterns,&matched,invert] (const char *name)->bool
			{
				bool match = false;
				if (!patterns.empty ())
				{
					matched.clear ();
					set.Match (name, &matched);
					for (auto m : matched)
					{
						// Negative match
						if (patterns[m][0] == '-')
							return invert;
						else
							match = true;
					}
				}
				return match^invert;
			};
			openexrid::Query query (&instance->Mask, match);

			// do the rendering
			Processor fred (effect, renderScale, dst, dstRect, dstRowBytes, renderWindow, query, colors != 0, black, alpha != 0);
			fred.process();
		}
		catch (const std::exception &e)
		{
			std::cerr << e.what () << std::endl;
			status = kOfxStatFailed;
		}

	}
	catch(OfxuNoImageException &) 
	{
		// if we were interrupted, the failed fetch is fine, just return kOfxStatOK
		// otherwise, something wierd happened
		if(!gEffectHost->abort(effect)) 
			status = kOfxStatFailed;
	}

	// release the data pointers
	if(outputImg)
		gEffectHost->clipReleaseImage(outputImg);

	return status;
}

// Set our clip preferences 
static OfxStatus getClipPreferences(OfxImageEffectHandle effect, OfxPropertySetHandle inArgs, OfxPropertySetHandle outArgs)
{
	Instance *instance = getInstanceData(effect);

	// Output is pre multiplied
	gPropHost->propSetString(outArgs, kOfxImageEffectPropPreMultiplication, 0, kOfxImagePreMultiplied);
	gPropHost->propSetInt(outArgs, kOfxImageEffectFrameVarying, 0, isFileAnimated (inArgs, instance) ? 1 : 0);

	return kOfxStatOK;
}

//  describe the plugin in context
static OfxStatus describeInContext(OfxImageEffectHandle effect, OfxPropertySetHandle inArgs)
{
	OfxPropertySetHandle clipProps;
	// define the single output clip in both contexts
	gEffectHost->clipDefine(effect, kOfxImageEffectOutputClipName, &clipProps);
	gPropHost->propSetString(clipProps, kOfxImageEffectPropSupportedComponents, 0, kOfxImageComponentRGBA);

	// define the parameters for this context

	// get a pointer to the effect's parameter set
	OfxParamSetHandle paramSet;
	gEffectHost->getParamSet(effect, &paramSet);

	// our 2 corners are normalised spatial 2D doubles
	OfxPropertySetHandle paramProps;

	// The input filename
	gParamHost->paramDefine(paramSet, kOfxParamTypeString, "file", &paramProps);
	gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "The openexrid file");
	gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "file");
	gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "file");
	gPropHost->propSetInt(paramProps, kOfxParamPropAnimates, 0, 0);
	gPropHost->propSetString(paramProps, kOfxParamPropStringMode, 0, kOfxParamStringIsFilePath);

	gParamHost->paramDefine(paramSet, kOfxParamTypeInteger, "firstFrame", &paramProps);
	gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "The frame range where this sequence will be displayed");
	gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "firstFrame");
	gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "first frame");
	gPropHost->propSetInt(paramProps, kOfxParamPropAnimates, 0, 0);
	gPropHost->propSetInt(paramProps, kOfxParamPropDisplayMin, 0, 0);
	gPropHost->propSetInt(paramProps, kOfxParamPropDisplayMax, 0, 100);
	gPropHost->propSetInt(paramProps, "OfxParamPropLayoutHint", 0, 2);

	gParamHost->paramDefine(paramSet, kOfxParamTypeChoice, "before", &paramProps);
	gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "Behaviour before the frame range");
	gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "before");
	gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "before");
	gPropHost->propSetInt(paramProps, kOfxParamPropAnimates, 0, 0);
	gPropHost->propSetString(paramProps, kOfxParamPropChoiceOption, 0, "hold");
	gPropHost->propSetString(paramProps, kOfxParamPropChoiceOption, 1, "loop");
	gPropHost->propSetString(paramProps, kOfxParamPropChoiceOption, 2, "bounce");
	gPropHost->propSetString(paramProps, kOfxParamPropChoiceOption, 3, "black");
	gPropHost->propSetString(paramProps, kOfxParamPropChoiceOption, 4, "error");	

	gParamHost->paramDefine(paramSet, kOfxParamTypeInteger, "lastFrame", &paramProps);
	gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "The frame range where this sequence will be displayed");
	gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "lastFrame");
	gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "last frame");
	gPropHost->propSetInt(paramProps, kOfxParamPropAnimates, 0, 0);
	gPropHost->propSetInt(paramProps, kOfxParamPropDisplayMin, 0, 0);
	gPropHost->propSetInt(paramProps, kOfxParamPropDisplayMax, 0, 100);
	gPropHost->propSetInt(paramProps, "OfxParamPropLayoutHint", 0, 2);

	gParamHost->paramDefine(paramSet, kOfxParamTypeChoice, "after", &paramProps);
	gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "Behaviour after the frame range");
	gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "after");
	gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "after");
	gPropHost->propSetInt(paramProps, kOfxParamPropAnimates, 0, 0);
	gPropHost->propSetString(paramProps, kOfxParamPropChoiceOption, 0, "hold");
	gPropHost->propSetString(paramProps, kOfxParamPropChoiceOption, 1, "loop");
	gPropHost->propSetString(paramProps, kOfxParamPropChoiceOption, 2, "bounce");
	gPropHost->propSetString(paramProps, kOfxParamPropChoiceOption, 3, "black");
	gPropHost->propSetString(paramProps, kOfxParamPropChoiceOption, 4, "error");	

	gParamHost->paramDefine(paramSet, kOfxParamTypeChoice, "frame", &paramProps);
	gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "Choose the first frame mode");
	gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "frame");
	gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "frame");
	gPropHost->propSetInt(paramProps, kOfxParamPropAnimates, 0, 0);
	gPropHost->propSetString(paramProps, kOfxParamPropChoiceOption, 0, "start at");
	gPropHost->propSetString(paramProps, kOfxParamPropChoiceOption, 1, "offset");
	gPropHost->propSetInt(paramProps, "OfxParamPropLayoutHint", 0, 2);

	gParamHost->paramDefine(paramSet, kOfxParamTypeInteger, "offset", &paramProps);
	gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "The offset or the frame start");
	gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "offset");
	gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "offset");
	gPropHost->propSetInt(paramProps, kOfxParamPropAnimates, 0, 0);
	gPropHost->propSetInt(paramProps, kOfxParamPropDisplayMin, 0, 0);
	gPropHost->propSetInt(paramProps, kOfxParamPropDisplayMax, 0, 100);

	// The mask pattern
	gParamHost->paramDefine(paramSet, kOfxParamTypeString, "pattern", &paramProps);
	gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "The object selection pattern");
	gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "pattern");
	gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "Pattern");
	gPropHost->propSetInt(paramProps, kOfxParamPropAnimates, 0, 0);
	gPropHost->propSetString(paramProps, kOfxParamPropStringMode, 0, kOfxParamStringIsMultiLine);

	// The false color button
	gParamHost->paramDefine(paramSet, kOfxParamTypeBoolean, "colors", &paramProps);
	gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "Show the image with false colors");
	gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "colors");
	gPropHost->propSetInt(paramProps, kOfxParamPropAnimates, 0, 0);
	gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "false colors");

	// The false color button
	gParamHost->paramDefine(paramSet, kOfxParamTypeBoolean, "invert", &paramProps);
	gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "Invert the selection");
	gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "invert");
	gPropHost->propSetInt(paramProps, kOfxParamPropAnimates, 0, 0);
	gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "invert");

	// The false color button
	gParamHost->paramDefine(paramSet, kOfxParamTypeBoolean, "alpha", &paramProps);
	gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "Render the alpha");
	gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "alpha");
	gPropHost->propSetInt(paramProps, kOfxParamPropAnimates, 0, 0);
	gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "alpha");

	return kOfxStatOK;
}

// the plugin's description routine
static OfxStatus describe(OfxImageEffectHandle effect)
{
	// first fetch the host APIs, this cannot be done before this call
	OfxStatus stat;
	if((stat = ofxuFetchHostSuites()) != kOfxStatOK)
		return stat;

	// get a pointer to the effect's set of properties
	OfxPropertySetHandle effectProps;
	gEffectHost->getPropertySet(effect, &effectProps);

	// set the bit depths the plugin can handle
	gPropHost->propSetString(effectProps, kOfxImageEffectPropSupportedPixelDepths, 2, kOfxBitDepthFloat);

	// set some labels and the group it belongs to
	gPropHost->propSetString(effectProps, kOfxPropLabel, 0, "OpenEXRId");
	gPropHost->propSetString(effectProps, kOfxImageEffectPluginPropGrouping, 0, "Image");

	// define the contexts we can be used in
	gPropHost->propSetString(effectProps, kOfxImageEffectPropSupportedContexts, 0, kOfxImageEffectContextGeneral);

	// set the property that is the overlay's main entry point for the plugin
	extern OfxStatus overlayMain(const char *action,  const void *handle, OfxPropertySetHandle inArgs, OfxPropertySetHandle /*outArgs*/);
	gPropHost->propSetPointer(effectProps, kOfxImageEffectPluginPropOverlayInteractV1, 0,  (void *) overlayMain);

	gPropHost->propSetString(effectProps, kOfxImageEffectPropClipPreferencesSlaveParam, 0, "file");

	return kOfxStatOK;
}

// The main function
static OfxStatus pluginMain(const char *action,  const void *handle, OfxPropertySetHandle inArgs, OfxPropertySetHandle outArgs)
{
	// cast to appropriate type
	OfxImageEffectHandle effect = (OfxImageEffectHandle ) handle;
	try 
	{
		if(strcmp(action, kOfxActionDescribe) == 0)
			return describe(effect);
		else if(strcmp(action, kOfxImageEffectActionDescribeInContext) == 0)
			return describeInContext(effect, inArgs);
		else if(strcmp(action, kOfxActionCreateInstance) == 0)
			return createInstance(effect);
		else if(strcmp(action, kOfxActionDestroyInstance) == 0)
			return destroyInstance(effect);
		else if(strcmp(action, kOfxImageEffectActionRender) == 0)
			return render(effect, inArgs, outArgs);
		else if(strcmp(action, kOfxImageEffectActionGetRegionOfDefinition) == 0)
			return getSpatialRoD(effect, inArgs, outArgs);
		else if(strcmp(action, kOfxImageEffectActionGetClipPreferences) == 0)
			return getClipPreferences(effect, inArgs, outArgs);
		else if(strcmp(action, kOfxActionPurgeCaches) == 0)
			return purgeCaches(effect);
		else if(strcmp(action,kOfxActionInstanceChanged) == 0)
			return onInstanceChanged(effect, inArgs);
	} 
	catch (std::bad_alloc)
	{
		// catch memory
		return kOfxStatErrMemory;
	}
	catch (const std::exception &e)
	{
		// standard exceptions
		std::cerr << e.what () << std::endl;
		return kOfxStatErrUnknown;
	} 
	catch (int err) 
	{
		// ho hum, gone wrong somehow
		return err;
	}
	catch ( ... ) 
	{
		// everything else
		//std::cout << "OFX Plugin error" << std::endl;
		return kOfxStatErrUnknown;
	}
    
	// other actions to take the default value
	return kOfxStatReplyDefault;
}

// function to set the host structure
static void setHostFunc(OfxHost *hostStruct)
{
	gHost = hostStruct;
}

static OfxPlugin basicPlugin = 
{
	kOfxImageEffectPluginApi,
	1,
	"fr.mercenariesengineering.openexrid",
	1,
	0,
	setHostFunc,
	pluginMain
};

EXPORT OfxPlugin *OfxGetPlugin(int nth)
{
	if(nth == 0)
		return &basicPlugin;
	return 0;
}
 
EXPORT int OfxGetNumberOfPlugins(void)
{
	return 1;
}
