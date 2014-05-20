/*
  ls4mkbom.cpp - generate file/directory list to be used by mkbom's -i option

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
#include <iostream>
#include <climits>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include "printnode.hpp"

using namespace std;

void usage() {
  cout << "Usage: ls4mkbom [-u uid] [-g gid] path" << endl << endl;
  cout << "\t-u\tForce user ID to the specified value" << endl;
  cout << "\t-g\tForce group ID to the specified value" << endl;
}

int main( int argc, char * argv[] ) {
  uint32_t uid = UINT_MAX;
  uint32_t gid = UINT_MAX;

  while (true) {
    char c = getopt(argc, argv, "hu:g:");
    if (c == -1) {
      break;
    }

    switch (c) {
    case 'u':
      uid = atol(optarg);
      break;
    case 'g':
      gid = atol(optarg);
      break;
    case 'h':
      usage();
      return 0;
    case ':':
    case '?':
      usage();
      return 1;
    }
  }

  if (optind == argc) {
    usage();
    return 1;
  }

  print_node( cout, argv[optind], uid, gid );
  return 0;
}
