/*
  lsbom.cpp - list the contents of bom files

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
#include "bom.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>

// NOTE: Windows does not have several of these headers
#include <string.h>
#if defined(WINDOWS)
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include <stdlib.h>
#include <unistd.h> // For getopt
#include <ctype.h>

using namespace std;

// Pass -D to enable debug outputs
#define DEBUG(level, msg)                                               \
  if (level <= debug) {cout << "DEBUG(" << level << "): " << msg << endl;}

enum {
  LIST_FILES = 1 << 0,
  LIST_DIRS  = 1 << 1,
  LIST_LINKS = 1 << 2,
  LIST_BDEVS = 1 << 3,
  LIST_CDEVS = 1 << 4,
  LIST_ALL = 0x1f,
};


static int debug = 0;
static char *data;
static BOMBlockTable *indexHeader;

char *lookup(int i, uint32_t *length = 0) {
  BOMPointer *index = indexHeader->blockPointers + ntohl(i);
  if (length) *length = ntohl(index->length);
  uint32_t addr = ntohl(index->address);

  DEBUG(2, "@ index=0x" << hex << ntohl(i)
        << " addr=0x" << hex << setw(4) << setfill('0') << addr
        << " len=" << dec << ntohl(index->length));

  return data + addr;
}


void short_usage() {
  cout << "Usage: lsbom [-h] [-s] [-f] [-d] [-l] [-b] [-c] [-m] [-x]\n"
       << "\t"
#if 0
    "[--arch archVal] "
#endif
    "[-p parameters] bom ..." << endl;
}


void usage_error(const char *msg) {
  cout << msg << endl;
  short_usage();
  exit(1);
}


void usage() {
  short_usage();
  cout << "\n"
    "\t-h              print full usage\n"
    "\t-s              print pathnames only\n"
    "\t-f              list files\n"
    "\t-d              list directories\n"
    "\t-l              list symbolic links\n"
    "\t-b              list block devices\n"
    "\t-c              list character devices\n"
    "\t-m              print modified times\n"
    "\t-x              suppress modes for directories and symlinks\n"
#if 0
    "\t--arch archVal  print info for architecture archVal (\"ppc\", "
    "\"i386\", \"hppa\", \"sparc\", etc)\n"
#endif
    "\t-p parameters   print only some of the results.  EACH OPTION CAN "
    "ONLY BE USED ONCE\n"
    "\t\tParameters:\n"
    "\t\t\tf    file name\n"
    "\t\t\tF    file name with quotes (i.e. \"/usr/bin/lsbom\")\n"
    "\t\t\tm    file mode (permissions)\n"
    "\t\t\tM    symbolic file mode\n"
    "\t\t\tg    group id\n"
    "\t\t\tG    group name\n"
    "\t\t\tu    user id\n"
    "\t\t\tU    user name\n"
    "\t\t\tt    mod time\n"
    "\t\t\tT    formatted mod time\n"
    "\t\t\ts    file size\n"
    "\t\t\tS    formatted size\n"
    "\t\t\tc    32-bit checksum\n"
    "\t\t\t/    user id/group id\n"
    "\t\t\t?    user name/group name\n"
    "\t\t\tl    link name\n"
    "\t\t\tL    quoted link name\n"
    "\t\t\t0    device type\n"
    "\t\t\t1    device major\n"
    "\t\t\t2    device minor\n"
       << flush;
}


void error(const char *msg) {
  cerr << msg << endl;
  exit(1);
}


int main(int argc, char *argv[]) {
  bool suppressDirSimModes = false;
  bool suppressDevSize = false;
  bool pathsOnly = false;
  int listType = 0;
  char params[16] = "";

  while (true) {
    char c = getopt(argc, argv, "hsfdlbcmxp:D::");
    if (c == -1) {
      break;
    }

    switch (c) {
    case 'h':
      usage();
      exit(0);
    case 's':
      pathsOnly = true;
      break;
    case 'f':
      listType |= LIST_FILES;
      break;
    case 'd':
      listType |= LIST_DIRS;
      break;
    case 'l':
      listType |= LIST_LINKS;
      break;
    case 'b':
      listType |= LIST_BDEVS;
      break;
    case 'c':
      listType |= LIST_CDEVS;
      break;
    case 'm':
      strcat(params, "T");
      break;
    case 'x':
      suppressDirSimModes = true;
      break;
    case 'a':
      usage_error("--arch not supported");
      break;
    case 'p':
      if (15 < strlen(optarg)) {
        usage_error("Too many parameters");
      }
      strcpy(params, optarg);
      break;
    case 'D':
      if (optarg) {
        debug = atoi(optarg);
      }
      else {
        debug++;
      }
      break;
    case ':':
    case '?':
      short_usage();
      exit(1);
      break;
    }
  }

  if (optind == argc) {
    usage();
    exit(1);
  }

  if (listType == 0) {
    listType = LIST_ALL;
  }
  if (!params[0]) {
    strcpy(params, "fm/scl0");
    suppressDevSize = true;
  }

  for (int i = optind; i < argc; i++) {
    fstream f(argv[i], ios::in | ios::binary);

    // Get file length
    f.seekg(0, ios::end);
    streampos length = f.tellg();
    f.seekg(0);

    // Allocate space
    data = new char[length];

    // Read data
    f.read(data, length);

    if (f.fail()) {
      cerr << "Failed to read BOM file" << endl;
      return 1;
    }

    f.close();

    BOMHeader *header = (BOMHeader *)data;

    if (string(header->magic, 8) != "BOMStore") {
      cerr << "Not a BOM file: " << argv[i] << endl;
      return 1;
    }

    indexHeader = (BOMBlockTable *)(data + ntohl(header->indexOffset));

    // Process vars
    BOMVars *vars = (BOMVars *)(data + ntohl(header->varsOffset));
    char *ptr = (char *)vars->first;
    for (unsigned i = 0; i < ntohl(vars->count); i++) {
      BOMVar *var = (BOMVar *)ptr;

      uint32_t varDataLength;
      char *varData = lookup(var->index, &varDataLength);
      BOMTree *tree = (BOMTree *)varData;
      string name = string(var->name, var->length);

      DEBUG(2, "BOMVar 0x" << hex << ntohl(var->index) << ' ' << name << ':');

      if (name == "Paths") {
        BOMPaths *paths = (BOMPaths *)lookup(tree->child);

        typedef map<uint32_t, string> filenames_t;
        typedef map<uint32_t, uint32_t> parents_t;
        filenames_t filenames;
        parents_t parents;

        while (paths->isLeaf == htons(0)) {
          paths = (BOMPaths *)lookup(paths->indices[0].index0);
        }

        while (paths) {
          for (unsigned j = 0; j < ntohs(paths->count); j++) {
            uint32_t index0 = paths->indices[j].index0;
            uint32_t index1 = paths->indices[j].index1;

            BOMFile *file = (BOMFile *)lookup(index1);
            BOMPathInfo1 *info1 = (BOMPathInfo1 *)lookup(index0);
            uint32_t length2;
            BOMPathInfo2 *info2 = (BOMPathInfo2 *)lookup(info1->index, &length2);

            // Compute full name
            string filename = file->name;
            filenames[info1->id] = filename;
            if (file->parent) {
              parents[info1->id] = file->parent;
            }

            parents_t::iterator it = parents.find(info1->id);
            while (it != parents.end()) {
              filename = filenames[it->second] + "/" + filename;
              it = parents.find(it->second);
            }

            // Check type
            switch (info2->type) {
            case TYPE_FILE:
              if (!(LIST_FILES & listType)) {
                continue;
              }
              break;
            case TYPE_DIR:
              if (!(LIST_DIRS & listType)) {
                continue;
              }
              break;
            case TYPE_LINK:
              if (!(LIST_LINKS & listType)) {
                continue;
              }
              break;
            case TYPE_DEV: {
              uint16_t mode = ntohs(info2->mode);
              bool isBlock = mode & 0x4000;
              if (isBlock && !(LIST_BDEVS & listType)) {
                continue;
              }
              if (!isBlock && !(LIST_CDEVS & listType)) {
                continue;
              }
              break;
            }
            }
            if (pathsOnly) {
              cout << filename << '\n';
            } else {
              // Print requested parameters
              bool printed = true;
              for (unsigned j = 0; params[j]; j++) {
                if (j && printed) {
                  cout << '\t';
                }
                printed = true;

                switch (params[j]) {
                case 'f':
                  cout << filename;
                  continue;
                case 'F':
                  cout << '"' << filename << '"';
                  continue;
                case 'g':
                  cout << dec << ntohl(info2->group);
                  continue;
                case 'G':
                  error("Group name not yet supported");
                  break;
                case 'u':
                  cout << dec << ntohl(info2->user);
                  continue;
                case 'U':
                  error("User name not yet supported");
                  break;
                case '/':
                  cout << dec << ntohl(info2->user) << '/' << ntohl(info2->group);
                  continue;
                case '?':
                  error("User/group name not yet supported");
                  break;

                default:
                  if (!suppressDirSimModes || (info2->type != TYPE_DIR && info2->type != TYPE_LINK)) {
                    switch (params[j]) {
                    case 'm':
                      cout << oct << ntohs(info2->mode);
                      continue;
                    case 'M':
                      error("Symbolic mode not yet supported");
                      break;
                    }
                  }

                  if (info2->type == TYPE_FILE || info2->type == TYPE_LINK) {
                    switch (params[j]) {
                    case 't':
                      cout << dec << ntohl(info2->modtime);
                      continue;
                    case 'T':
                      error("Formated mod time not yet supported");
                      break;
                    case 'c':
                      cout << dec << ntohl(info2->checksum);
                      continue;
                    }
                  }

                  if (info2->type != TYPE_DIR && (!suppressDevSize || info2->type != TYPE_DEV)) {
                    switch (params[j]) {
                    case 's':
                      cout << dec << ntohl(info2->size);
                      continue;
                    case 'S':
                      error("Formated size not yet supported");
                      break;
                    }
                  }

                  if (info2->type == TYPE_LINK) {
                    switch (params[j]) {
                    case 'l':
                      cout << info2->linkName;
                      continue;
                    case 'L':
                      cout << '"' << info2->linkName << '"';
                      continue;
                    }
                  }

                  if (info2->type == TYPE_DEV) {
                    uint32_t devType = ntohl(info2->devType);

                    switch (params[j]) {
                    case '0':
                      cout << dec << devType;
                      continue;
                    case '1':
                      cout << dec << (devType >> 24);
                      continue;
                    case '2':
                      cout << dec << (devType & 0xff);
                      continue;
                    }
                  }
                }

                printed = false;
              }
            }
            cout << '\n';

            DEBUG(1, "id=0x" << hex << ntohl(info1->id) << ' '
                  << "parent=0x" << ntohl(file->parent) << ' '
                  << "type=" << dec << (unsigned)info2->type << ' '
                  << "unknown0=" << dec << (unsigned)info2->unknown0 << ' '
                  << "architecture=0x" << hex << ntohs(info2->architecture)
                  << ' '
                  << "unknown1=" << dec << (unsigned)info2->unknown1 << ' '
                  << "length2=" << dec << length2);

            if (3 < debug) {
              for (unsigned k = 0; k < length2; k++) {
                if (k) {
                  if (k % 16 == 0 || k == length2 - 1) {
                    unsigned len = k % 16;
                    if (!len) {
                      len = 16;
                    }

                    if (len < 16) {
                      for (unsigned l = 0; l < 16 - len; l++) {
                        cout << "     ";
                      }
                      cout << ' ';
                    }

                    for (unsigned l = k - len; l < k; l++) {
                      if (l % 8 == 0) {
                        cout << ' ';
                      }

                      unsigned char c = ((unsigned char *)info2)[l];
                      if (isprint(c)) {
                        cout << (char)c;
                      } else {
                        cout << '.';
                      }
                    }

                    cout << '\n';
                  } else if (k % 8 == 0) {
                    cout << ' ';
                  }
                }

                cout << "0x" << setfill('0') << setw(2) << hex << (unsigned)((unsigned char *)info2)[k] << ' ';
              }
            }
          }

          if (paths->forward == htonl(0)) {
            paths = 0;
          } else {
            paths = (BOMPaths *)lookup(paths->forward);
          }
        }
      }

      ptr += sizeof(BOMVar) + var->length;
    }
  }

  cout << flush;

  return 0;
}
