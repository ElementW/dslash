#------------------------------------------------------------------------------
# DSlash, a Nintendo DS ROM tool
# based on ndstrim by recover89@gmail.com
# Copyright (C) 2008 hyplex@gmail.com
# Homepage: http://dslash.googlecode.com
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#-------------------------------------------------------------------------------

#makefile

CC=gcc
CFLAGS=-pg -g -Wall
CPPFLAGS=
OBJS=  dslash.o tables.o
FINAL = dslash

all : dslash


debug: $(OBJS)
	$(CC) -g -Wall -DMEMWATCH -o $(FINAL) $(OBJS)


dslash : $(OBJS)
	$(CC) $(CFLAGS) -o $(FINAL) $(OBJS)
	
clean: 
	rm -f $(OBJS) $(FINAL) core

