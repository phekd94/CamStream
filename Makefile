
#==============================================================================

.PHONY: all clean

#==============================================================================

# compiler name
CC = gcc

#------------------------------------------------------------------------------

# directory with builded ffmpeg
FFMPEG_BUILD = /home/ekd/Documents/ffmpeg/ffmpeg_build

#------------------------------------------------------------------------------

# flags for compiler
C_FLAGS = \
	-Wall \
	-Wextra \
	-I $(FFMPEG_BUILD)/include/ \
	-I ./ \
	-c \
	$(DEBUG_FLAG) 
	# -g - for debugging enabled

#------------------------------------------------------------------------------

# SDL2_DISABLE
# FFMPEG_DISABLE
# DMAI_DISABLE

PRE_DEFINE = \
	-DSDL2_DISABLE \
	-DDMAI_DISABLE
	#-DFFMPEG_DISABLE

#------------------------------------------------------------------------------

# flags for linker
LD_FLAGS = -L$(FFMPEG_BUILD)/lib/

#------------------------------------------------------------------------------

# using libraries
LD_LIBS = -lSDL2 -lavcodec -lavutil -lavformat -lswresample
# -lmcheck - for check malloc() and free() using environment variable 
# MALLOC_CHECK_

#------------------------------------------------------------------------------

# source file list
SOURCES = \
	main.c \
	CamStream_ErrorFunctions.c \
	CamStream_Capture.c \
	CamStream_Transmit.c \
	CamStream_Print.c \
	CamStream_CalcTime.c \
	CamStream_Encode.c \
	CamStream_Decode.c \
	CamStream_InetSockets.c \
	CamStream_Terminate.c \
	CamStream_IO.c \
	CamStream_Commands.c 

#------------------------------------------------------------------------------

# objective file list from source file list
OBJECTS = $(SOURCES:.c=.o)

#------------------------------------------------------------------------------

# executable target
EXECUTABLE = cams

#==============================================================================

#all: $(SOURCES) $(EXECUTABLE)
all: $(EXECUTABLE)

#------------------------------------------------------------------------------

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LD_FLAGS) $(OBJECTS) -o $@ $(LD_LIBS)

#------------------------------------------------------------------------------

#.cpp.o:
#	$(CC) $(C_FLAGS) $< -o $@

%.o: %.c
	$(CC) $(PRE_DEFINE) $(C_FLAGS) $<

#==============================================================================

clean:
	rm -rf *.o $(EXECUTABLE)

#==============================================================================
