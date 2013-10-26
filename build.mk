#
#  build.mk - Makefile
#
#  Copyright (C) 2013 Fabian Renn - bomutils (at) gmail.com
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2, or (at your option)
#  any later version.
#  
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA  02110-1301 USA.
OPTFLAGS=-O2 -g0 -s

MKBOM=mkbom$(SUFFIX)
DUMPBOM=dumpbom$(SUFFIX)
LSBOM=lsbom$(SUFFIX)
LS4MKBOM=ls4mkbom$(SUFFIX)

all : $(MKBOM) $(DUMPBOM) $(LSBOM) $(LS4MKBOM)

install : all
	cp $(MKBOM) $(BIN_DIR)/
	cp $(DUMPBOM) $(BIN_DIR)/
	cp $(LSBOM) $(BIN_DIR)/
	cp $(LS4MKBOM) $(BIN_DIR)/
	cp mkbom.1 $(MAN_DIR)/man1/
	cp dumpbom.1 $(MAN_DIR)/man1/
	cp lsbom.1 $(MAN_DIR)/man1/
	cp ls4mkbom.1 $(MAN_DIR)/man1/

$(MKBOM) : mkbom.o printnode.o crc32.o
	$(CXX) -o $@ $(LDFLAGS) $^ $(LIBS)

$(DUMPBOM) : dumpbom.o 
	$(CXX) -o $@ $(LDFLAGS) $^ $(LIBS)

$(LSBOM) : lsbom.o 
	$(CXX) -o $@ $(LDFLAGS) $^ $(LIBS)

$(LS4MKBOM) : ls4mkbom.o printnode.o printnode.o crc32.o
	$(CXX) -o $@ $(LDFLAGS) $^ $(LIBS)

mkbom.o : mkbom.cpp bom.h printnode.hpp
	$(CXX) -c $(OPTFLAGS) $(CXXFLAGS) $(CFLAGS) $<

printnode.o : printnode.cpp printnode.hpp crc32.hpp
	$(CXX) -c $(OPTFLAGS) $(CXXFLAGS) $(CFLAGS) $<

crc32.o : crc32.cpp crc32.hpp crc32_poly.hpp
	$(CXX) -c $(OPTFLAGS) $(CXXFLAGS) $(CFLAGS) $<

dumpbom.o : dumpbom.cpp bom.h
	$(CXX) -c $(OPTFLAGS) $(CXXFLAGS) $(CFLAGS) $<

lsbom.o : lsbom.cpp bom.h
	$(CXX) -c $(OPTFLAGS) $(CXXFLAGS) $(CFLAGS) $<

ls4mkbom.o : ls4mkbom.cpp printnode.hpp
	$(CXX) -c $(OPTFLAGS) $(CXXFLAGS) $(CFLAGS) $<

clean :
	rm -f *.o
	rm -f $(MKBOM)
	rm -f $(DUMPBOM)
	rm -f $(LSBOM)
	rm -f $(LS4MKBOM)

