# gepasp makefile
CC=gcc
# todo: Optimisierung und Name anpassen
TARGET=main
OPTIMIZATION=3
COPTS=-g -mcpu=cortex-a53 -mfpu=neon -mfloat-abi=hard -marm -o $(TARGET) -O$(OPTIMIZATION)

all: bin

bin: main.c windowImage.S zoomImage.S
	$(CC) $(COPTS) $+ 

clean:
	rm $(TARGET)
