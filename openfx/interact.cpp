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

#include <ofxImageEffect.h>
#include <ofxKeySyms.h>
#include <string.h>
#include <math.h>
#include <algorithm>
#include <set>
#ifdef WIN32
#include <Windows.h>
#undef min
#undef max
#endif // WIN32
#include <GL/gl.h>
#include "ofxUtilities.h"
#include "instance.h"

extern bool gIsNuke;

#if defined __APPLE__ || defined linux || defined __FreeBSD__
#  define EXPORT __attribute__((visibility("default")))
#elif defined _WIN32
#  define EXPORT OfxExport
#else
#  error Not building on your operating system quite yet
#endif

// Split a string in a string set
std::set<std::string> split (const char *str, const std::string &delim)
{
	std::set<std::string> result;
	const char *start = str;
	while (true)
	{
		if (*str == '\0' || delim.find (*str) != delim.npos)
		{
			if (start < str)
				result.emplace (start, str);
			start = str+1;
		}
		if (*str == '\0')
			break;
		++str;
	}
	return result;
}

// Join strings in a single string
template<class T>
std::string join (const T &names, const char *sep)
{
	std::string result;
	for (const auto &name : names)
	{
		if (!result.empty ())
			result += sep;
		result += name;
	}
	return result;
}

// Escape the regexp characters
std::string escapeRegExp (const char *s)
{
	const char *sc = ".^$*+?()[{\\|";
	std::string result = "^";
	while (*s)
	{
		if (std::find (sc, sc+12, *s) != sc+12)
			result += gIsNuke ? "\\\\" : "\\";
		result += *s;
		++s;
	}
	result += "$";
	return result;
}

// Escape the backslash
std::string escape (const char *s)
{
	std::string result;
	while (*s)
	{
		if (*s == '\\')
			result += '\\';
		result += *s;
		++s;
	}
	return result;
}

// the interaction routines
struct MyInteractData {
	int DownX, DownY;
	bool Down, Shift;
	OfxParamHandle patternParam;

	explicit MyInteractData(OfxParamHandle pParam)
	: Down (false), Shift (false)
	, patternParam(pParam)
	{
	}
};

// get the interact data from an interact instance
static MyInteractData *
getInteractData(OfxInteractHandle interactInstance)
{    
  void *dataV = ofxuGetInteractInstanceData(interactInstance);
  return (MyInteractData *) dataV;
}

// creation of an interact instance
static OfxStatus 
interactDescribe(OfxInteractHandle /*interactDescriptor*/)
{

  // and we are good
  return kOfxStatOK;
}

// creation of an interact instance
static OfxStatus interactCreateInstance(OfxImageEffectHandle effect, OfxInteractHandle interactInstance)
{
	// get the parameter set for this effect
	OfxParamSetHandle paramSet;
	gEffectHost->getParamSet(effect, &paramSet);

	// fetch a handle to the point param from the parameter set
	OfxParamHandle patternParam;
	gParamHost->paramGetHandle(paramSet, "pattern", &patternParam, 0);

	// make my interact's instance data
	MyInteractData *data = new MyInteractData(patternParam);

	// and set the interact's data pointer
	ofxuSetInteractInstanceData(interactInstance, (void *) data);

	OfxPropertySetHandle interactProps;
	gInteractHost->interactGetPropertySet(interactInstance, &interactProps);

	// slave this interact to the point param so redraws are triggered cleanly
	gPropHost->propSetString(interactProps, kOfxInteractPropSlaveToParam, 0, "pattern");

	return kOfxStatOK;
}

// destruction of an interact instance
static OfxStatus interactDestroyInstance(OfxImageEffectHandle, OfxInteractHandle interactInstance)
{
	MyInteractData *data = getInteractData(interactInstance);
	delete data;
	return kOfxStatOK;
}

// size of the cross hair in screen pixels
#define kXHairSize 10

