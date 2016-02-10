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

// *** A dirent.h API like for directory listing on Windows

#ifdef WIN32
#include <Windows.h>
#undef min
#undef max

struct dirent
{
	char	d_name[MAX_PATH + 1];
};

// ***************************************************************************

struct DIR
{
	bool First;
	HANDLE Handle;
	WIN32_FIND_DATA Data;
	dirent Dirent;
};

// ***************************************************************************

DIR *opendir (const char *filename);
dirent *readdir (DIR *dir);
void closedir (DIR *dir);

#else // WIN32

#include <sys/types.h>
#include <dirent.h>

#endif // WIN32
