# --------------------------------------------------------------------
# Š¿’¼‘‹ 1.27   Makefile for Borland C++ Compiler 5.5.1
# --------------------------------------------------------------------
#

# --------------------------------------------------------------------

TARGET	= kwstyle.exe

# --------------------------------------------------------------------

CC	= bcc32
###RC	= brc32 -i"c:\Borland\Bcc55\include"
RC	= brc32 -i"c:\programs\Borland\Bcc55\include"
LINKER	= ilink32 -v- -x -Gn -c -Tpe
DLLLINKER	= ilink32 -v- -x -Gn -c -Tpd
GUIFLAGS= -aa
RCFLAGS	= -r -dWIN32
LIBS	= import32.lib cw32.lib
CFLAGS	= -c -DSTRICT -O2 -P

# --------------------------------------------------------------------

SRCS	= kwstyle.cpp
RES	= kwstyle.ico small.ico
OBJS	= $(SRCS:.cpp=.obj)

# --------------------------------------------------------------------

all: $(TARGET)

# --------------------------------------------------------------------

$(TARGET): $(OBJS) kwstyle.res
	$(LINKER) $(GUIFLAGS) @&&!
c0w32.obj $(OBJS)
$(TARGET)
kwstyle.map
$(LIBS)
	# default .def file
kwstyle.res
!

# --------------------------------------------------------------------

kwstyle.res: kwstyle.rc $(RES)
	$(RC) $(RCFLAGS) kwstyle.rc

# --------------------------------------------------------------------

%.obj: %.cpp
	$(CC) $(CFLAGS) $*.cpp

kwstyle.obj: kwstyle.cpp kwstyle.h resource.h stdafx.h targetver.h
stdafx.cpp: stdafx.h targetver.h

# --------------------------------------------------------------------

install: all
	- del ..\..\$(TARGET)
	copy $(TARGET) ..\..

clean:
	- del *.obj
	- del *.exe
	- del *.dll
	- del *.res
	- del *.tds

# --------------------------------------------------------------------
