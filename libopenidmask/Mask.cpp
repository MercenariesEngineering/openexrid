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
#include <ImfHeader.h>
#include <ImfDeepFrameBuffer.h>
#include <ImfDeepScanLineInputFile.h>
#include <ImfDeepScanLineOutputFile.h>
#include <ImfIntAttribute.h>
#include <ImfPartType.h>
#include <ImfStringAttribute.h>
#include <ImathBox.h>
#include <assert.h>

using namespace Imf;
using namespace Imath;
using namespace std;
using namespace openidmask;

// ***************************************************************************

Mask::Mask () : _Width (0), _Height (0) {}

// ***************************************************************************

void Mask::read (const char *filename)
{
	DeepScanLineInputFile file (filename);
	const Header& header = file.header();

	// The data window is supposed full for now..
	const Box2i dataWindow = header.dataWindow();
	_Width = dataWindow.max.x - dataWindow.min.x + 1;
	_Height = dataWindow.max.y - dataWindow.min.y + 1;

	// Check the version
	const Imf::IntAttribute *version = header.findTypedAttribute<Imf::IntAttribute> ("OIMVersion");
	if (!version)
		throw runtime_error ("The OIMVersion attribute is missing");
	if (version->value () > (int)_Version)
		throw runtime_error ("The file has been created by an unknown version of the library");

	// Get the name attribute
	const Imf::StringAttribute *names = header.findTypedAttribute<Imf::StringAttribute> ("OIMNames");
	if (!names)
		throw runtime_error ("The OIMNames attribute is missing");
	
	// Copy the names
	_Names = names->value ();

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
	_PixelsIndexes.resize (_Width*_Height+1);

	// Initialize the frame buffer
	DeepFrameBuffer frameBuffer;
	frameBuffer.insertSampleCountSlice (Slice (UINT,
		(char *) (&_PixelsIndexes[0]),
		sizeof (uint32_t),
		sizeof (uint32_t)*_Width));

	// For each pixel of a single line, the pointer on the id values
	vector<uint32_t*> id (_Width);
	frameBuffer.insert ("OIMID", DeepSlice (UINT, (char *)&id[0], sizeof (uint32_t*), 0, sizeof (Sample)));

	// For each pixel of a single line, the pointer on the coverage values
	vector<float*> coverage (_Width);
	frameBuffer.insert ("OIMA", DeepSlice (FLOAT, (char *)&coverage[0], sizeof (float*), 0, sizeof (Sample)));

	file.setFrameBuffer(frameBuffer);

	// Read the whole pixel sample counts
	file.readPixelSampleCounts(0, _Height-1);

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
	_Samples.clear ();
	_Samples.resize (index, {0,0});

	// For each line
	int i = 0;
	for (int y = 0; y < _Height; y++)
	{
		// For each pixel
		for (int x = 0; x < _Width; x++, ++i)
		{
			// The sample id and coverage pointers for this pixel
			const uint32_t count = _PixelsIndexes[i+1]-_PixelsIndexes[i];

			// Avoid invalide indexes
			id[x] = count ? &_Samples[_PixelsIndexes[i]].Id : NULL;
			coverage[x] = count ? &_Samples[_PixelsIndexes[i]].Coverage : NULL;
		}
		file.readPixels (y);
	}
}

// ***************************************************************************

Mask::Mask (const Builder &builder, const std::vector<std::string> &names) : _Width (builder._Width), _Height (builder._Height)
{
	// * Build _NamesIndexes

	// First, fill _NamesIndexes with the size of each string
	const size_t namesN = names.size ();
	_NamesIndexes.resize (namesN, 0);
	for (size_t i = 0; i < namesN; ++i)
		// Including the ending \0
		_NamesIndexes[i] = (uint32_t)names[i].length()+1;

	// Accumulates the string size to build a string index
	uint32_t index = 0;
	for (auto &i : _NamesIndexes)
	{
		const uint32_t tmp = i;
		i = index;
		index += tmp;
	}

	// Allocates the string buffer
	_Names.reserve (index);

	// Concatenate the names into _Names
	for (const auto &name : names)
	{
		_Names += name;
		_Names += '\0';
	}

	// * Build _PixelsIndexes

	// First accumulates the samples count to build an index
	// Current index
	size_t indexN = 0;
	// We need one more index to get the size of the last pixel
	_PixelsIndexes.reserve (_Width*_Height+1);
	for (const auto &p : builder._Pixels)
	{
		_PixelsIndexes.push_back ((uint32_t)indexN);
		indexN += p.size ();
	}
	// One last index to get the size of the last pixel
	_PixelsIndexes.push_back ((uint32_t)indexN);

	// Concatenate the samples
	_Samples.reserve (indexN);
	for (const auto &samples : builder._Pixels)
		_Samples.insert (_Samples.end (), samples.begin (), samples.end ());
}

// ***************************************************************************

void Mask::write (const char *filename, Compression compression) const
{
	// EXR Header
	// Right now, the image window is the data window
	Header header (_Width, _Height);
	header.channels().insert ("OIMID", Channel (UINT));
	header.channels().insert ("OIMA", Channel (FLOAT));
	header.setType (DEEPSCANLINE);
	header.compression () = compression;

	// Write the names in an Attribute
	header.insert ("OIMVersion", Imf::IntAttribute (_Version));
	header.insert ("OIMNames", Imf::StringAttribute (_Names));

	DeepScanLineOutputFile file (filename, header);
	DeepFrameBuffer frameBuffer;

	// Build a sample count buffer for a line
	vector<uint32_t> sampleCount (_Width);
	frameBuffer.insertSampleCountSlice (Slice (UINT, (char *)(&sampleCount[0]), 
										sizeof (uint32_t),
										0));

	// A line of id
	vector<const uint32_t*> id (_Width);
	frameBuffer.insert ("OIMID",
						DeepSlice (UINT,
						(char *) (&id[0]),
						sizeof (uint32_t*),
						0,
						sizeof (Sample)));

	// A line of coverage
	vector<const float*> coverage (_Width);
	frameBuffer.insert ("OIMA",
						DeepSlice (FLOAT,
						(char *) (&coverage[0]),
						sizeof (float*),
						0,
						sizeof (Sample)));
	
	file.setFrameBuffer(frameBuffer);

	// For each line
	int i = 0;
	for (int y = 0; y < _Height; y++)
	{
		// For each pixel
		for (int x = 0; x < _Width; x++, ++i)
		{
			sampleCount[x] = _PixelsIndexes[i+1] - _PixelsIndexes[i];
			id[x] = &_Samples[_PixelsIndexes[i]].Id;
			coverage[x] = &_Samples[_PixelsIndexes[i]].Coverage;
		}
		file.writePixels(1);
	}
}

// ***************************************************************************