// draw an interact instance
static OfxStatus interactDraw(OfxImageEffectHandle  effect, OfxInteractHandle interactInstance, OfxPropertySetHandle drawArgs)
{
	// get my private interact data
	MyInteractData *data = getInteractData(interactInstance);
	Instance *instance = (Instance*)ofxuGetEffectInstanceData(effect);

	if (data->Down)
	{
		double penPos[2];
		gPropHost->propGetDoubleN(drawArgs, kOfxInteractPropPenPosition, 2, penPos);

		// Get the image size
		const std::pair<int, int> &size = instance->Mask.getSize ();
		const float x0 = (float)data->DownX;
		const float y0 = size.second-(float)data->DownY-1;
		const float x1 = (float)penPos[0];
		const float y1 = (float)penPos[1];

		glPushAttrib (GL_ENABLE_BIT|GL_CURRENT_BIT);
			// if the we have selected the Xhair, draw it highlit
			glColor3f(1, 1, 1);
		
			glEnable(GL_COLOR_LOGIC_OP);
			glLogicOp(GL_XOR);
			glLineStipple(1, 0xF0F0);
			glEnable(GL_LINE_STIPPLE);

			glBegin(GL_LINE_STRIP);

				glVertex2f(x0, y0);
				glVertex2f(x1, y0);
				glVertex2f(x1, y1);
				glVertex2f(x0, y1);
				glVertex2f(x0, y0);

			glEnd();
		glPopAttrib ();
	}

	return kOfxStatOK;
}

// function reacting to pen motion
static OfxStatus interactPenMotion(OfxImageEffectHandle effect, OfxInteractHandle interactInstance, OfxPropertySetHandle inArgs)
{
	// get my data handle
	return kOfxStatOK;
}

static OfxStatus interactPenDown(OfxImageEffectHandle  effect, OfxInteractHandle interactInstance, OfxPropertySetHandle inArgs)
{
	// get my data handle
	MyInteractData *data = getInteractData(interactInstance);

	double penPos[2];
	gPropHost->propGetDoubleN(inArgs, kOfxInteractPropPenPosition, 2, penPos);

	Instance *instance = (Instance*)ofxuGetEffectInstanceData(effect);

	// Get the image size
	const std::pair<int, int> &size = instance->Mask.getSize ();
	data->DownX = (int)penPos[0];
	data->DownY = size.second-(int)penPos[1]-1;
	data->Down = true;

	return kOfxStatOK;
}

static OfxStatus interactKeyDown (OfxImageEffectHandle  effect, OfxInteractHandle interactInstance, OfxPropertySetHandle inArgs)
{
	MyInteractData *data = getInteractData(interactInstance);

	int key;
	gPropHost->propGetInt (inArgs, kOfxPropKeySym, 0, &key);
	if (key == kOfxKey_Shift_L || key == kOfxKey_Shift_R)
		data->Shift = true;
	
	return kOfxStatOK;
}

static OfxStatus interactKeyUp (OfxImageEffectHandle  effect, OfxInteractHandle interactInstance, OfxPropertySetHandle inArgs)
{
	MyInteractData *data = getInteractData(interactInstance);

	int key;
	gPropHost->propGetInt (inArgs, kOfxPropKeySym, 0, &key);
	if (key == kOfxKey_Shift_L || key == kOfxKey_Shift_R)
		data->Shift = false;
	
	return kOfxStatOK;
}

