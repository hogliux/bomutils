/*
  dumpbom.cpp - dump internal variables and blocks of bom files (for debugging)

  Copyright (C) 2013 Fabian Renn - bomutils (at) gmail.com

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
*/
#include <iostream>
#include <cstring>
#include <fstream>
#include <string>
#include <iomanip>

#if defined(WINDOWS)
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#include "bom.h"

using namespace std;

void print_tree( BOMTree * tree, char * buffer, BOMBlockTable * block_table ) {
  tree->version = ntohl( tree->version );
  tree->child = ntohl( tree->child );
  tree->blockSize = ntohl( tree->blockSize );
  tree->pathCount = ntohl( tree->pathCount );
  string type( tree->tree, 4 );
  cout << "tree->tree = " << type << endl;
  cout << "tree->version = " << tree->version << endl;
  cout << "tree->child = " << tree->child << endl;
  cout << "tree->blockSize = " << tree->blockSize << endl;
  cout << "tree->pathCount = " << tree->pathCount << endl;
  cout << "tree->unknown3 = " << (int)tree->unknown3 << endl;
  BOMPointer & child_ptr = block_table->blockPointers[tree->child];
  BOMPaths * paths = (BOMPaths*)&buffer[child_ptr.address];
  paths->isLeaf = ntohs( paths->isLeaf );
  paths->count = ntohs( paths->count );
  paths->forward = ntohs( paths->forward );
  paths->backward = ntohs( paths->backward );
  cout << "paths->isLeaf = " << paths->isLeaf << endl;
  cout << "paths->count = " << paths->count << endl;
  cout << "paths->forward = " << paths->forward << endl;
  cout << "paths->backward = " << paths->backward << endl;
}

