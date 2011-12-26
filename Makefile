CC = gcc
CFLAGS = -O3 -s -Wall -lshlwapi -Wl,--nxcompat -Wl,--dynamicbase -mwindows

EXE = elevate.exe

all: build

elevate.exe: elevate.o elevate.rc.o

build: $(EXE)

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

%.rc.o: %.rc
	windres.exe $^ $@

%.exe:
	$(CC) $^ $(CFLAGS) -o $@

clean :
	$(RM) -rf *.o *.exe

.PHONY: all build clean release
.SUFFIXES:
