# --------------------------------------------------------------------
# 漢直窓 1.27   Makefile for Borland C++ Compiler 5.5.1
# --------------------------------------------------------------------
#
# 「漢索窓」(kansaku.exe) のためのエントリを追加しています。
# 「漢索窓」のソースとして追加したファイルは以下のとおり:
# - kansaku.h
# - kansaku.c
# - kansaku.ico
# - kansakum.ico
# - kansaku.rc

# --------------------------------------------------------------------

TARGET	= kanchoku.exe
TARGET2	= kansaku.exe
TARGET3	= wininfo.exe
TARGET4	= kanchar.dll

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

# 漢直窓
##<v127a - gg>
##SRCS	= main.c table_window.c block.c parser.c mg_table.c control_window.c \
##	  tcode.c st_table.c moji.c bushu_dic.c debug.c
SRCS	= main.c table_window.c block.c parser.c mg_table.c control_window.c \
	  tcode.c st_table.c moji.c bushu_dic.c debug.c gg_dic.c
##</v127a - gg>
RES	= kanchoku.ico kanmini0.ico kanmini1.ico
OBJS	= $(SRCS:.c=.obj)

# 漢索窓
##<multishift2>
##SRCS2	= kansaku.c block.c moji.c parser.c st_table.c debug.c
SRCS2	= kansaku.c block.c moji.c parser.c st_table.c debug.c tcode.c
##</multishift2>
RES2	= kansaku.ico kansakum.ico
OBJS2	= $(SRCS2:.c=.obj)

SRCS3	= wininfo.c
OBJS3	= $(SRCS3:.c=.obj)

SRCS4	= kanchar.c
OBJS4	= $(SRCS4:.c=.obj)

# --------------------------------------------------------------------

all: $(TARGET) $(TARGET2) $(TARGET3) $(TARGET4)

# --------------------------------------------------------------------

# 漢直窓
$(TARGET): $(OBJS) kanchoku.res
	$(LINKER) $(GUIFLAGS) @&&!
c0w32.obj $(OBJS)
$(TARGET)
kanchoku.map
$(LIBS)
	# default .def file
kanchoku.res
!

# 漢索窓
$(TARGET2): $(OBJS2) kansaku.res
	$(LINKER) $(GUIFLAGS) @&&!
c0w32.obj $(OBJS2)
$(TARGET2)
kansaku.map
$(LIBS)
	# default .def file
kansaku.res
!

$(TARGET3): $(OBJS3)
	$(LINKER) $(GUIFLAGS) @&&!
c0w32.obj $(OBJS3)
$(TARGET3)
wininfo.map
$(LIBS)
	# default .def file
	# no .res file
!

$(TARGET4): $(OBJS4)
	$(DLLLINKER) @&&!
c0d32.obj $(OBJS4)
$(TARGET4)
kanchar.map
$(LIBS) imm32.lib
	# default .def file
	# no .res file
!
# --------------------------------------------------------------------

# 漢直窓
kanchoku.res: kanchoku.rc $(RES)
	$(RC) $(RCFLAGS) kanchoku.rc

# 漢索窓
kansaku.res: kansaku.rc $(RES2)
	$(RC) $(RCFLAGS) kansaku.rc

# --------------------------------------------------------------------

%.obj: %.c
	$(CC) $(CFLAGS) $*.c

##<v127a - gg>
gg_dic.obj: gg_dic.c gg_dic.h moji.h debug.h
##</v127a - gg>
block.obj: block.c block.h tc.h debug.h
bushu_dic.obj: bushu_dic.c bushu_dic.h moji.h debug.h
control_window.obj: control_window.c control_window.h table_window.h
main.obj: main.c control_window.h table_window.h
mg_table.obj: mg_table.c mg_table.h debug.h
moji.obj: moji.c moji.h debug.h
parser.obj: parser.c parser.h block.h moji.h tc.h debug.h
st_table.obj: st_table.c st_table.h block.h moji.h tc.h debug.h
##<v127a - gg>
##table_window.obj: table_window.c table_window.h block.h bushu_dic.h mg_table.h moji.h parser.h tcode.h version.h debug.h
##tcode.obj: tcode.c tcode.h tc.h block.h bushu_dic.h mg_table.h moji.h st_table.h table_window.h debug.h
table_window.obj: table_window.c table_window.h block.h bushu_dic.h mg_table.h moji.h parser.h tcode.h version.h debug.h gg_dic.h
tcode.obj: tcode.c tcode.h tc.h block.h bushu_dic.h mg_table.h moji.h st_table.h table_window.h debug.h gg_dic.h
##</v127a - gg>
debug.obj: debug.c debug.h
##<multishift2>
##kansaku.obj: kansaku.c kansaku.h block.h parser.h st_table.h
kansaku.obj: kansaku.c kansaku.h block.h parser.h st_table.h tcode.c
##</multishift2>

wininfo.obj: wininfo.c

kanchar.obj: kanchar.c kanchar.h

# --------------------------------------------------------------------

install: all
	- del ..\$(TARGET)
	- del ..\$(TARGET2)
	- del ..\$(TARGET3)
	- del ..\$(TARGET4)
	copy $(TARGET) ..
	copy $(TARGET2) ..
	copy $(TARGET3) ..
	copy $(TARGET4) ..

clean:
	- del *.obj
	- del *.exe
	- del *.dll
	- del *.res
	- del *.tds

# --------------------------------------------------------------------
