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

#include "instance.h"
#include "dir.h"
#include <cstring>
#include "ofxUtilities.h"
#include <algorithm>
#include <string>
#include <sstream>
#include <iomanip>

/* This file contains the range related functions */

bool matchFramePattern (const std::string &file, const std::string &pre, const std::string &post, size_t off, int n, int &frame)
{
	if (file.size () != pre.size ()+n+post.size ()) return false;
	if (file.compare (0, pre.size (), pre) != 0) return false;
	if (file.compare (pre.size ()+n, file.npos, post) != 0) return false;

	frame = atoi (file.substr (pre.size (), n).c_str ());
	return true;
}

// Get the frame range of a filename using a pattern
bool getFrameRange (const std::string &file, int &start, int &end)
{
	// If the file already contains special characters abort
	const size_t tagS = file.find ('#');
	
	// No frame hashes
	if (tagS == file.npos)
	{
		start = end = 1;
		return true;
	}

	// Count the number of hash
	int n = 1;
	while (file[tagS+n] == '#') ++n;

	std::string pre = file.substr (0, file.find_first_of ("\\/", tagS+n));
	const std::string post = pre.substr (tagS+n);
	const size_t pos = pre.find_last_of ("\\/")+1;	// pos is 0 if not \\ nor /
	pre = pre.substr (pos, tagS-pos);

	// Get the parent folder where the hash tag name is stored
	const std::string parentDir = file.substr (0, pos);

	// Iterates the filenames where the hash tag name is stored
	start = INT_MAX;
	end = INT_MIN;
	DIR *dir = opendir (parentDir.c_str ());
	if (!dir)
		return false;
	dirent *entry;
	while (entry = readdir (dir))
	{
		int frame;
		if (matchFramePattern (entry->d_name, pre, post, tagS-parentDir.size(), n, frame))
		{
			start = std::min (frame, start);
			end = std::max (frame, start);
		}
	}
	closedir (dir);

	return start<=end;
}

// OFX callback for parameter changes
OfxStatus onInstanceChanged(OfxImageEffectHandle effect, OfxPropertySetHandle inArgs)
{
	Instance *instance = (Instance *) ofxuGetEffectInstanceData(effect);
	char *paramName, *changeReason;
	if (gPropHost->propGetString (inArgs, kOfxPropName, 0, &paramName) == kOfxStatOK &&
		gPropHost->propGetString (inArgs, kOfxPropChangeReason, 0, &changeReason) == kOfxStatOK &&
		strcmp (changeReason, kOfxChangeUserEdited) == 0)
	{
		// The user changed some range related parameters
		// Synchronize the other parameters
		OfxPropertySetHandle effectProps;
		gEffectHost->getPropertySet(effect, &effectProps);
		if (strcmp (paramName, "file") == 0)
		{
			int start;
			int end;
			char *file;
			if (gParamHost->paramGetValue (instance->File, &file) == kOfxStatOK &&
				getFrameRange (file, start, end))
			{
				gParamHost->paramSetValue (instance->FirstFrame, start);
				gParamHost->paramSetValue (instance->LastFrame, end);
			}
		}
	}
	return kOfxStatOK;
}

// Positive modulus
int modabs (int a, int b)
{
	return (a = a%b) < 0 ? b+a : a;
}

int clampFrame (int mode, int frame, int len, bool &black)
{
	switch (mode)
	{
	case 3:
		black = true;
	case 0:
		if (frame < 0)
			return 0;
		else
			return len-1;
	case 1:
		return modabs (frame, len);
	case 2:
		int tmp;
		return ((tmp = modabs (frame, len*2-2)) >= len ? len*2-2-tmp : tmp);
	}
	return frame;
};

// Compute the final frame name using the file pattern and the boundary rules
std::string computeFinalName (OfxPropertySetHandle inArgs, Instance *instance, bool &black)
{
	black = false;
	const char *_filename;
	gParamHost->paramGetValue(instance->File, &_filename);
	const std::string filename = _filename;

	const size_t hashPos = filename.find ('#');
	if (hashPos == filename.npos)
		return filename;

	double time;
	gPropHost->propGetDouble(inArgs, kOfxPropTime, 0, &time);

	// Count the number of hash
	int n = 1;
	while (filename[hashPos+n] == '#') ++n;

	// ** Clamp the final time
	int firstFrame;
	gParamHost->paramGetValue (instance->FirstFrame, &firstFrame);
	int lastFrame;
	gParamHost->paramGetValue (instance->LastFrame, &lastFrame);
	int frameMode;
	gParamHost->paramGetValue (instance->Frame, &frameMode);
	int offset;
	gParamHost->paramGetValue (instance->Offset, &offset);
	int before;
	gParamHost->paramGetValue (instance->Before, &before);
	int after;
	gParamHost->paramGetValue (instance->After, &after);

	const int relativeTime = frameMode==0?(int)time-offset:(int)time+offset-firstFrame;
	const int len = lastFrame-firstFrame+1;

	const int finalTime =
	(
		relativeTime < 0 ? clampFrame (before, relativeTime, len, black) :
		relativeTime >= len ? clampFrame (after, relativeTime, len, black) :
		relativeTime
	)+firstFrame;

//	std::cout << "Frame " << finalTime << std::endl;

	std::stringstream ss;
	ss << std::setw(n) << std::setfill('0') << finalTime;
	return filename.substr (0, hashPos) + ss.str () + filename.substr (hashPos+n);
}

bool isFileAnimated (OfxPropertySetHandle inArgs, Instance *instance)
{
	const char *_filename;
	gParamHost->paramGetValue(instance->File, &_filename);
	const std::string filename = _filename;

	const size_t hashPos = filename.find ('#');
	return hashPos != filename.npos;
}
