CFLAGS = -O2 -mwindows -lgdi32 -s

all: clean pizdets.exe

clean:
	rm -f *.exe *.o

%.o: %.bmp
	ld -r -b binary -o $@ $<

%.exe: main.c sw.o
	gcc $^ -o $@ $(CFLAGS)
