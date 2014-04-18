/*
  crc32.cpp - Calculate CRC-32 checksums of files

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
#if defined(WINDOWS)
#include <windows.h>
#include <stdio.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#endif

#include <iostream>

#include "crc32.hpp"
#include "crc32_poly.hpp"

using namespace std;

/* 512k default buffer size */
#define BUFFER_SIZE 512 * 1024

uint32_t calc_crc32( const char * file_path ) {
#if defined(WINDOWS)
  OFSTRUCT ignore;
  HFILE f = OpenFile( file_path, &ignore, OF_READ );
  if ( f == HFILE_ERROR ) {
#else
  int f = open( file_path, O_RDONLY );
  if ( f < 0 ) {
#endif
    cerr << "Cannot open file \"" << file_path << "\". Unable to calculate crc!" << endl;
    exit(1);
  }

#if defined(WINDOWS)
  LARGE_INTEGER li;
  li.QuadPart = 0;
  li.LowPart = SetFilePointer( (HANDLE)f, li.LowPart, &li.HighPart, FILE_END );
  if ( ( li.LowPart == INVALID_SET_FILE_POINTER ) && ( GetLastError() != NO_ERROR ) ) {
    cerr << "IO seek error while calculating crc of file \"" << file_path << "\"!" << endl;
    exit(1);
  }
  int64_t file_length = li.QuadPart;
  li.QuadPart = 0;
  li.LowPart = SetFilePointer( (HANDLE)f, li.LowPart, &li.HighPart, FILE_BEGIN );
  if ( ( li.LowPart == INVALID_SET_FILE_POINTER ) && ( GetLastError() != NO_ERROR ) ) {
    cerr << "IO seek error while calculating crc of file \"" << file_path << "\"!" << endl;
    exit(1);
  }
#else
  off_t file_length = lseek( f, 0, SEEK_END );
  lseek( f, 0, SEEK_SET );
#endif

  unsigned int buffer_size = ( file_length < BUFFER_SIZE ) ? file_length : BUFFER_SIZE;
  uint8_t * buffer = new uint8_t[buffer_size];
  if ( buffer == NULL ) {
#if defined(WINDOWS)
    CloseHandle( (HANDLE) f );
#else
    close(f);
#endif
    cerr << "Not enough memory to calculate crc checksum" << endl;
    exit(1);
  }

  int64_t file_pos = 0;
  uint32_t crc = 0;
  while ( file_pos < file_length ) {
    int bytes = ( ( file_length - file_pos ) < BUFFER_SIZE ) ? ( file_length - file_pos ) : BUFFER_SIZE;
    off_t buf_pos = 0;
    while ( buf_pos < bytes ) {
#if defined(WINDOWS)
      DWORD read;
      if ( ReadFile( (HANDLE)f, &buffer[buf_pos], bytes-buf_pos, &read, NULL ) == false ) {
        cerr << "IO error while calculating checksum of file \"" << file_path << "\"!" << endl;
        CloseHandle( (HANDLE) f );
        exit(1);
      }
      buf_pos += read;
#else
      ssize_t r = read( f, (void*)&buffer[buf_pos], bytes-buf_pos );
      if ( r == 0 ) {
        cerr << "Unexpected EOF in calculating checksum" << endl;
        close(f);
        exit(1);
      } else if ( r < 0 ) {
        cerr << "IO error (" << r << ") while calculating checksum of file \"" << file_path << "\"!" << endl;
        close(f);
        exit(1);
      } else {
        buf_pos += r;
      }
#endif
    }
    for ( int i=0; i<bytes; ++i ) {
      crc = crc_table[buffer[i] ^ ( ( crc >> 24 ) & 0xFF )] ^ ( crc << 8 );
    }
    file_pos += bytes;
  }

  /* cksum.c also calculates the crc-32 on the length of the file */
  while ( file_length > 0 ) {
    crc = crc_table[( ((uint64_t)file_length) & 0xFF ) ^ ( ( crc >> 24 ) & 0xFF )] ^ ( crc << 8 );
    file_length >>= 8;
  }

  delete [] buffer;
#if defined(WINDOWS)
  CloseHandle( (HANDLE) f );
#else
  close(f);
#endif
  /* invert all bits */
  return crc ^ 0xffffffff;
}

uint32_t calc_str_crc32(const char * str) {
  size_t num_bytes = strlen(str);
  uint32_t crc = 0;
  for (size_t i = 0; i < num_bytes; ++i) {
    crc = crc_table[str[i] ^ ((crc >> 24) & 0xFF)] ^ (crc << 8);
  }

  /* cksum.c also calculates the crc-32 on the length of the file */
  while (num_bytes > 0) {
    crc = crc_table[(((uint64_t) num_bytes) & 0xFF) ^ ((crc >> 24) & 0xFF)] ^ (crc << 8);
    num_bytes >>= 8;
  }

  return crc ^ 0xffffffff;
}
