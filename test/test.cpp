#include <vector>
#include <iostream>
#include <string>
#include <cstdio>
#include <stdexcept>
#include "../openexrid/Builder.h"
#include "../openexrid/Mask.h"
#include "../openexrid/Query.h"

using namespace std;
using namespace openexrid;

// Number max of samples per pixel during the test
const int SamplesMax = 4;

// Number of different names
const int NameN = 100;

// Image resolution
const int Width = 192;
const int Height = 108;

// Number of errors during the test
int Errors = 0;

// The temporary file
const char *filename = "temp.exr";

std::string to_string (int d)
{
	char buffer[256];
	std::sprintf (buffer, "%d", d);
	return std::string (buffer);
}

class TestMatch
{
public:
	std::vector<std::string>	&names;
	int				i;
	TestMatch (std::vector<std::string> &_names, int _i) : names (_names), i (_i)
	{}
	bool operator () (const char *name) const
	{
		return names[i] == name;
	}
};

int main(int argc, char **argv)
{
	try
	{
		// Generate some names
		vector<string> names;
		for (int i = 0; i < NameN; ++i)
			names.push_back (std::to_string (rand()));

		// The name id per pixels
		struct entry
		{
			int Id;
			float Z;
			float Values[3];
		};
		vector<vector<entry> >	pixelToNames (Width*Height);

		// Generate a random image
		for (vector<vector<entry> >::iterator itp = pixelToNames.begin (); itp != pixelToNames.end (); ++itp)
		{
			const int samplesN = rand ()%(SamplesMax+1);
			const float weight = 1.f/(float)samplesN;
			const int baseId = rand();
			for (int s = 0; s < samplesN; ++s)
				itp->push_back ({(baseId+s*10)%NameN, (float)rand(), {weight*(float)rand()/(float)RAND_MAX, weight*(float)rand()/(float)RAND_MAX, weight*(float)rand()/(float)RAND_MAX}});
		}

		cout << "Fill a mask id map" << endl;
		std::vector<std::string> slices;
		slices.push_back ("R");
		slices.push_back ("G");
		slices.push_back ("B");
		Builder builder (Width, Height, slices);

		// Fill the builder
		for (int y = 0; y < Height; ++y)
		for (int x = 0; x < Width; ++x)
		{
			const vector<entry> &pixel = pixelToNames[x+y*Width];
			for (size_t s = 0; s < pixel.size (); ++s)
				// Let's use the id as Z value
				builder.addCoverage (x, y, pixel[s].Id, pixel[s].Z, pixel[s].Values);
		}

		{
			// Concat the names
			std::string _names;
			for (vector<string>::const_iterator ite = names.begin(); ite != names.end(); ++ite)
			{
				_names += *ite;
				_names += '\0';
			}

			cout << "Finish the mask" << endl;
			builder.finish();

			cout << "Write the mask" << endl;
			builder.write (filename, _names.c_str (), (int)_names.size());
		}

		// Read a mask
		cout << "Read the mask" << endl;
		Mask mask;
		mask.read (filename);

		const int R = mask.findSlice ("R");
		const int G = mask.findSlice ("G");
		const int B = mask.findSlice ("B");

		// Check the mask
		cout << "Check the mask" << endl;
		// Build the mask of every name
		for (int i = 0; i < NameN; ++i)
		{
			// Create a query for this name
			Query query (&mask, TestMatch (names, i));

			for (int y=0; y<Height; ++y)
			for (int x=0; x<Width; ++x)
			{
				// Recompute the coverage from the original image
				const vector<entry> &pixel = pixelToNames[x+y*Width];
				const float weight = 1.f/(float)pixel.size ();
				float weightSum[3] = {0, 0, 0};
				for (size_t s = 0; s < pixel.size (); ++s)
					for (int v = 0; v < 3; ++v)
						if (pixel[s].Id == i)
							weightSum[v] += (float)(half)pixel[s].Values[v];

				// The coverage from the file
				std::vector<float> coverage;
				query.getSliceData (x, y, coverage);

				if ((half)weightSum[0] != (half)coverage[R])
					cout << weightSum[0] << " " << coverage[R] << " " << (weightSum[0]-coverage[R])/weightSum[0] << endl;
				Errors += int((half)weightSum[0] != (half)coverage[R]);
				Errors += int((half)weightSum[1] != (half)coverage[G]);
				Errors += int((half)weightSum[2] != (half)coverage[B]);
			}
		}
	}
	catch (runtime_error &e)
	{
		cout << e.what () << endl;
		remove (filename);
		return -1;
	}

	remove (filename);
	cout << Errors << " error(s)" << endl;

	return Errors;
}

