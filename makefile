CC=gcc
CFLAGS=-Wall `pkg-config --cflags gtk+-3.0`
LDFLAGS=`pkg-config --libs gtk+-3.0` -lhamlib -lpulse -lpulse-simple -lsndfile

all: dash

dash: main.c radio_control.c
	$(CC) $(CFLAGS) -o dash main.c radio_control.c record.c $(LDFLAGS)

clean:
	rm -f dash
	