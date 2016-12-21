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


#include <stdexcept>
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

Builder::Builder (int width, int height, const std::vector<std::string> &slices) : _Width (width), 	_Height (height), _Finished (false), _Pixels (width*height), _Slices (slices) {}

//**********************************************************************

void Builder::addCoverage (int x, int y, uint32_t id, float z, const float *sliceValues)
{
	// The pixel sample list
	SampleList &sl = _Pixels[x+y*_Width];
	sl.addCoverage (id, z, sliceValues, (int)_Slices.size ());
}

//**********************************************************************

static int PtFuncCompare(void const *a, void const *b)
{
	float _a = ((SampleList::Header*)a)->Z;
	float _b = ((SampleList::Header*)b)->Z;
	return _a < _b ? -1 : _a > _b ? 1 : 0;
}

void Builder::finish ()
{
	if (_Finished)
		throw runtime_error ("Builder::finish has been already called");

	std::vector<std::string>::iterator ite = std::find (_Slices.begin(), _Slices.end (), "A");
	if (ite == _Slices.end())
		throw runtime_error ("No A channel");
	const int A = (int)(ite-_Slices.begin ());

	const int vn = (int)_Slices.size ();
	std::vector<float> acc;
	for (vector<SampleList>::iterator ite = _Pixels.begin(); ite != _Pixels.end(); ++ite)
	{
		acc.clear ();
		acc.resize (vn, 0.f);

		const int sn = ite->getSampleN(vn);
		for (int s = 0; s < sn; ++s)
		{
			SampleList::Header &header = ite->getSampleHeader(s,vn);

			// Average the Z
			if (header.Count > 0)
				header.Z /= (float)header.Count;
		}

		// Sort the samples in the pixel by Z
		if (sn > 0)
			qsort (&ite->getSampleHeader(0,vn), sn, sizeof(uint32_t)*(SampleList::HeaderSize+vn), PtFuncCompare);

		// Cumulate the values in the EXR deep image way
		for (int s = 0; s < sn; ++s)
		{
			float *values = ite->getSampleValues (s,vn);
			for (int v = 0; v < vn; ++v)
				values[v] = (acc[v] += values[v]);
		}
		for (int s = sn-1; s > 0; --s)
		{
			float *values = ite->getSampleValues (s,vn);
			float *valuesPrev = ite->getSampleValues (s-1,vn);
			const float oma = 1.f-valuesPrev[A];
			for (int v = 0; v < vn; ++v)
				values[v] = (values[v]-valuesPrev[v])/(oma > 0.f ? oma : 1.f);
		}
	}

	_Finished = true;
}

//**********************************************************************

void Builder::write (const char *filename, const char *names, int namesLength, bool computeDataWindow, Compression compression) const
{
	if (!_Finished)
		throw runtime_error ("Builder::finish has not been called");

	const int vn = (int)_Slices.size ();

	Imath::Box2i dataW;

	// If required, compute the data window
	if (computeDataWindow)
	{
		for (int y = 0; y < _Height; ++y)
		for (int x = 0; x < _Width; ++x)
		{
			if (_Pixels[x+y*_Width].getSampleN (vn) > 0)
				dataW.extendBy (Imath::V2i (x, y));
		}

		// Exr doesn't like empty images
		if (dataW.isEmpty())
			dataW.extendBy (Imath::V2i(0,0));
	}
	else
			// Or use the whole image
			dataW = Imath::Box2i (Imath::V2i(0,0),Imath::V2i(_Width-1,_Height-1));

	// EXR Header
	// Right now, the image window is the data window
	Header header (_Width, _Height, dataW);
	header.channels().insert ("Id", Channel (UINT));
	header.channels().insert ("Z", Channel (FLOAT));
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

	// A line of Z
	vector<float> _z;
	vector<const float*> z (_Width);
	frameBuffer.insert ("Z",
						DeepSlice (FLOAT,
						(char *) (&z[0]),
						sizeof (float*),
						0,
						sizeof (float)));

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
	for (int y = dataW.min.y; y <= dataW.max.y; y++)
	{
		const int lineStart = y*_Width;

		ids.clear ();
		_z.clear ();
		values.clear ();

		// For each pixel
		for (int x = 0; x < _Width; x++)
		{
			const SampleList &sl = _Pixels[lineStart+x];
			const int sn = sl.getSampleN(vn);
			for (int s = 0; s < sn; ++s)
			{
				const SampleList::Header &header = sl.getSampleHeader(s, vn);
				ids.push_back (header.Id);
				_z.push_back (header.Z);
				const float *src = sl.getSampleValues(s, vn);
				for (int v = 0; v < vn; ++v)
					values.push_back(src[v]);
			}

			sampleCount[x] = sn;
		}

		int index = 0;
		for (int x = 0; x < _Width; x++)
		{
			const int sn = _Pixels[lineStart+x].getSampleN(vn);

			// Set the ids pointer, it may have changed
			id[x] = sn ? &ids[index] : NULL;
			z[x] = sn ? &_z[index] : NULL;
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
