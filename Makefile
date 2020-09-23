CC = gcc
CFLAGS = -g -fPIC
#CFLAGS = -g -DDEBUG

PREFIX = /usr
PUBLIC_SEQUENCES = $(PREFIX)/share/AutoZen

# uncomment one of the OS= lines below if you're compiling on one of those OSen. 
#OS= -D__FreeBSD__
#OS= -D__OpenBSD__

#DEBUG_LIBS= -lefence

all: autozen-nopulse seq2wav

strip: autozen-nopulse seq2wav
	strip autozen-nopulse seq2wav

autozen: autozen.c wave_table.o *.xpm
	$(CC) $(OS) -DGTK_ENABLE_BROKEN -D_REENTRANT -DPUBLIC_SEQUENCES='"$(PUBLIC_SEQUENCES)"' -DUSE_PULSE $(CFLAGS) `pkg-config --cflags gtk+-2.0` `pkg-config  --libs gtk+-2.0` -pthread autozen.c wave_table.o -o autozen $(DEBUG_LIBS) -l pulse-simple -lm
#	$(CC) $(OS) -D_REENTRANT -DPUBLIC_SEQUENCES='"$(PUBLIC_SEQUENCES)"' -DUSE_PULSE $(CFLAGS) `gtk-config --cflags` `gtk-config  --libs` -pthread autozen.c wave_table.o -o autozen $(DEBUG_LIBS) -l pulse-simple

autozen-nopulse: autozen.c *.xpm wave_table.o
	$(CC) $(OS) -D_REENTRANT -DPUBLIC_SEQUENCES='"$(PUBLIC_SEQUENCES)"' `pkg-config gtk+-2.0 --cflags` `pkg-config gtk+-2.0 --libs` -pthread autozen.c wave_table.o -o autozen-nopulse $(DEBUG_LIBS) -lm

seq2wav: seq2wav.c wave_table.o
	$(CC) seq2wav.c -o seq2wav wave_table.o -lm
clean: 
	rm -f autozen autozen-nopulse seq2wav *.o

audio: deep_delta_slide.wav deep-relax.wav delta_slide.wav moderate-meditation.wav relax.wav theta.wav wake-up.wav 

deep_delta_slide.wav: deep_delta_slide.seq
	./seq2wav $< $@
deep-relax.wav: deep-relax.seq
	./seq2wav $< $@
delta_slide.wav: delta_slide.seq
	./seq2wav $< $@
moderate-meditation.wav: moderate-meditation.seq
	./seq2wav $< $@
relax.wav: relax.seq
	./seq2wav $< $@
theta.wav: theta.seq
	./seq2wav $< $@
wake-up.wav: wake-up.seq
	./seq2wav $< $@



install: all
	install -d $(PREFIX)/bin
	install zentime $(PREFIX)/bin
	install -s seq2wav $(PREFIX)/bin
	install -s autozen $(PREFIX)/bin
	install -d $(PREFIX)/share/AutoZen
	install -m 644 *.seq $(PREFIX)/share/AutoZen
	install -d $(PREFIX)/share/doc/AutoZen/HTML/images
	install -m 644 doc/HTML/*.html $(PREFIX)/share/doc/AutoZen/HTML
	install -m 644 doc/HTML/images/* $(PREFIX)/share/doc/AutoZen/HTML/images
	install -d $(PREFIX)/man/man1
	install -m 644 doc/autozen.1 $(PREFIX)/man/man1

tags: *.[ch] *.xpm
	ctags *.[ch] *.xpm
