CC=gcc
CFLAGS=-Wall `pkg-config --cflags gtk+-3.0`
LDFLAGS=`pkg-config --libs gtk+-3.0` -lhamlib 

all: dash record

dash: main.c radio_control.c
	$(CC) $(CFLAGS) -o dash main.c radio_control.c $(LDFLAGS)
record: record.c
	$(CC) -o record record.c -lpulse -lpulse-simple -lsndfile 
clean:
	rm -f dash
	rm -f record