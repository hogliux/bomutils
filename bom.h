/*
  bom.h - structure of a bom file

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

  Initial work done by Joseph Coffland. Further contributions by Julian Devlin.
  Numerous further improvements by Baron Roberts.
*/
#pragma once
#include <stdint.h>

#if defined(WINDOWS)
#pragma pack(push,1)
#endif

struct BOMHeader {
  char magic[8]; /* "BOMStore" */
  uint32_t version; /* always 1 */
  uint32_t numberOfBlocks; /* number of non-null entries in BOMBlockTable */
  uint32_t indexOffset; /*  offset to index table */
  uint32_t indexLength; /* length of index table, indexOffset + indexLength = file_length */
  uint32_t varsOffset; 
  uint32_t varsLength; 
} __attribute__((packed));


struct BOMPointer {
  uint32_t address;
  uint32_t length;
} __attribute__((packed));


struct BOMBlockTable {
  uint32_t numberOfBlockTablePointers; /* not all of them will be occupied. See header for number of non-null blocks */
  BOMPointer blockPointers[]; /* first entry must always be a null entry */
} __attribute__((packed)); /* immedietely after this comes the free list */

struct BOMFreeList {
  uint32_t numberOfFreeListPointers;
  BOMPointer freelistPointers[]; /* mkbom seems to add two more empty freelist pointers add the end and includes this in header->indexLength */
} __attribute__((packed));

struct BOMInfoEntry {
  uint32_t unknown0; /* always zero ? */
  uint32_t unknown1; /* always zero ? */
  uint32_t unknown2; /* some obscure value??? zero for empty boms?? */
  uint32_t unknown3; /* always zero ? */
} __attribute__((packed));

struct BOMInfo {
  uint32_t version; /* version always 1 */
  uint32_t numberOfPaths; /* number of leaves + 1 ? */
  uint32_t numberOfInfoEntries; /* number of what follows */
  BOMInfoEntry entries[];
} __attribute__((packed));

struct BOMTree { 
  char tree[4]; /* "tree" */
  uint32_t version; /* always 0x1 */
  uint32_t child; /* index for BOMPaths */
  uint32_t blockSize; /* always 4096 */
  uint32_t pathCount; /* total number of paths in all leaves combined */
  uint8_t unknown3;
} __attribute__((packed));

struct BOMVIndex { 
  uint32_t unknown0; /* always 1 */
  uint32_t indexToVTree; 
  uint32_t unknown2; /* always 0 */
  uint8_t unknown3; /* always 0 */
} __attribute__((packed));

struct BOMVar {
  uint32_t index;
  uint8_t length;
  char name[];
} __attribute__((packed));


struct BOMVars {
  uint32_t count;
  BOMVar first[];
} __attribute__((packed));


struct BOMPathIndices {
  uint32_t index0;
  uint32_t index1;
} __attribute__((packed));


struct BOMPaths {
  uint16_t isLeaf;
  uint16_t count; 
  uint32_t forward;
  uint32_t backward;
  BOMPathIndices indices[];
} __attribute__((packed));


enum {
  TYPE_FILE = 1,
  TYPE_DIR  = 2,
  TYPE_LINK = 3,
  TYPE_DEV  = 4
};

struct BOMPathInfo2 {
  uint8_t type; // See types above
  uint8_t unknown0; // = 1?
  uint16_t architecture; // Not sure exactly what this means
  uint16_t mode;
  uint32_t user;
  uint32_t group; 
  uint32_t modtime;
  uint32_t size;
  uint8_t unknown1; // = 1?
  union {
    uint32_t checksum;
    uint32_t devType;
  };
  uint32_t linkNameLength;
  char linkName[];
} __attribute__((packed));


struct BOMPathInfo1 {
  uint32_t id;
  uint32_t index; /* Pointer to BOMPathInfo2 */
} __attribute__((packed));


struct BOMFile {
  uint32_t parent; /* Parent BOMPathInfo1->id */
  char name[];
} __attribute__((packed));

#if defined(WINDOWS)
#pragma pack(pop)
#endif
