#include <vector>
#include <iostream>
#include <string>
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
			names.push_back (std::to_string (rand()));

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
		Builder builder (Width, Height, {"R","G","B"});

		// Fill the builder
		for (int y = 0; y < Height; ++y)
		for (int x = 0; x < Width; ++x)
		{
			const vector<int> &pixel = pixelToNames[x+y*Width];
			const float weight = 1.f/(float)pixel.size ();
			const float values[3] = {weight, weight*0.75f, weight*0.5f};
			for (size_t s = 0; s < pixel.size (); ++s)
				builder.addCoverage (x, y, pixel[s], values);
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
			Query query (&mask, [&names,i](const char *name){return names[i] == name;});

			for (int y=0; y<Height; ++y)
			for (int x=0; x<Width; ++x)
			{
				// Recompute the coverage from the original image
				const vector<int> &pixel = pixelToNames[x+y*Width];
				const float weight = 1.f/(float)pixel.size ();
				const float values[3] = {weight, weight*0.75f, weight*0.5f};
				float weightSum[3] = {0, 0, 0};
				for (size_t s = 0; s < pixel.size (); ++s)
				{
					weightSum[0] += names[pixel[s]] == names[i] ? (half)values[0] : (half)0;
					weightSum[1] += names[pixel[s]] == names[i] ? (half)values[1] : (half)0;
					weightSum[2] += names[pixel[s]] == names[i] ? (half)values[2] : (half)0;
				}

				// The coverage from the file
				std::vector<float> coverage;
				query.getSliceData (x, y, coverage);

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

