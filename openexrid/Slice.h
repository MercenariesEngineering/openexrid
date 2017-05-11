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
		float			Weight;	// Accumulated Weight per fragment to average Z
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
	void addCoverage (uint32_t id, float z, float weight, const float *sliceValues, int valueN)
	{
		if (weight == 0.f) return;

		const int sn = getSampleN (valueN);
		for (int s = 0; s < sn; ++s)
		{
			Header &header = getSampleHeader (s, valueN);
			if (header.Id == id)
			{
				header.Z += z*weight;
				header.Weight += weight;
				float *values = getSampleValues (s, valueN);
				for (int v = 0; v < valueN; ++v)
					values[v] += sliceValues[v]*weight;
				return;
			}
		}

		// Not found, add an entry for this id
		const size_t index = _Data.size ();
		_Data.resize (index+valueN+HeaderSize, 0);

		// The id
		Header &header = *(Header*)&_Data[index];
		header.Id = id;
		header.Z = z*weight;
		header.Weight = weight;

		// The values
		for (int v = 0; v < valueN; ++v)
			*(float*)&_Data[index+v+HeaderSize] = sliceValues[v]*weight;
	}

	// Normalize the samples by the weight sum
	void normalize (float weightSum, int valueN)
	{
		const int sn = getSampleN (valueN);
		for (int s = 0; s < sn; ++s)
		{
			SampleList::Header &header = getSampleHeader(s,valueN);

			// Average the Z using the fragment weight sum, not the pixel weight sum
			if (header.Weight != 0)
				header.Z /= header.Weight;

			if (weightSum != 0.f)
			{
				float *values = getSampleValues (s,valueN);
				for (int v = 0; v < valueN; ++v)
					values[v] /= weightSum;
			}
		}
	}

private:
	std::vector<uint32_t>	_Data;
};

}