int main ( int argc, char * argv[] ) {
  cerr << "bomutils - 5 days of work from me. 1 minute and $2 from you? :-) Donate at http://bomutils.github.io/" << endl << endl;

  if ( argc != 2 ) {
    cerr << "Usage: dumpbom bomfile" << endl;
    return 1;
  }
  char * buffer;
  streampos file_length;
  {
    ifstream bom_file( argv[1], ios::binary | ios::in );

    bom_file.seekg(0, ios::end);
    file_length = bom_file.tellg();
    bom_file.seekg(0);

    buffer = new char[file_length];
    bom_file.read( buffer, file_length );

    if ( bom_file.fail() ) {
      cerr << "Unable to read bomfile" << endl;
      return 1;
    }

    bom_file.close();
  }

  cout << argv[1] << endl;
  cout << "file_length = " << file_length << endl;

  cout << "Header:" << endl;
  cout << "-----------------------------------------------------" << endl;
  BOMHeader * header = (BOMHeader*)buffer;
  header->version = ntohl( header->version );
  header->numberOfBlocks = ntohl( header->numberOfBlocks );
  header->indexOffset = ntohl( header->indexOffset );
  header->indexLength = ntohl( header->indexLength );
  header->varsOffset = ntohl( header->varsOffset );
  header->varsLength = ntohl( header->varsLength );

  BOMBlockTable * block_table = (BOMBlockTable*)&buffer[header->indexOffset];
  block_table->numberOfBlockTablePointers = ntohl( block_table->numberOfBlockTablePointers );
  int numberOfNonNullEntries = 0;
  for ( unsigned int i=0; i < block_table->numberOfBlockTablePointers; ++i ) {
    if ( block_table->blockPointers[i].address != 0 ) {
      numberOfNonNullEntries++;
      block_table->blockPointers[i].address = ntohl( block_table->blockPointers[i].address );
      block_table->blockPointers[i].length = ntohl( block_table->blockPointers[i].length );
    }
  }
  {
    string magic( header->magic, 8 );
    cout << "magic = \"" << magic << "\"" << endl;
    cout << "version = " << header->version << endl;
    cout << "numberOfBlocks = " << header->numberOfBlocks << endl;
    cout << "indexOffset = " << header->indexOffset << endl;
    cout << "indexLength = " << header->indexLength << endl;
    cout << "varsOffset = " << header->varsOffset << endl;
    cout << "varsLength = " << header->varsLength << endl;
    cout << "(calculated number of blocks = " << numberOfNonNullEntries << ")" << endl;
  }

  cout << endl << "Index Table:" << endl;
  cout << "-----------------------------------------------------" << endl;

  cout << "numberOfBlockTableEntries = " << block_table->numberOfBlockTablePointers << endl;
#if 0
  for ( unsigned int i=0; i < block_table->numberOfBlockTablePointers; ++i ) {
    if ( block_table->blockPointers[i].address != 0 ) {
      cout << "{" << endl;
      cout << "\tid = " << i << endl;
      cout << "\taddress = " << setbase(16) << "0x" << block_table->blockPointers[i].address << setbase(10) << endl;
      cout << "\tlength = " << block_table->blockPointers[i].length << endl;
      cout << "}," << endl;
    }
  }
#endif

  uint32_t free_list_pos = header->indexOffset+sizeof(uint32_t)+(block_table->numberOfBlockTablePointers*sizeof(BOMPointer));
  BOMFreeList * free_list = (BOMFreeList*)&buffer[free_list_pos];
  free_list->numberOfFreeListPointers = ntohl( free_list->numberOfFreeListPointers );
  cout << endl << "Free List:" << endl;
  cout << "-----------------------------------------------------" << endl;
  cout << "numberOfFreeListPointers = " << free_list->numberOfFreeListPointers << endl;
  for ( unsigned int i=0; i < free_list->numberOfFreeListPointers ; ++i ) {
    free_list->freelistPointers[i].address = ntohl( free_list->freelistPointers[i].address );
    free_list->freelistPointers[i].length = ntohl( free_list->freelistPointers[i].length );
  }
#if 0
  for ( unsigned int i=0; i < free_list->numberOfFreeListPointers; ++i ) {
    cout << "{" << endl;
    cout << "\tid = " << i << endl;
    cout << "\taddress = " << setbase(16) << "0x" << free_list->freelistPointers[i].address << setbase(10) << endl;
    cout << "\tlength = " << free_list->freelistPointers[i].length << endl;
    cout << "}," << endl;
  }
#endif

  cout << endl << "Variables:" << endl;
  cout << "-----------------------------------------------------" << endl;

  int var_count;
  {
    BOMVars * vars = (BOMVars*)&buffer[header->varsOffset];
    unsigned int total_length = 0;
    vars->count = ntohl( vars->count );
    total_length += sizeof(uint32_t);
    BOMVar * var = &vars->first[0];
    var_count = vars->count;
    for ( int i=0; i<var_count; ++i ) {
      var->index = ntohl( var->index );
      total_length += sizeof(uint32_t);
      total_length += var->length + 1;
      var = (BOMVar*)&buffer[header->varsOffset+total_length];
    }
    
    cout << "vars->count = " << vars->count << endl;
    cout << "( calculated length = " << total_length << ")" << endl;
    var = &vars->first[0];
    total_length = sizeof(uint32_t);
    for ( int i=0; i<var_count; ++i ) {
      if ( i != 0 ) {
	cout << ",";
      }
      total_length += sizeof(uint32_t);
      total_length += var->length + 1;
      string name( var->name, var->length );
      cout << "\"" << name << "\"";
      var = (BOMVar*)&buffer[header->varsOffset+total_length];
    }
    cout << endl;
  }

  unsigned int total_length = sizeof(uint32_t);
  for ( int i=0; i<var_count; ++i ) {
    BOMVar * var = (BOMVar*)&buffer[header->varsOffset+total_length];
    total_length += sizeof(uint32_t) + 1 + var->length;
    string name( var->name, var->length );
    BOMPointer & ptr = block_table->blockPointers[var->index];
    cout << endl << "\"" << name << "\" (file offset: 0x" << setbase(16) << ptr.address << setbase(10) << " length: " << ptr.length << " )" << endl;
    cout << "-----------------------------------------------------" << endl;
    if ( ( name == "Paths" ) || ( name == "HLIndex" ) || ( name == "Size64" ) ) {
      BOMTree * tree = (BOMTree*)&buffer[ptr.address];
      print_tree( tree, buffer, block_table );
    } else if ( name == "BomInfo" ) {
      BOMInfo * info = (BOMInfo*)&buffer[ptr.address];
      info->version = ntohl( info->version );
      info->numberOfPaths = ntohl( info->numberOfPaths );
      info->numberOfInfoEntries = ntohl( info->numberOfInfoEntries );
      for ( unsigned int i=0; i<info->numberOfInfoEntries; ++i ) {
	info->entries[i].unknown0 = ntohl( info->entries[i].unknown0 );
	info->entries[i].unknown1 = ntohl( info->entries[i].unknown1 );
	info->entries[i].unknown2 = ntohl( info->entries[i].unknown2 );
	info->entries[i].unknown3 = ntohl( info->entries[i].unknown3 );
      }
      cout << "info->version = " << info->version << endl;
      cout << "info->numberOfPaths = " << info->numberOfPaths << endl;
      cout << "info->numberOfInfoEntries = " << info->numberOfInfoEntries << endl;
      for ( unsigned int i=0; i<info->numberOfInfoEntries; ++i ) {
	cout << "info->entries[i].unknown0 = " << info->entries[i].unknown0 << endl;
	cout << "info->entries[i].unknown1 = " << info->entries[i].unknown1 << endl;
	cout << "info->entries[i].unknown2 = " << info->entries[i].unknown2 << endl;
	cout << "info->entries[i].unknown3 = " << info->entries[i].unknown3 << endl;
      }
    } else if ( name == "VIndex" ) {
      BOMVIndex * vindex = (BOMVIndex*)&buffer[ptr.address];
      vindex->unknown0 = ntohl( vindex->unknown0 );
      vindex->indexToVTree = ntohl( vindex->indexToVTree );
      vindex->unknown2 = ntohl( vindex->unknown2 );
      vindex->unknown3 = vindex->unknown3;
      cout << "vindex->unknown0 = " << vindex->unknown0 << endl;
      cout << "vindex->indexToVTree = " << vindex->indexToVTree << endl;
      cout << "vindex->unknown2 = " << vindex->unknown2 << endl;
      cout << "vindex->unknown3 = " << (int)vindex->unknown3 << endl;
      BOMPointer & v_ptr = block_table->blockPointers[vindex->indexToVTree];
      BOMTree * tree = (BOMTree*)&buffer[v_ptr.address];
      print_tree( tree, buffer, block_table );
    } else {
      unsigned int i;
      uint32_t * raw = (uint32_t*)&buffer[ptr.address];
      for ( i=0; i<ptr.length/sizeof(uint32_t); ++i ) {
	cout << "0x" << setbase(16) << setw(8) << setfill('0') << ntohl(raw[i]) << setbase(10) << endl;
      }
      i*=sizeof(uint32_t);
      for ( ; i<ptr.length; ++i ) {
	cout << "0x" << setbase(16) << setw(2) << setfill('0') << (int)buffer[ptr.address+i] << endl;
      }
    }
  }

  delete [] buffer;
  return 0;

}
