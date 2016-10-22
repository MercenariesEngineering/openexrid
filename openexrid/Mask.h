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

#pragma once
#include "Sample.h"
#include "Slice.h"

#include <vector>
#include <map>
#include <ImfCompression.h>
#include <half.h>
#include <algorithm>

namespace openexrid
{

// The Mask object hold the data needed to dynamically craft the mask images.
// The Mask is built using a Builder object. It can be loaded and saved in an EXR file.
class Mask
{
	friend class Query;
public:

	// Build an empty Mask
	Mask ();

	// Build a mask using the Builder.
	Mask (const class Builder &builder, const std::vector<std::string> &names);

	// Read a Mask from an EXR file.
	// This method throws exceptions in case of reading issues.
	void read (const char *filename);

	// Write the mask in an EXR file.
	void write (const char *filename, Imf::Compression compression=Imf::ZIPS_COMPRESSION) const;

	// Returns the image size
	// This method is thread safe
	inline std::pair<int,int>	getSize () const;

	// Returns the number of sample in the pixel
	// This method is thread safe
	inline int getSampleN (int x, int y) const;

	// Returns the pixel n-th sample
	// This method is thread safe
	// x and y and samples must be in the valid range
	inline void getSample (int x, int y, int sample, Sample &result) const;

	// Returns the sample name
	// This method is thread safe
	// x and y and samples must be in the valid range
	// The returned pointer is valid until the Mask content is changed or destroyed
	inline const char *getSampleName (int x, int y, int sample) const;

	// Returns the id limit, i-e the largest id + 1.
	// This method is thread safe
	inline uint32_t getIdN () const;

	// Returns the name using a sample id.
	// The id should be < than getIdN(). "" is returned if no name is found for this id.
	// The returned pointer is valid until the Mask content is changed or destroyed
	// This method is thread safe
	inline const char *getName (uint32_t id) const;

	// Returns the number of slice in this image
	// This method is thread safe
	inline int getSliceN () const;

	// Returns the name of the nth slice
	// This method is thread safe
	inline const std::string &getSlice (int slice) const;

	// Find a slice by name.
	// Returns -1 if the slice is not found.
	// This method is thread safe
	inline int findSlice (const char *name) const;
private:

	// The image resolution
	int _Width, _Height;

	// For each name, the index of the begining of the string in the _Names buffer.
	std::vector<uint32_t>	_NamesIndexes;

	// All the names concatenated in a large string. The strings are C strings, 
	// with an ending \0 character.
	std::string				_Names;

	// For each pixels, the index of the first pixel sample in the _Samples vector.
	// The number of sample in the pixel p is (_PixelsIndexes[p+1]-_PixelsIndexes[p]).
	// _PixelsIndexes size is _Width*_Height+1.
	std::vector<uint32_t>	_PixelsIndexes;

	// The pixel id concatenated in a single vector.
	std::vector<uint32_t>	_Ids;

	// The slices available in this mask
	std::vector<std::string>	_Slices;

	// The pixel samples concatenated in a single vector.
	std::vector<std::vector<half> >	_SlicesData;

	// Mask version
	static const uint32_t	_Version = 1;
};

inline std::pair<int,int> Mask::getSize () const 
{
	return std::pair<int,int> (_Width, _Height);
}

inline int Mask::getSampleN (int x, int y) const
{
	const int offset = x+y*_Width;
	return _PixelsIndexes[offset+1]-_PixelsIndexes[offset];
}

inline void Mask::getSample (int x, int y, int sample, Sample &result) const
{
	const int index = _PixelsIndexes[x+y*_Width]+sample;
	result.Id = _Ids[index];
	result.Values.clear ();
	for (size_t s = 0; s < _Slices.size (); ++s)
		result.Values.push_back (_SlicesData[s][index]);
}

inline const char *Mask::getSampleName (int x, int y, int sample) const
{
	return getName (_Ids[_PixelsIndexes[x+y*_Width]+sample]);
}

inline uint32_t Mask::getIdN () const
{
	return (uint32_t)_NamesIndexes.size ();
}

inline const char *Mask::getName (uint32_t id) const
{
	if (id < getIdN ())
		return &_Names[_NamesIndexes[id]];
	else
		return "";
}

inline int Mask::getSliceN () const
{
	return (int)_Slices.size ();
}

inline const std::string &Mask::getSlice (int slice) const
{
	return _Slices[slice];
}

inline int Mask::findSlice (const char *name) const
{
	std::vector<std::string>::const_iterator ite = std::find (_Slices.begin (), _Slices.end (), name);
	return ite == _Slices.end () ? -1 : (int)(ite-_Slices.begin ());
}

}