static OfxStatus interactPenUp(OfxImageEffectHandle  effect, OfxInteractHandle interactInstance, OfxPropertySetHandle inArgs)
{
	// get my data handle
	MyInteractData *data = getInteractData(interactInstance);
	Instance *instance = (Instance*)ofxuGetEffectInstanceData(effect);

	double penPos[2];
	gPropHost->propGetDoubleN(inArgs, kOfxInteractPropPenPosition, 2, penPos);

	if (data->Down)
	{
		// Get the image size
		const std::pair<int, int> &size = instance->Mask.getSize ();
		const int upX = (int)penPos[0];
		const int upY = size.second-(int)penPos[1]-1;

		std::set<std::string> names;
		openexrid::Sample sample;

		const int alpha = instance->Mask.findSlice ("A");

		const int maxX = std::min (std::max (upX, data->DownX)+1, size.first);
		const int maxY = std::min (std::max (upY, data->DownY)+1, size.first);
		for (int y = std::max (std::min (upY, data->DownY), 0); y < maxY; ++y)
		for (int x = std::max (std::min (upX, data->DownX), 0); x < maxX; ++x)
 		{
 			// Get the max coverage sample in the pixel
			float maxCoverage = 0;
			uint32_t maxId = ~0U;
			const int sampleN = instance->Mask.getSampleN (x, y);
			for (int s = 0; s < sampleN; ++s)
			{
				openexrid::Sample sample;
				instance->Mask.getSample (x, y, s, sample);
				if (alpha == -1 || sample.Values[alpha] > maxCoverage)
				{
					maxId = sample.Id;
					maxCoverage = sample.Values[alpha];
				}
			}

			// Found something ?
			if (maxId != ~0U)
			{
				const char *name = instance->Mask.getName (maxId);
				names.insert (escapeRegExp (name));
			}
		}
 
		// Get the old pattern
		const char *_oldPattern;
		gParamHost->paramGetValue (data->patternParam, &_oldPattern);
		std::string oldPattern = _oldPattern;
		
		/* Nuke escapes the \ of the text parameter */
		if (gIsNuke)
			oldPattern = escape (oldPattern.c_str ());
		std::set<std::string> oldNames (split (oldPattern.c_str (), "\n\r"));

		// Shift ?
		if (data->Shift)
		{
			// Reverse the selection for the previously selected names
			for (const auto &name : oldNames)
			{
				// Try insert
				auto r = names.insert (name);

				// Already there, remove it
				if (!r.second)
					names.erase (r.first);
			}
		}

		// get the point param's value
		std::string pattern = join (names, "\n");
		gParamHost->paramSetValue(data->patternParam, pattern.c_str ());
		data->Down = false;
	}
	return kOfxStatOK;
}

// the entry point for the overlay
OfxStatus overlayMain(const char *action,  const void *handle, OfxPropertySetHandle inArgs, OfxPropertySetHandle /*outArgs*/)
{
	OfxInteractHandle interact = (OfxInteractHandle ) handle;

	OfxPropertySetHandle props;
	gInteractHost->interactGetPropertySet(interact, &props);

	if(strcmp(action, kOfxActionDescribe) == 0)
		return interactDescribe(interact);
	else 
	{
		// fetch the effect instance from the interact
		OfxImageEffectHandle effect;
		gPropHost->propGetPointer(props, kOfxPropEffectInstance, 0, (void **) &effect); 

		if(strcmp(action, kOfxActionCreateInstance) == 0)
			return interactCreateInstance(effect, interact);
		else if(strcmp(action, kOfxActionDestroyInstance) == 0)
			return interactDestroyInstance(effect, interact);
		else if(strcmp(action, kOfxInteractActionDraw) == 0)
			return interactDraw(effect, interact, inArgs);
		else if(strcmp(action, kOfxInteractActionPenMotion) == 0)
			return interactPenMotion(effect, interact, inArgs);
		else if(strcmp(action, kOfxInteractActionPenDown) == 0)
			return interactPenDown(effect, interact, inArgs);
		else if(strcmp(action, kOfxInteractActionPenUp) == 0)
			return interactPenUp(effect, interact, inArgs);
		else if(strcmp(action, kOfxInteractActionKeyDown) == 0)
			return interactKeyDown(effect, interact, inArgs);
		else if(strcmp(action, kOfxInteractActionKeyUp) == 0)
			return interactKeyUp(effect, interact, inArgs);

		return kOfxStatReplyDefault;
	}
	return kOfxStatReplyDefault;
}
