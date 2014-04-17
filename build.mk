#
#  build.mk - Makefile
#
#  Copyright (C) 2013 Fabian Renn - fabian.renn (at) gmail.com
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
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA  02110-1301 USA
#
#  Initial work done by Joseph Coffland and Julian Devlin.
#  Numerous further improvements by Baron Roberts.
OPTFLAGS=-O2 -g0 -s

APP_SOURCES=\
	mkbom.cpp \
	dumpbom.cpp \
	lsbom.cpp \
	ls4mkbom.cpp

COMMON_SOURCES=\
	printnode.cpp \
	crc32.cpp

APPS=$(addsuffix $(SUFFIX),$(APP_SOURCES:.cpp=))

all : $(APPS)

install : all
	install -d $(DESTDIR)$(BIN_DIR)
	install -d $(DESTDIR)$(MAN_DIR)/man1
	install -m 0755 $(APPS) $(DESTDIR)$(BIN_DIR)
	install -m 0644 *.1 $(DESTDIR)$(MAN_DIR)/man1

%.o : %.cpp
	$(CXX) -c $(OPTFLAGS) $(CXXFLAGS) $(CFLAGS) $<

%.d : %.cpp
	@set -e; rm -f $@; $(CXX) -MM $(OPTFLAGS) $(CXXFLAGS) $(CFLAGS) $< > $@.$$$$; \
	sed -e 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$

%$(SUFFIX) : %.o $(COMMON_SOURCES:.cpp=.o)
	$(CXX) -o $@ $(LDFLAGS) $^ $(LIBS)

-include $(APP_SOURCES:.cpp=.d) $(COMMON_SOURCES:.cpp=.d)

clean :
	rm -f *.o *.d
	rm -f $(APPS)

