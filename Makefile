#
#  Makefile.unix - unix/linux make file
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

SRC_DIR       := $(shell dirname "$(realpath $(firstword $(MAKEFILE_LIST)))")

CXX=g++
PREFIX=/usr

SUFFIX=
CXXFLAGS=-g -O3 -ffile-prefix-map="$(SRC_DIR)=." -fstack-protector-all -Wformat -Werror=format-security -D_FORTIFY_SOURCE=2 -s
LDFLAGS=-pie -Wl,-z,now,-z,relro
LIBS=

BIN_DIR=$(PREFIX)/bin
MAN_DIR=$(PREFIX)/share/man

include build.mk
