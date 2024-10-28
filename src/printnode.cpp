/*
  printnode.cpp - generate file/directory list

  Copyright (C) 2013 Fabian Renn - fabian.renn (at) gmail.com

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA  02110-1301 USA.

  Initial work done by Joseph Coffland and Julian Devlin.
  Numerous further improvements by Baron Roberts.
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <sstream>
#include <iomanip>
#include <stdexcept>

#include "printnode.hpp"
#include "crc32.hpp"

using namespace std;

/* on unix system_path = path; on windows system_path is the windows native path format of path */
void print_node( ostream & output, const string & base, const string & system_path, const string & path,
        uint32_t uid, uint32_t gid, bool includeHidden) {
  struct stat s;
  string fullpath( base );
#if defined(WINDOWS)
  if ( system_path.size() != 0 ) {
    fullpath += string( "\\" ) + system_path;
  }
  int stat_ret = stat( fullpath.c_str(), &s );
#else
  fullpath += string( "/" ) + system_path;
  int stat_ret = lstat( fullpath.c_str(), &s );
#endif
  if ( stat_ret != 0 ) {
    cerr << "Unable to find path: " << fullpath << endl;
    exit(1);
  }
  output << path << "\t" << setbase(8) << s.st_mode << "\t" << setbase(10);
  output << (uid == UINT_MAX ? s.st_uid : uid) << "/" << (gid == UINT_MAX ? s.st_gid : gid);
  output << "\t" << s.st_mtime;
  if ( S_ISREG(s.st_mode) ) {
    output << "\t" << s.st_size << "\t" << calc_crc32( fullpath.c_str() );
  }
#if !defined(WINDOWS)
  if ( S_ISLNK( s.st_mode) ) {
    char buffer[PATH_MAX + 1];
    ssize_t num_bytes = readlink(fullpath.c_str(), buffer, PATH_MAX);
    if ( num_bytes < 0 ) {
      cerr << "Unable to read symbolic link: " << fullpath.c_str() << endl;
      exit(1);
    }
    buffer[num_bytes] = '\0';
    output << "\t" << s.st_size << "\t" << calc_str_crc32(buffer) << "\t" << buffer;
  }
#endif
  output << endl;
  if ( S_ISDIR( s.st_mode ) ) {
    DIR * d = opendir( fullpath.c_str() );
    struct dirent * dir;
    while ( ( dir = readdir( d ) ) != NULL ) {
      string name = string( dir->d_name );
      if (   !includeHidden
           ? dir->d_name[0] != '.'
	   : (name.compare(".") != 0 && name.compare("..") != 0)
      ) {

        string new_path(path);
        new_path += string( "/" ) + name;
#if defined(WINDOWS)
        string new_system_path(system_path);
        new_system_path += string( "\\" ) + name;
#else
        string new_system_path( new_path );
#endif
        print_node( output, base, new_system_path, new_path, uid, gid, includeHidden );
      }
    }
    closedir( d );
  }
}

void print_node( ostream & output, string directory, uint32_t uid, uint32_t gid, bool includeHidden ) {
  if ( directory.size() < 1 ) {
    cerr << "Invalid path" << endl;
    exit(1);
  }
  if ( directory[directory.size()-1] == '/' ) {
    directory = directory.substr( 0, directory.size() - 1 );
  }
  struct stat s;
  if ( stat( directory.c_str(), &s ) != 0 ) {
    cerr << "Unable to find path: " << directory << endl;
    exit(1);
  }
  if ( S_ISDIR( s.st_mode ) == false ) {
    cout << endl << "Argument must be a directory" << endl;
    exit(1);
  }
  print_node( output, directory, "", ".", uid, gid, includeHidden );
}
