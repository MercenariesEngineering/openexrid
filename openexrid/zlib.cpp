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

#include <string>
#include <zlib.h>
#include <sstream>
#include <stdexcept>
#include <memory.h>

// ***************************************************************************

// Deflate using zlib
std::string deflate (const std::string& str)
{
	z_stream zs;
	memset(&zs, 0, sizeof(zs));

	if (deflateInit(&zs, Z_BEST_COMPRESSION) != Z_OK)
		throw (std::runtime_error("deflateInit failed while compressing."));

	zs.next_in = (Bytef*)str.data();
	zs.avail_in = (uInt)str.size();

	int ret;
	char outbuffer[32];
	std::string outstring;

	do 
	{
		zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
		zs.avail_out = sizeof(outbuffer);
		ret = deflate(&zs, Z_FINISH);
		if (outstring.size() < zs.total_out)
		{
			outstring.append(outbuffer,
			zs.total_out - outstring.size());
		}
	} 
	while (ret == Z_OK);

	deflateEnd(&zs);

	if (ret != Z_STREAM_END)
	{
		std::ostringstream oss;
		oss << "Exception during zlib compression: (" << ret << ") " << zs.msg;
		throw(std::runtime_error(oss.str()));
	}

	return outstring;
}

// ***************************************************************************

// Inflate using zlib
std::string inflate (const std::string& str)
{
	z_stream zs;
	memset(&zs, 0, sizeof(zs));

	if (inflateInit(&zs) != Z_OK)
		throw (std::runtime_error("inflateInit failed while decompressing."));

	zs.next_in = (Bytef*)str.data();
	zs.avail_in = (uInt)str.size();

	int ret;
	char outbuffer[32768];
	std::string outstring;

	do
	{
		zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
		zs.avail_out = sizeof(outbuffer);

		ret = inflate(&zs, 0);

		if (outstring.size() < zs.total_out)
		{
			outstring.append(outbuffer,
			zs.total_out - outstring.size());
		}

	}
	while (ret == Z_OK);

	inflateEnd(&zs);

	if (ret != Z_STREAM_END)
	{
		std::ostringstream oss;
		oss << "Exception during zlib decompression: (" << ret << ") " << zs.msg;
		throw (std::runtime_error(oss.str()));
	}

	return outstring;
}

// ***************************************************************************
