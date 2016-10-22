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

#include "Builder.h"
#include "Mask.h"

#include <algorithm>
#include <assert.h>

#include <ImfChannelList.h>
#include <ImfDeepFrameBuffer.h>
#include <ImfDeepScanLineInputFile.h>
#include <ImfDeepScanLineOutputFile.h>
#include <ImfIntAttribute.h>
#include <ImfPartType.h>
#include <ImfStringAttribute.h>
#include <ImathBox.h>

using namespace Imf;
using namespace openexrid;
using namespace std;

// Compression
extern std::string deflate (const char *str, int len);

//**********************************************************************

Builder::Builder (int width, int height, const std::vector<std::string> &slices) : _Width (width), 	_Height (height), _Pixels (width*height), _Slices (slices) {}

//**********************************************************************

void Builder::addCoverage (int x, int y, uint32_t id, const float *sliceValues)
{
	// The pixel sample list
	SampleList &sl = _Pixels[x+y*_Width];
	sl.addCoverage (id, sliceValues, (int)_Slices.size ());
}

//**********************************************************************

void Builder::write (const char *filename, const char *names, int namesLength, Compression compression) const
{
	// EXR Header
	// Right now, the image window is the data window
	Header header (_Width, _Height);
	header.channels().insert ("Id", Channel (UINT));
	for (size_t s = 0; s < _Slices.size (); ++s)
		header.channels().insert (_Slices[s], Channel (HALF));
	header.setType (DEEPSCANLINE);
	header.compression () = compression;

	// Write the names in an Attribute
	header.insert ("EXRIdVersion", Imf::IntAttribute (Mask::Version));
	header.insert ("EXRIdNames", Imf::StringAttribute (deflate (names, namesLength)));

	DeepScanLineOutputFile file (filename, header);
	DeepFrameBuffer frameBuffer;

	// Build a sample count buffer for a line
	vector<uint32_t> sampleCount (_Width);
	frameBuffer.insertSampleCountSlice (Imf::Slice (UINT, (char *)(&sampleCount[0]), 
										sizeof (uint32_t),
										0));

	// A line of id
	vector<uint32_t> ids;
	vector<const uint32_t*> id (_Width);
	frameBuffer.insert ("Id",
						DeepSlice (UINT,
						(char *) (&id[0]),
						sizeof (uint32_t*),
						0,
						sizeof (uint32_t)));

	const int vn = (int)_Slices.size ();

	// A line of coverage
	vector<half> values;
	vector<vector<const half*> > slices (_Slices.size ());
	for (size_t s = 0; s < _Slices.size (); ++s)
	{
		slices[s].resize (_Width);
		frameBuffer.insert (_Slices[s],
							DeepSlice (HALF,
							(char *) (&slices[s][0]),
							sizeof (half*),
							0,
							sizeof (half)*vn));
	}
	
	file.setFrameBuffer(frameBuffer);

	// For each line
	int i = 0;
	for (int y = 0; y < _Height; y++)
	{
		ids.clear ();
		values.clear ();

		// For each pixel
		for (int x = 0; x < _Width; x++)
		{
			const SampleList &sl = _Pixels[i+x];
			const int sn = sl.getSampleN(vn);
			for (int s = 0; s < sn; ++s)
			{
				const uint32_t id = sl.getSampleId(s, vn);
				assert (id < 100);
				ids.push_back (id);
				const float *src = sl.getSampleValues(s, vn);
				for (int v = 0; v < vn; ++v)
					values.push_back(src[v]);
			}

			sampleCount[x] = sn;
		}

		int index = 0;
		for (int x = 0; x < _Width; x++, ++i)
		{
			const int sn = _Pixels[i].getSampleN(vn);

			// Set the ids pointer, it may have changed
			id[x] = sn ? &ids[index] : NULL;
			for (int v = 0; v < vn; ++v)
			{
				// Set the value pointers, they may have changed
				slices[v][x] = sn ? &values[index*vn+v] : NULL;
			}

			index += sn;
		}
		file.writePixels(1);
	}
}

// ***************************************************************************
