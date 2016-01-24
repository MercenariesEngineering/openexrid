#include <vector>
#include <iostream>
#include <string>
#include "../libopenidmask/Builder.h"
#include "../libopenidmask/Mask.h"
#include "../libopenidmask/Query.h"

using namespace std;
using namespace openidmask;

// Number max of samples per pixel during the test
const int SamplesMax = 4;

// Number of different names
const int NameN = 100;

// Image resolution
const int Width = 1920;
const int Height = 1080;

// Number of errors during the test
int Errors = 0;

// The temporary file
const char *filename = "temp.exr";

int main(int argc, char **argv)
{
	try
	{
		// Generate some names
		vector<string> names;
		for (int i = 0; i < NameN; ++i)
			names.push_back (to_string (rand()));

		// The name id per pixels
		vector<vector<int> >	pixelToNames (Width*Height);

		// Generate a random image
		for (auto &pixel : pixelToNames)
		{
			const int samplesN = rand ()%(SamplesMax+1);
			for (int s = 0; s < samplesN; ++s)
				pixel.push_back (rand()%NameN);
		}

		cout << "Fill a mask id map" << endl;
		Builder builder (Width, Height);

		// Fill the builder
		for (int y = 0; y < Height; ++y)
		for (int x = 0; x < Width; ++x)
		{
			const vector<int> &pixel = pixelToNames[x+y*Width];
			const float weight = 1.f/(float)pixel.size ();
			for (size_t s = 0; s < pixel.size (); ++s)
				builder.addCoverage (x, y, weight, pixel[s]);
		}

		{
			// Build and write a mask
			cout << "Build the mask id map" << endl;
			const Mask mask (builder, names);

			cout << "Write the mask" << endl;
			mask.write (filename);
		}

		// Read a mask
		cout << "Read the mask" << endl;
		Mask mask (filename);

		// Check the mask
		cout << "Check the mask" << endl;
		// Build the mask of every name
		for (int i = 0; i < NameN; ++i)
		{
			// Create a query for this name
			Query query (&mask, [&names,i](const char *name){return names[i] == name;});

			for (int y=0; y<Height; ++y)
			for (int x=0; x<Width; ++x)
			{
				// The coverage from the file
				const float coverage = query.getCoverage (x, y);

				// Recompute the coverage from the original image
				const vector<int> &pixel = pixelToNames[x+y*Width];
				const float weight = 1.f/(float)pixel.size ();
				float weightSum = 0;
				for (size_t s = 0; s < pixel.size (); ++s)
					weightSum += names[pixel[s]] == names[i] ? weight : 0;

				Errors += int(weightSum != coverage);
			}
		}
	}
	catch (exception &e)
	{
		cout << e.what () << endl;
		remove (filename);
		return -1;
	}

	remove (filename);
	cout << Errors << " error(s)" << endl;

	return Errors;
}

