CC=avr-gcc
OPTLEVEL=-Os
F_CPU=-DF_CPU=16000000
MMCU=-mmcu=atmega328p
CFLAGS=-c -I. $(OPTLEVEL) -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -ffunction-sections -fdata-sections $(F_CPU) $(MMCU)

all: CCSDS.o ccsds_xbee.o uart_xbee.o xbee.o timekeeper.o
	avr-ar rcs libccsdsXbee.a CCSDS.o ccsds_xbee.o uart_xbee.o xbee.o timekeeper.o

CCSDS.o: CCSDS/CCSDS.c
	$(CC) $(CFLAGS) CCSDS/CCSDS.c -o CCSDS.o

ccsds_xbee.o: ccsds_xbee/ccsds_xbee.c
	$(CC) $(CFLAGS) ccsds_xbee/ccsds_xbee.c -o ccsds_xbee.o

uart_xbee.o: uart_xbee/uart_xbee.c
	$(CC) $(CFLAGS) uart_xbee/uart_xbee.c -o uart_xbee.o

xbee.o: xbee/xbee.c
	$(CC) $(CFLAGS) xbee/xbee.c -o xbee.o

timekeeper.o: timekeeper/timekeeper.c
	$(CC) $(CFLAGS) timekeeper/timekeeper.c -o timekeeper.o

clean:
	rm *.o

purge: clean
	rm *.a

.PHONY: clean purge

