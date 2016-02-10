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

#ifdef WIN32

// *** A dirent.h API like for directory listing on Windows

#include "dir.h"
#include <string>

DIR *opendir (const char *filename)
{
	DIR *dir = new DIR;
	std::string path = filename;
	path += "/*";
	dir->Handle = FindFirstFile(path.c_str (), &(dir->Data));
	dir->First = true;
	if (dir->Handle == INVALID_HANDLE_VALUE)
	{
		delete dir;
		dir = NULL;
	}
	return dir;
}

// ***************************************************************************

dirent *readdir (DIR *dir)
{
	if (dir->First || FindNextFile (dir->Handle, &(dir->Data)))
	{
		dir->First = false;
		strncpy (dir->Dirent.d_name, dir->Data.cFileName, sizeof (dir->Dirent.d_name)-1);
		return &(dir->Dirent);
	}
	return NULL;
}

// ***************************************************************************

void closedir (DIR *dir)
{
	FindClose (dir->Handle);
	delete dir;
}

#endif // WIN32