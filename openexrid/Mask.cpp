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
#include <stdexcept>

using namespace Imf;
using namespace Imath;
using namespace std;
using namespace openexrid;

// Compression
extern std::string deflate (const std::string& str);
extern std::string inflate (const std::string& str);

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
	const Imf::IntAttribute *version = header.findTypedAttribute<Imf::IntAttribute> ("EXRIdVersion");
	if (!version)
		throw runtime_error ("The EXRIdVersion attribute is missing");
	if (version->value () > (int)_Version)
		throw runtime_error ("The file has been created by an unknown version of the library");

	// Get the name attribute
	const Imf::StringAttribute *names = header.findTypedAttribute<Imf::StringAttribute> ("EXRIdNames");
	if (!names)
		throw runtime_error ("The EXRIdNames attribute is missing");
	
	// Copy the names
	_Names = inflate (names->value ());

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
	_Ids.clear ();
	_Ids.resize (index, 0);
	_SlicesData.resize (_Slices.size ());
	for (size_t s = 0; s < _Slices.size (); ++s)
	{
		_SlicesData[s].clear ();
		_SlicesData[s].resize (index, 0.f);
	}

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
			id[x] = count ? &_Ids[_PixelsIndexes[i]] : NULL;
			for (size_t s = 0; s < _Slices.size (); ++s)
				slices[s][x] = count ? &_SlicesData[s][_PixelsIndexes[i]] : NULL;
		}
		file.readPixels (y);
	}
}

// ***************************************************************************

Mask::Mask (const Builder &builder, const std::vector<std::string> &names) : _Width (builder._Width), _Height (builder._Height)
{
	_Slices = builder._Slices;

	// * Build _NamesIndexes
	const int valueN = (int)builder._Slices.size ();

	// First, fill _NamesIndexes with the size of each string
	const size_t namesN = names.size ();
	_NamesIndexes.resize (namesN, 0);
	for (size_t i = 0; i < namesN; ++i)
		// Including the ending \0
		_NamesIndexes[i] = (uint32_t)names[i].length()+1;

	// Accumulates the string size to build a string index
	uint32_t index = 0;
	for (std::vector<uint32_t>::iterator iti = _NamesIndexes.begin (); iti != _NamesIndexes.end (); ++iti)
	{
		const uint32_t tmp = *iti;
		*iti = index;
		index += tmp;
	}

	// Allocates the string buffer
	_Names.reserve (index);

	// Concatenate the names into _Names
	for (std::vector<std::string>::const_iterator itn = names.begin (); itn != names.end (); ++itn)
	{
		_Names += *itn;
		_Names += '\0';
	}

	// * Build _PixelsIndexes

	// First accumulates the samples count to build an index
	// Current index
	size_t indexN = 0;
	// We need one more index to get the size of the last pixel
	_PixelsIndexes.reserve (_Width*_Height+1);
	std::vector<SampleList>::const_iterator	itp;
	for (itp = builder._Pixels.begin (); itp != builder._Pixels.end (); ++itp)
	{
		_PixelsIndexes.push_back ((uint32_t)indexN);
		indexN += itp->getSampleN (valueN);
	}
	// One last index to get the size of the last pixel
	_PixelsIndexes.push_back ((uint32_t)indexN);

	// Concatenate the samples
	_Ids.reserve (indexN);
	_SlicesData.resize (_Slices.size ());
	for (size_t s = 0; s < _Slices.size (); ++s)
		_SlicesData[s].reserve (indexN);
	for (itp = builder._Pixels.begin (); itp != builder._Pixels.end (); ++itp)
	{
		const int sampleN = itp->getSampleN (valueN);
		for (int s = 0; s < sampleN; ++s)
		{
			_Ids.push_back (itp->getSampleId (s, valueN));
			for (size_t sl = 0; sl < _Slices.size (); ++sl)
				_SlicesData[sl].push_back (itp->getSampleValues (s, valueN)[sl]);
		}
	}
}

// ***************************************************************************

void Mask::write (const char *filename, Compression compression) const
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
	header.insert ("EXRIdVersion", Imf::IntAttribute (_Version));
	header.insert ("EXRIdNames", Imf::StringAttribute (deflate (_Names)));

	DeepScanLineOutputFile file (filename, header);
	DeepFrameBuffer frameBuffer;

	// Build a sample count buffer for a line
	vector<uint32_t> sampleCount (_Width);
	frameBuffer.insertSampleCountSlice (Imf::Slice (UINT, (char *)(&sampleCount[0]), 
										sizeof (uint32_t),
										0));

	// A line of id
	vector<const uint32_t*> id (_Width);
	frameBuffer.insert ("Id",
						DeepSlice (UINT,
						(char *) (&id[0]),
						sizeof (uint32_t*),
						0,
						sizeof (uint32_t)));

	// A line of coverage
	vector<vector<const half*> > slices (_Slices.size ());
	for (size_t s = 0; s < _Slices.size (); ++s)
	{
		slices[s].resize (_Width);
		frameBuffer.insert (_Slices[s],
							DeepSlice (HALF,
							(char *) (&slices[s][0]),
							sizeof (half*),
							0,
							sizeof (half)));
	}
	
	file.setFrameBuffer(frameBuffer);

	// For each line
	int i = 0;
	for (int y = 0; y < _Height; y++)
	{
		// For each pixel
		for (int x = 0; x < _Width; x++, ++i)
		{
			sampleCount[x] = _PixelsIndexes[i+1] - _PixelsIndexes[i];
			id[x] = &_Ids[_PixelsIndexes[i]];
			for (size_t s = 0; s < _Slices.size (); ++s)
				slices[s][x] = &_SlicesData[s][_PixelsIndexes[i]];
		}
		file.writePixels(1);
	}
}

// ***************************************************************************
