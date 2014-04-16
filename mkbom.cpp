/*
  mkbom.cpp - create bill-of-materials file for Mac OS X installers

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
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include <iterator>
#include <algorithm>
#include <iomanip>
#include <stdlib.h>
#include <libgen.h>
#include <stdint.h>
#if defined(WINDOWS)
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include <cstring>

#include "bom.h"
#include "printnode.hpp"

using namespace std;

typedef enum {
  kNullNode,
  kFileNode,
  kDirectoryNode,
  kSymbolicLinkNode,
  kRootNode
} tNodeType;

struct Node {
  map<string, Node> children;
  tNodeType type;
  uint32_t mode;
  uint32_t uid;
  uint32_t gid;
  uint32_t size;
  uint32_t checksum;
  uint32_t linkNameLength;
  string linkName;
  Node()
  : type( kNullNode ) {
  }
};

class BOMStorage {
private:
  uint32_t size_of_header;
  BOMHeader * header;

  uint32_t size_of_vars;
  uint32_t num_vars;
  BOMVars * vars;

  uint32_t size_of_block_table;
  uint32_t num_block_entries;
  BOMBlockTable * block_table;

  uint32_t size_of_free_list;
  uint32_t num_free_list_entries;
  BOMFreeList * free_list;

  uint32_t entry_size;
  char * entries;
public:
  BOMStorage() {
    size_of_header = 512;
    header = (BOMHeader*)malloc( size_of_header );

    num_block_entries = 1;
    size_of_block_table = sizeof(uint32_t) + ( num_block_entries * sizeof(BOMPointer) );
    block_table = (BOMBlockTable *)malloc( size_of_block_table );

    size_of_free_list = sizeof(uint32_t) + ( 2 * sizeof(BOMPointer) );
    free_list = (BOMFreeList*)malloc( size_of_free_list );
    num_free_list_entries = 0;

    num_vars = 0;
    size_of_vars = sizeof(uint32_t);
    vars = (BOMVars*)malloc( size_of_vars );

    entry_size = 0;
    entries = NULL;

    memset( header, 0, size_of_header );
    memcpy( header->magic, "BOMStore", 8 );
    header->version = htonl( 1 );
    header->numberOfBlocks = htonl( 0 );
    header->indexOffset = htonl( size_of_header + size_of_vars + entry_size );
    header->indexLength = htonl( size_of_block_table + size_of_free_list );
    header->varsOffset = htonl( size_of_header );
    header->varsLength = htonl( size_of_vars );

    block_table->numberOfBlockTablePointers = htonl( num_block_entries );
    block_table->blockPointers[0].address = htonl( 0 );
    block_table->blockPointers[0].length = htonl( 0 );

    vars->count = htonl( 0 );

    free_list->numberOfFreeListPointers = htonl( num_free_list_entries );
    for ( unsigned int i=0; i<2; ++i ) {
      free_list->freelistPointers[i].address = htonl(0);
      free_list->freelistPointers[i].length = htonl(0);
    }

  }

  int addBlock( const void * data, uint32_t length ) {
    if ( entries == NULL ) {
      entries = (char*)malloc( length );
    } else {
      entries = (char*)realloc( entries, length + entry_size );
    }
    memcpy( &entries[entry_size], data, length );
    
    size_of_block_table = sizeof(uint32_t) + ( ( num_block_entries + 1 ) * sizeof(BOMPointer) );
    block_table = (BOMBlockTable *)realloc( block_table, size_of_block_table );
    block_table->blockPointers[num_block_entries].address = entry_size; /* this will be converted to the right value later on */
    block_table->blockPointers[num_block_entries].length = htonl( length );
    num_block_entries++;
    entry_size += length;
    block_table->numberOfBlockTablePointers = htonl( num_block_entries );

    /* update header */
    header->numberOfBlocks = htonl( ntohl( header->numberOfBlocks ) + 1  );
    header->indexLength = htonl( size_of_block_table + size_of_free_list );
    return num_block_entries-1;
  }

  void addVar( const char * name, const void * data, uint32_t length ) {
    unsigned int new_size = sizeof(uint32_t) + 1 + strlen(name);

    vars = (BOMVars*)realloc( vars, size_of_vars + new_size );
    BOMVar * var = (BOMVar*)&(((char*)vars)[size_of_vars]);
    size_of_vars += new_size;
    var->index = htonl( addBlock( data, length ) );
    var->length = strlen(name);
    memcpy( var->name, name, strlen(name) );
    vars->count = htonl( ntohl( vars->count ) + 1 );
    /* update header */
    header->indexOffset = htonl( size_of_header + size_of_vars + entry_size );
    header->varsLength = htonl( size_of_vars );
  }

  void write( ofstream & bom_file ) {
    bom_file.write( (char*)header, size_of_header );
    bom_file.write( (char*)vars, size_of_vars );
    if ( entries != NULL ) {
      bom_file.write( (char*)entries, entry_size );
    }
    BOMBlockTable * temp = (BOMBlockTable *)malloc( size_of_block_table );
    memcpy( temp, block_table, size_of_block_table );
    for ( unsigned int i=0; i<ntohl(temp->numberOfBlockTablePointers); ++i ) {
      if ( temp->blockPointers[i].length != 0 ) {
	temp->blockPointers[i].address = htonl( temp->blockPointers[i].address + size_of_header + size_of_vars );
      }
    }
    bom_file.write( (char*)temp, size_of_block_table );
    free( ( void*) temp );
    bom_file.write( (char*)free_list, size_of_free_list );
  }

  ~BOMStorage() {
    if ( entries != NULL ) {
      free( (void*) entries );
    }
    free( (void*) vars );
    free( (void*) free_list );
    free( (void*) block_table );
    free( (void*) header );
  }
};

