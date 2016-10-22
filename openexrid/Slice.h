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

#include <map>
#include <ImfPixelType.h>
#include "Mask.h"

namespace openexrid
{

/* The SampleList is a single array per pixel.
 * Each object present in the pixel as a single entry in this list.
 *
 * The entry structure is :
 * Header
 * float[valueN] - accumulated values
 */
class SampleList
{
public:
	struct Header
	{
		uint32_t	Id;			// Object Id
		float			Z;			// Accumulated Z
		uint32_t	Count;	// Number of accumulated samples
	};
	

	static const int HeaderSize = 3;	// Unaligned header size in uint32_t

	inline int getSampleN (int valueN) const
	{
		return (int)_Data.size ()/(valueN+HeaderSize);
	}
	Header &getSampleHeader (int sample, int valueN) const
	{
		return *(Header*)&_Data[sample*(valueN+HeaderSize)];
	}
	const float *getSampleValues (int sample, int valueN) const
	{
		return (const float*)&_Data[sample*(valueN+HeaderSize)+HeaderSize];
	}
	float *getSampleValues (int sample, int valueN)
	{
		return (float*)&_Data[sample*(valueN+HeaderSize)+HeaderSize];
	}
	void addCoverage (uint32_t id, float z, const float *sliceValues, int valueN)
	{
		// Check non zero
		bool allZero = true;
		for (int v = 0; v < valueN; ++v)
			allZero &= sliceValues[v] == 0.f;
		if (allZero) return;

		const int sn = getSampleN (valueN);
		for (int s = 0; s < sn; ++s)
		{
			Header &header = getSampleHeader (s, valueN);
			if (header.Id == id)
			{
				header.Z += z;
				++header.Count;
				float *values = getSampleValues (s, valueN);
				for (int v = 0; v < valueN; ++v)
					values[v] += sliceValues[v];
				return;
			}
		}

		// Not found, add an entry for this id
		const size_t index = _Data.size ();
		_Data.resize (index+valueN+HeaderSize, 0);

		// The id
		Header &header = *(Header*)&_Data[index];
		header.Id = id;
		header.Z = z;
		header.Count = 1;

		// The values
		for (int v = 0; v < valueN; ++v)
			*(float*)&_Data[index+v+HeaderSize] = sliceValues[v];
	}
private:
	std::vector<uint32_t>	_Data;
};

}
