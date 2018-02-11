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

#include "Mask.h"
#include "Builder.h"

#include <ImfChannelList.h>
#include <ImfDeepFrameBuffer.h>
#include <ImfDeepScanLineInputFile.h>
#include <ImfDeepScanLineOutputFile.h>
#include <ImfIntAttribute.h>
#include <ImfPartType.h>
#include <ImfStringAttribute.h>
#include <ImathBox.h>
#include <assert.h>
#include <stdexcept>

using namespace Imf;
using namespace Imath;
using namespace std;
using namespace openexrid;

// Compression
extern std::string b64decode (const std::string& str);
extern std::string inflate (const std::string& str);

// ***************************************************************************

Mask::Mask () : _Width (0), _Height (0), _A (-1) {}

// ***************************************************************************

void Mask::read (const char *filename)
{
	DeepScanLineInputFile file (filename);
	const Header& header = file.header();

	const Box2i displayWindow = header.displayWindow();
	_Width = displayWindow.max.x - displayWindow.min.x + 1;
	_Height = displayWindow.max.y - displayWindow.min.y + 1;

	const Box2i dataWindow = header.dataWindow();

	// Check the version
	const Imf::IntAttribute *version = header.findTypedAttribute<Imf::IntAttribute> ("EXRIdVersion");
	if (!version)
		throw runtime_error ("The EXRIdVersion attribute is missing");
	if (version->value () > (int)Version)
		throw runtime_error ("The file has been created by an unknown version of the library");

	// Get the name attribute
	const Imf::StringAttribute *names = header.findTypedAttribute<Imf::StringAttribute> ("EXRIdNames");
	if (!names)
		throw runtime_error ("The EXRIdNames attribute is missing");
	
	// Copy the names
	if (version->value() < 3)
		_Names = inflate (names->value ());
	else
		_Names = inflate (b64decode(names->value ()));

	// Count the names
	int namesN = 0;
	{
		size_t index = 0;
		while (index < _Names.size ())
		{
			index += strnlen (&_Names[index], _Names.size ()-index)+1;
			++namesN;
		}
	}

	// Build the name indexes
	_NamesIndexes.clear ();
	_NamesIndexes.reserve (namesN);
	{
		// Current index
		size_t index = 0;
		while (index < _Names.size ())
		{
			// Push the index of the current name
			_NamesIndexes.push_back ((uint32_t)index);
			index += strnlen (&_Names[index], _Names.size ()-index)+1;
		}
		assert (_NamesIndexes.size () == namesN);
	}

	// Allocate the pixel indexes
	_PixelsIndexes.clear ();
	_PixelsIndexes.resize (_Width*_Height+1, 0);

	// Initialize the frame buffer
	DeepFrameBuffer frameBuffer;
	frameBuffer.insertSampleCountSlice (Imf::Slice (UINT,
		(char *) (&_PixelsIndexes[0]),
		sizeof (uint32_t),
		sizeof (uint32_t)*_Width));

	// For each pixel of a single line, the pointer on the id values
	vector<uint32_t*> id (_Width);
	frameBuffer.insert ("Id", DeepSlice (UINT, (char *)&id[0], sizeof (uint32_t*), 0, sizeof (uint32_t)));

	// Read the slices
	_Slices.clear ();
	const ChannelList &channels = header.channels ();
	for (ChannelList::ConstIterator channel = channels.begin (); channel != channels.end (); ++channel)
	{
		if (channel.channel ().type == HALF)
			_Slices.push_back (channel.name());
	}

	// For each pixel of a single line, the pointer on the coverage values
	vector<vector<half*> > slices (_Slices.size ());
	for (size_t s = 0; s < _Slices.size (); ++s)
	{
		slices[s].resize (_Width);
		frameBuffer.insert (_Slices[s], DeepSlice (HALF, (char *)&slices[s][0], sizeof (half*), 0, sizeof (half)));
	}

	file.setFrameBuffer(frameBuffer);

	// Read the whole pixel sample counts
	file.readPixelSampleCounts(dataWindow.min.y, dataWindow.max.y);

	// Accumulate the sample counts to get the indexes
	// The current index
	uint32_t index = 0;
	for (int i = 0; i < _Width*_Height+1; ++i)
	{
		const uint32_t n = _PixelsIndexes[i];
		// Convert from a sample count to a sample index
		_PixelsIndexes[i] = index;
		index += n;
	}

	// Resize the samples
	_Ids.clear ();
	_Ids.resize (index, 0);
	_SlicesData.resize (_Slices.size ());
	for (size_t s = 0; s < _Slices.size (); ++s)
	{
		_SlicesData[s].clear ();
		_SlicesData[s].resize (index, 0.f);
	}

	// For each line
	for (int y = dataWindow.min.y; y <= dataWindow.max.y; y++)
	{
		const int lineStart = y*_Width;
		// For each pixel
		for (int x = 0; x < _Width; x++)
		{
			const int _i = lineStart+x;
			// The sample id and coverage pointers for this pixel
			const uint32_t count = _PixelsIndexes[_i+1]-_PixelsIndexes[_i];

			// Avoid invalide indexes
			id[x] = count ? &_Ids[_PixelsIndexes[_i]] : NULL;
			for (size_t s = 0; s < _Slices.size (); ++s)
				slices[s][x] = count ? &_SlicesData[s][_PixelsIndexes[_i]] : NULL;
		}
		file.readPixels (y);

		// In version 1, samples are already uncumulated
		if (version->value () > 1)
		{
			const int A = findSlice ("A");
			for (int x = 0; x < _Width; x++)
			{
				const int _i = lineStart+x;
				const uint32_t count = _PixelsIndexes[_i+1]-_PixelsIndexes[_i];
				if (count == 0) continue;
				// Uncumulate the pixels value
				float prevAlpha = 0.f;
				for (uint32_t s = 0; s < count; ++s)
				{
					const int curr = _PixelsIndexes[_i]+s;
					const float alpha = (float)_SlicesData[A][curr];
					for (size_t v = 0; v < _Slices.size (); ++v)
						_SlicesData[v][curr] = (1.f-prevAlpha)*_SlicesData[v][curr];
					prevAlpha += (1.f-prevAlpha)*alpha;
				}
			}
		}
	}
}

// ***************************************************************************