uint32_t dec_octal_to_int( uint32_t dec_rep_octal ) {
  uint32_t retval = 0;
  for ( unsigned int n = 1; dec_rep_octal; n*=8 ) {
    unsigned int digit = dec_rep_octal - ( ( dec_rep_octal / 10 ) * 10 );
    if ( digit > 7 ) {
      throw runtime_error( "argument not in dec oct rep" );
    }
    retval += digit * n;
    dec_rep_octal /= 10;
  }
  return retval;
}

void write_bom( istream & lsbom_file, const string & output_path ) {
  Node root;
  unsigned int num;
  root.type = kRootNode;
  {
    map<string, Node> all_nodes;
    string line;
    while( getline( lsbom_file, line ) ) {
      Node n;
      string name;
      vector<string> elements;
      {
	stringstream ss( line );
	getline( ss, name, '\t' );
	if ( ss.good() == false ) {
	  cerr << endl << "Syntax error in lsbom input" << endl;
	  exit(1);
	}
	{
	  string rest;
	  getline( ss, rest );
    std::size_t it = rest.find("/");
    if (it != string::npos) {
	    rest[it] = ' ';
	  }
	  stringstream item_stream( rest );
	  copy( istream_iterator<string>( item_stream ), istream_iterator<string>(), back_inserter( elements ) );
	}
      }
      n.mode = dec_octal_to_int( atol( elements[0].c_str() ) );
      n.uid = atol( elements[1].c_str() );
      n.gid = atol( elements[2].c_str() );
      n.size = 0;
      n.checksum = 0;
      n.linkNameLength = 0;
      if ( ( n.mode & 0xF000 ) == 0x4000 ) {
        n.type = kDirectoryNode;
      } else if ( ( n.mode & 0xF000 ) == 0x8000 ) {
        n.type = kFileNode;
        n.size = atol(elements[3].c_str());
        n.checksum = atol(elements[4].c_str());
      } else if ((n.mode & 0xF000) == 0xA000) {
        n.type = kSymbolicLinkNode;
        n.size = atol(elements[3].c_str());
        n.checksum = atol(elements[4].c_str());
        n.linkNameLength = elements[5].size() + 1;
        n.linkName = elements[5];
      } else {
        cerr << endl << "Node type not supported" << endl;
        exit(1);
      }
      all_nodes[name] = n;
    }
    /* create tree */
    for ( map<string,Node>::const_iterator it=all_nodes.begin(); it != all_nodes.end(); ++it ) {
      vector<string> path_elements;
      {
	stringstream ss(it->first);
	for ( string element; getline( ss, element, '/' ); path_elements.push_back( element ) );
      }
      Node * parent = &root;
      std::string full_path;
      for ( vector<string>::const_iterator jt = path_elements.begin(); jt != path_elements.end(); ++jt ) {
	full_path += *jt;
	/* search for this path element */
	map<string,Node>::iterator kt;
	if ( ( kt = parent->children.find( *jt ) ) == parent->children.end() ) {
	  /* this node was not found in the parent, we must create it */
	  map<string,Node>::const_iterator lt;
	  if ( ( lt = all_nodes.find( full_path ) ) == all_nodes.end() ) {
	    cerr << endl << "Parent directory of file/folder \"" << full_path << "\" does not appear in list" << endl;
	    exit(1);
	  }
	  parent->children[*jt] = lt->second;
	  kt = parent->children.find( *jt );
	}	  
	parent = &kt->second;
	full_path += "/";
      }
    }
    num = all_nodes.size();
  }


  BOMStorage bom;
  {
    unsigned int bom_info_size = ( sizeof(uint32_t)*3 ) + ( ( ( num != 0 ) ? 1 : 0 ) * sizeof( BOMInfoEntry ) );
    BOMInfo * info = (BOMInfo*)malloc( bom_info_size );
    memset( info, 0, bom_info_size );
    info->version = htonl( 1 );
    info->numberOfPaths = htonl( num + 1 );
    info->numberOfInfoEntries = htonl( ( num != 0 ) ? 1 : 0 );
    if ( num != 0 ) {
      // info->entries[0].unknown2 = htonl( 57826303 ); /* ???? */
      info->entries[0].unknown2 = htonl( 0 ); /* ???? */
    }
    bom.addVar( "BomInfo", info, bom_info_size );
    free( info );
  }

  {
    BOMTree tree;
    memcpy( tree.tree, "tree", 4 );
    tree.version = htonl(1);
    tree.blockSize = htonl( 4096 );
    tree.pathCount = htonl( num );
    tree.unknown3 = 0; /* ?? */

    unsigned int path_size = ( sizeof( uint16_t ) * 2 ) + ( sizeof( uint32_t ) * 2 ) + ( num * sizeof( BOMPathIndices ) );
    BOMPaths * paths = (BOMPaths*)malloc( path_size );
    paths->isLeaf = htons(1);
    paths->count = htons(num);
    paths->forward = 0;
    paths->backward = 0;

    vector<std::pair<uint32_t,const Node*> > stack;
    
    stack.push_back( std::pair<uint32_t,const Node*>( 0, &root ) );
    unsigned int j = 0;
    while ( stack.size() != 0 ) {
      const Node & arg = *stack[0].second;
      uint32_t parent = stack[0].first;
      stack.erase( stack.begin() );
      for ( map<string,Node>::const_iterator it=arg.children.begin(); it != arg.children.end(); ++it ) {
        const Node & node = it->second;
        string s = it->first;
        
        unsigned int bom_path_info2_size = sizeof(BOMPathInfo2) + node.linkNameLength;
        BOMPathInfo2 * info2 = (BOMPathInfo2*) malloc(bom_path_info2_size);
        if (node.type == kDirectoryNode) {
          info2->type = TYPE_DIR;
        } else if (node.type == kFileNode) {
          info2->type = TYPE_FILE;
        } else {
          info2->type = TYPE_LINK;
        }
        info2->unknown0 = 1;
        info2->architecture = htons(3); /* ?? */
        info2->mode = htons(node.mode);
        info2->user = htonl(node.uid);
        info2->group = htonl(node.gid);
        info2->modtime = 0;
        info2->size = htonl(node.size);
        info2->unknown1 = 1;
        info2->checksum = htonl(node.checksum);
        info2->linkNameLength = htonl(node.linkNameLength);
        strcpy( info2->linkName, node.linkName.c_str() );
	
        BOMPathInfo1 info1;
        info1.id = htonl( j + 1 );
        info1.index = htonl(bom.addBlock(info2, bom_path_info2_size));
        paths->indices[j].index0 = htonl( bom.addBlock( &info1, sizeof(BOMPathInfo1) ) );
        
        free((void*) info2);
	
        unsigned int bom_file_size = sizeof(uint32_t) + 1 + s.size();
        BOMFile * f = (BOMFile*)malloc( bom_file_size );
        f->parent = htonl( parent );
        strcpy( f->name, s.c_str() );
        paths->indices[j].index1 = htonl( bom.addBlock( f, bom_file_size ) );
        free( (void*) f );
	
        stack.push_back( std::pair<uint32_t, const Node*>( j + 1, &node ) );      
        j++;
      }
    }
    tree.child = htonl( bom.addBlock( paths, path_size ) );
    free( ( void*) paths );
    bom.addVar( "Paths", &tree, sizeof( BOMTree ) );
  }

  {
    unsigned int path_size = ( sizeof( uint16_t ) * 2 ) + ( sizeof( uint32_t ) * 2 );
    BOMPaths * empty_path = (BOMPaths*)malloc( path_size );
    empty_path->isLeaf = htons(1);
    empty_path->count = htons(0);
    empty_path->forward = htonl(0);
    empty_path->backward = htonl(0);

    BOMTree tree;
    memcpy( tree.tree, "tree", 4 );
    tree.version = htonl(1);
    tree.blockSize = htonl(4096);
    tree.pathCount = htonl(0);
    tree.unknown3 = 0;
    
    tree.child = htonl( bom.addBlock( empty_path, path_size ) );
    bom.addVar( "HLIndex", &tree, sizeof(BOMTree) );

    BOMVIndex vindex;
    vindex.unknown0 = htonl(1);
    tree.child = htonl( bom.addBlock( empty_path, path_size ) );
    tree.blockSize = htonl(128);
    vindex.indexToVTree = htonl( bom.addBlock( &tree, sizeof(BOMTree) ) );
    vindex.unknown2 = htonl(0);
    vindex.unknown3 = 0;
    bom.addVar( "VIndex", &vindex, sizeof(BOMVIndex) );

    tree.blockSize = htonl(4096);
    tree.child = htonl( bom.addBlock( empty_path, path_size ) );
    bom.addVar( "Size64", &tree, sizeof(BOMTree) );
   
    free( (void*)empty_path );
  }
  
  ofstream o_file( output_path.c_str(), ios::binary | ios::out );
  if ( o_file.fail() ) {
    cerr << endl << "Unable to open output file: " << output_path << endl;
    exit(1);
  }
  bom.write( o_file );
}

int main( int argc, char * argv[] ) {
  if ( ( argc != 4 ) && ( argc != 3 ) ) {
    cerr << "Usage: mkbom [-i] input output" << endl;
    return 1;
  }

  if ( string( argv[1] ) == "-i" ) {
    if ( argc != 4 ) {
      cerr << "Usage: mkbom [-i] input output" << endl;
      return 1;
    }
    ifstream file_list( argv[2] );
    if ( file_list.fail() ) {
      cerr << endl << "Unable to open file list: " << argv[2] << endl;
      return 1;
    }
    write_bom( file_list, string( argv[3] ) );
  } else {
    if ( argc != 3 ) {
      cerr << "Usage: mkbom [-i] input output" << endl;
      return 1;
    }
    string buffer;
    {
      stringstream ss;
      print_node( ss, string( argv[1] ) );
      buffer = ss.str();
    }
    stringstream file_list( buffer );
    write_bom( file_list, string( argv[2] ) );
  }
  return 0;
}
