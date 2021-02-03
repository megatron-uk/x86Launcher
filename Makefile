# NOTE:
#
# You must call owsetenv.sh (part of the OpenWatcom distribution)
# before attempting to run this makefile
#

# Path to tools
TOOLBASE = /opt/toolchains/x86/watcom
CC 		= $(TOOLBASE)/binl64/wcc
ASM 		= $(TOOLBASE)/binl64/wasm
LD 		= $(TOOLBASE)/binl64/wlink
STRIP	= $(TOOLBASE)/binl64/wstrip
RM		= rm
RMFLAGS	= -f -v

# Compile and link flags
SYSTEM	= dos
CFLAGS	= -2 -bc -d0 -ox -ml -zq
LDFLAGS =
LIBS 	=
INCLUDE	= $(TOOLBASE)/h

# Targets
TARGET = launcher.exe

all: $(TARGET)

# A list of all the object files used in the launcher 
OBJFILES = obj/bmp.o obj/data.o obj/filter.o obj/fstools.o obj/gfx.o obj/ini.o obj/input.o obj/main.o obj/palette.o obj/timers.o obj/ui.o obj/utils.o obj/vesa.o

# Link the main launcher target
$(TARGET): $(OBJFILES)
	$(LD) system $(SYSTEM) $(LDFLAGS) name $(TARGET) file { $(OBJFILES) }
	
# Individual objects
obj/bmp.o: src/bmp.c
	$(CC) $(CFLAGS) -i=$(INCLUDE) src/bmp.c -fo=obj/bmp.o

obj/data.o: src/data.c
	$(CC) $(CFLAGS) -i=$(INCLUDE) src/data.c -fo=obj/data.o

obj/filter.o: src/filter.c
	$(CC) $(CFLAGS) -i=$(INCLUDE) src/filter.c -fo=obj/filter.o
	
obj/fstools.o: src/fstools.c
	$(CC) $(CFLAGS) -i=$(INCLUDE) src/fstools.c -fo=obj/fstools.o
	
obj/gfx.o: src/gfx.c
	$(CC) $(CFLAGS) -i=$(INCLUDE) src/gfx.c -fo=obj/gfx.o

obj/ini.o: src/ini.c
	$(CC) $(CFLAGS) -i=$(INCLUDE) src/ini.c -fo=obj/ini.o
	
obj/input.o: src/input.c
	$(CC) $(CFLAGS) -i=$(INCLUDE) src/input.c -fo=obj/input.o

obj/main.o: src/main.c
	$(CC) $(CFLAGS) -i=$(INCLUDE) src/main.c -fo=obj/main.o
	
obj/palette.o: src/palette.c
	$(CC) $(CFLAGS) -i=$(INCLUDE) src/palette.c -fo=obj/palette.o

obj/timers.o: src/timers.c
	$(CC) $(CFLAGS) -i=$(INCLUDE) src/timers.c -fo=obj/timers.o
	
obj/ui.o: src/ui.c
	$(CC) $(CFLAGS) -i=$(INCLUDE) src/ui.c -fo=obj/ui.o
	
obj/utils.o: src/utils.c
	$(CC) $(CFLAGS) -i=$(INCLUDE) src/utils.c -fo=obj/utils.o

obj/vesa.o: src/vesa.c
	$(CC) $(CFLAGS) -i=$(INCLUDE) src/vesa.c -fo=obj/vesa.o
	
# Clean up
clean:
	$(RM) $(RMFLAGS) obj/* 
	$(RM) $(RMFLAGS) $(TARGET)