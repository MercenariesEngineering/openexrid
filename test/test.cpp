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

// The name id per pixels
struct entry
{
	int Id;
	float Z;
	float Values[4];
	float Weight;
	bool operator<(const entry &o) const
	{
		return Z < o.Z;
	}
};

int _main(int argc, char **argv)
{
	try
	{
		// Generate some names
		vector<string> names;
		for (int i = 0; i < NameN; ++i)
			names.push_back (::to_string (rand()));

		vector<vector<entry> >	pixelToNames (Width*Height);

		// Generate a random image
		for (vector<vector<entry> >::iterator itp = pixelToNames.begin (); itp != pixelToNames.end (); ++itp)
		{
			const size_t x = (itp - pixelToNames.begin ())%Width;
			const size_t y = (itp - pixelToNames.begin ())/Width;

			// Compute an empty rectangle around to test the dataWindow
			if (x == 0 || x == Width-1 || y == 0 || y == Height-1)
				continue;

			const int samplesN = rand ()%(SamplesMax+1);
			const float weight = 1.f/(float)samplesN;
			const int baseId = rand();
			for (int s = 0; s < samplesN; ++s)
			{
				entry	e;
				e.Id = (baseId+s*10)%NameN;
				e.Z = (float)((baseId+s*10)%NameN);
				e.Values[0] = (float)rand()/(float)RAND_MAX;
				e.Values[1] = (float)rand()/(float)RAND_MAX;
				e.Values[2] = (float)rand()/(float)RAND_MAX;
				e.Values[3] = (float)rand()/(float)RAND_MAX;
				e.Weight = weight;
				itp->push_back (e);
			}

			// Sort by Z
			sort (itp->begin(), itp->end ());
		}

		cout << "Fill a mask id map" << endl;
		std::vector<std::string> slices;
		slices.push_back ("R");
		slices.push_back ("G");
		slices.push_back ("B");
		slices.push_back ("A");
		Builder builder (Width, Height, slices);
		const int TestA = 3;

		// Fill the builder
		for (int y = 0; y < Height; ++y)
		for (int x = 0; x < Width; ++x)
		{
			const vector<entry> &pixel = pixelToNames[x+y*Width];
			for (size_t s = 0; s < pixel.size (); ++s)
				// Let's use the id as Z value
				builder.addCoverage (x, y, pixel[s].Id, pixel[s].Z, pixel[s].Weight, pixel[s].Values);
		}

		std::vector<float> pixelWeights (Width*Height, 0.f);
		{
			// Concat the names
			std::string _names;
			for (vector<string>::const_iterator ite = names.begin(); ite != names.end(); ++ite)
			{
				_names += *ite;
				_names += '\0';
			}

			// Build the final weight array
			for (int i =0; i < Width*Height; ++i)
			{
				const vector<entry> &pixel = pixelToNames[i];
				for (size_t s = 0; s < pixel.size (); ++s)
					pixelWeights[i] += pixel[s].Weight;
			}

			cout << "Finish the mask" << endl;
			builder.finish (pixelWeights);

			cout << "Write the mask" << endl;
			builder.write (filename, _names.c_str (), (int)_names.size(), true);
		}

		// Accumulate the pixels values
		for (int y=0; y<Height; ++y)
		for (int x=0; x<Width; ++x)
		{
			vector<entry> &pixel = pixelToNames[x+y*Width];
			if (!pixel.empty())
			{
				// First normalize the pixel using the weights
				const float weightSum = pixelWeights[x+y*Width];
				for (int v = 0; v < 4; ++v)
				{
					for (size_t s = 0; s < pixel.size (); ++s)
					{
						if (weightSum != 0)
							pixel[s].Values[v] = pixel[s].Values[v]*pixel[s].Weight/weightSum;
					}
				}

				// The accumulates the fragments in a deep fashion way
				for (int v = 0; v < 4; ++v)
				{
					float acc = 0.f;
					for (size_t s = 0; s < pixel.size (); ++s)
						pixel[s].Values[v] = (acc += pixel[s].Values[v]);
				}
				for (size_t s = pixel.size ()-1; s > 0; --s)
				{
					for (int v = 0; v < 4; ++v)
						pixel[s].Values[v] = (pixel[s].Values[v]-pixel[s-1].Values[v])/(1.f-pixel[s-1].Values[TestA]);
				}
			}
		}

		// Read a mask
		cout << "Read the mask" << endl;
		Mask mask;
		mask.read (filename);

		const int R = mask.findSlice ("R");
		const int G = mask.findSlice ("G");
		const int B = mask.findSlice ("B");
		const int A = mask.findSlice ("A");

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
				float weightSum[4] = {0, 0, 0, 0};

				// Fill the accumulated buffer
				float prevAlpha = 0.f;
				for (size_t s = 0; s < pixel.size (); ++s)
				{
					if (pixel[s].Id == i)
					{
						for (int v = 0; v < 4; ++v)
							weightSum[v] += (half)((1.f - prevAlpha)*(half)pixel[s].Values[v]);
					}
					prevAlpha += (1.f - prevAlpha)*(half)pixel[s].Values[TestA];
				}

				// The coverage from the file
				std::vector<float> coverage;
				query.getSliceData (x, y, coverage);

				Errors += int(weightSum[0] != coverage[R]);
				Errors += int(weightSum[1] != coverage[G]);
				Errors += int(weightSum[2] != coverage[B]);
				Errors += int(weightSum[3] != coverage[A]);
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

/// ********************************* prototype

const string characters ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-");

string generateName (int nameSize)
{
	string s;
	for (size_t i = 0; i < nameSize; ++i)
		s += characters[rand ()%characters.size()];
	return s;
}

void fill (vector<string> &pathes, size_t begin, size_t end, const string &parent, int nameSize)
{
	const size_t size = end-begin;
	string newPath = parent + "/" + generateName(nameSize);
	if (size == 0)
		return;
	if (size == 1)
		pathes[begin] = newPath;
	else
	{
		const size_t middle = begin+size/2;
		fill (pathes, begin, middle, newPath, nameSize);
		fill (pathes, middle, end, newPath, nameSize);
	}
}

vector<string> buildPathes (int objectCount, int nameSize)
{
	vector<string> pathes (objectCount);
	fill (pathes, 0, pathes.size (), "", nameSize);
	return pathes;
}

float randf ()
{
	return (float)rand()/(float)RAND_MAX;
}

void drawQuad (int x, int y, int size, int id, float z, Builder &builder, float coverage)
{
	const float rgba[] = {randf()/coverage,randf()/coverage,randf()/coverage,randf()/coverage};
	for (int _y = 0; _y < size; ++_y)
	for (int _x = 0; _x < size; ++_x)
		builder.addCoverage (x+_x, y+_y, id, z, 1.f, rgba);
}

vector<char> concat (const vector<string> &strings)
{
	vector<char> result;
	for (size_t i = 0; i < strings.size (); ++i)
	{
		const string &s = strings[i];
		for (size_t j = 0; j < s.size (); ++j)
			result.push_back (s[j]);
		result.push_back ('\0');
	}
	return result;
}

float averageSize (const vector<string> &pathes)
{
	float sum = 0;
	for (size_t i = 0; i < pathes.size (); ++i)
		sum += (float)pathes[i].size();
	return sum / (float)pathes.size();
}

size_t getFileSize (const char *file)
{
	struct stat _stat;
	stat (file, &_stat);
	return _stat.st_size;
}

void generate (int argc, char **argv)
{
	if (argc < 6)
	{
		cout << "USAGE : TEST generate width height nameSize objectCount coverage fileOut" << endl;
		return;
	}

	int arg = 2;
	const int width = atoi (argv[arg++]);
	const int height = atoi (argv[arg++]);
	const int nameSize = atoi (argv[arg++]);
	const int objectCount = atoi (argv[arg++]);
	const float coverage = (float)atof (argv[arg++]);
	const char *fileOut = argv[arg++];

	const float squareSize = sqrt ((float)(width*height*coverage)/(float)objectCount);
	const int squareMin = (int)squareSize;

	vector<string> pathes (buildPathes (objectCount, nameSize));

	vector<string> slices;
	slices.push_back ("R");
	slices.push_back ("G");
	slices.push_back ("B");
	slices.push_back ("A");
	Builder builder (width, height, slices);

	// Probability to take the upper size
	const float probabilityUp = (squareSize*squareSize - squareMin*squareMin) / ((squareMin+1)*(squareMin+1) - squareMin*squareMin);

	float realCoverage = 0;
	for (size_t i = 0; i < pathes.size(); ++i)
	{
//		cout << pathes[i] << endl;

		// Square position
		const int square = (randf() < probabilityUp) ? squareMin+1 : squareMin;
		const int x = rand()%(width-square);
		const int y = rand()%(height-square);
		realCoverage += square*square;
		drawQuad (x, y, square, (int)i, (float)i, builder, coverage);
	}
	realCoverage /= (float)(width*height);

	vector<float> weights (width*height, 1.f);
	builder.finish (weights);
	const vector<char> names = concat (pathes);
	builder.write (fileOut, names.data(), (int)names.size(), false, Imf::ZIPS_COMPRESSION);


	const string fileOutNoString = string(fileOut)+".nostring";
	const string fileOutNoId = string(fileOut)+".noid";
	builder.write (fileOutNoString.c_str(), names.data(), (int)names.size(), false, Imf::ZIPS_COMPRESSION, true, false);
	builder.write (fileOutNoId.c_str(), names.data(), (int)names.size(), false, Imf::ZIPS_COMPRESSION, false, true);
	
	extern std::string deflate (const char *str, int len);
	const string compressed = deflate (names.data(), (int)names.size());

	cout << "File size: " << getFileSize(fileOut) << endl;
	const size_t idSize = getFileSize(fileOut)-getFileSize(fileOutNoId.c_str());
	cout << "File id size: " << idSize << endl;
	const size_t stringSize = getFileSize(fileOut)-getFileSize(fileOutNoString.c_str());
	cout << "File strings size: " << stringSize << endl;
	cout << "string/Id cost: " << 100.f*(float)stringSize/(float)idSize << "%" << endl;
	cout << "Coverage: " << realCoverage << endl;
	cout << "Average path size: " << averageSize (pathes) << endl;
	cout << "String size: " << names.size() << endl;
	cout << "Compressed strings size: " << compressed.size() << " (" << 100.f*(float)compressed.size()/(float)names.size() << "%)" << endl;
	cout << "Compressed strings bytes per ids: " << (float)compressed.size()/(float)pathes.size() << endl;
	cout << "String cost: " << 100.f*(float)(getFileSize(fileOut)-getFileSize(fileOutNoString.c_str()))/getFileSize(fileOut) << "%" << endl;
	cout << "Id cost: " << 100.f*(float)idSize/(float)getFileSize(fileOut) << "%" << endl;
	cout << "string/Id cost: " << 100.f*(float)stringSize/(float)idSize << "%" << endl;
}

float computeCoverage (const Mask &mask)
{
	const int w = (int)mask.getSize ().first;
	const int h = (int)mask.getSize ().second;
	int sum = 0;
	for (int y = 0; y < h; ++y)
	for (int x = 0; x < w; ++x)
	{
		sum += mask.getSampleN (x, y);
	}
	return (float)sum/(float)(w*h);
}

void read (int argc, char **argv)
{
	if (argc < 3)
	{
		cout << "USAGE : TEST read fileIn" << endl;
		return;
	}

	int arg = 2;
	const char *fileIn = argv[arg++];

	Mask mask;
	mask.read (fileIn);

	vector<string> pathes;
	for (uint32_t i = 0; i < mask.getIdN(); ++i)
	{
		cout << mask.getName (i) << endl;
		pathes.push_back (mask.getName (i));
	}

	const vector<char> names = concat (pathes);
	extern std::string deflate (const char *str, int len);
	const string compressed = deflate (names.data(), (int)names.size());

	cout << "Width: " << mask.getSize ().first << endl;
	cout << "Height: " << mask.getSize ().second << endl;
	cout << "Coverage: " << computeCoverage(mask) << endl;
	cout << "Average path size: " << averageSize (pathes) << endl;
	cout << "String size: " << names.size() << endl;
	cout << "Compressed strings size: " << compressed.size() << " (" << 100.f*(float)compressed.size()/(float)names.size() << "%)" << endl;
	cout << "Compressed strings bytes per ids: " << (float)compressed.size()/(float)pathes.size() << endl;
}

int main (int argc, char **argv)
{
	if (argc > 1 && strcmp (argv[1], "generate") == 0)
		generate (argc, argv);
	else if (argc > 1 && strcmp (argv[1], "read") == 0)
		read (argc, argv);
	else
		cout << "USAGE : TEST [generate|read]" << endl;

	return 0;
}
