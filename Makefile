CC=gcc
CFLAGS=-pedantic -Wall -Wextra -std=c11 -O2 $(shell pkg-config --cflags --libs libudev)
BIN=displayDetector

all: ${BIN}

${BIN}: displayDetector.c Makefile
	${CC} -o ${BIN} displayDetector.c ${CFLAGS}

clean:
	rm ${BIN}

