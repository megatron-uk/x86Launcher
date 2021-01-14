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
RMFLAGS	= -f

# Compile and link flags
SYSTEM	= dos
CFLAGS	= -0 -ox -ml -zq
LDFLAGS =
LIBS 	=
INCLUDE	= $(TOOLBASE)/h

# Targets
TARGET = launcher.exe

all: $(TARGET)

clean:
	$(RM) $(RMFLAGS) obj/*.o

OBJFILES = obj/test.o

# Exe

$(TARGET): $(OBJFILES)
	$(LD) system $(SYSTEM) $(LDFLAGS) name $(TARGET) file { $(OBJFILES) }
	
# Individual objects
	
obj/test.o: src/test.c
	$(CC) $(CFLAGS) -i=$(INCLUDE) src/test.c -fo=obj/test.o