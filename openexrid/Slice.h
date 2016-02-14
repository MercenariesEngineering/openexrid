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

class SampleList
{
public:
	inline int getSampleN (int valueN) const
	{
		return (int)_Data.size ()/(valueN+1);
	}
	uint32_t getSampleId (int sample, int valueN) const
	{
		return _Data[sample*(valueN+1)];
	}
	const float *getSampleValues (int sample, int valueN) const
	{
		return (const float*)&_Data[sample*(valueN+1)+1];
	}
	float *getSampleValues (int sample, int valueN)
	{
		return (float*)&_Data[sample*(valueN+1)+1];
	}
	void addCoverage (uint32_t id, const float *sliceValues, int valueN)
	{
		// Check non zero
		bool allZero = true;
		for (int v = 0; v < valueN; ++v)
			allZero &= sliceValues[v] == 0.f;
		if (allZero) return;

		const int sn = getSampleN (valueN);
		for (int s = 0; s < sn; ++s)
		{
			if (getSampleId (s, valueN) == id)
			{
				float *values = getSampleValues (s, valueN);
				for (int v = 0; v < valueN; ++v)
					values[v] += sliceValues[v];
				return;
			}
		}

		// Not found, add an entry for this id
		const size_t index = _Data.size ();
		_Data.resize (index+valueN+1, 0);

		// The id
		_Data[index] = id;

		// The values
		for (int v = 0; v < valueN; ++v)
			*(float*)&_Data[index+v+1] = sliceValues[v];
	}
private:
	std::vector<uint32_t>	_Data;
};

}
