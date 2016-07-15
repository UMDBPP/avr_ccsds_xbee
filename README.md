# avr_ccsds_xbee
A library for interfacing avr systems with Digi's XBee modules as well as with the CCSDS protocol

## Structure
The lowest level of this library is located in the uart_xbee directory. UART stands for Universal Asynchronous Receiver/Transmitter, and is how we will be talking to the XBee itself. This is a rather heavy piece of the program, as it contains an RX buffer.

The next level of the library is located in the xbee directory. These files are for software integration with XBee's. It contains a few of the frame types that is Defined by Digi, and implements their functionality in sending and receiving byte streams of data in a structured way. It also uses the uart_xbee program to actually send data through the XBee's.

The next part of the library is the CCSDS directory. This is NASA code that defines the CCSDS protocol. Here, we can find the packet structure that we use to hold our raw data. It has two options: command and telemetry. The command option is used if we want to send a command with some parameters to another system, and the telemetry is used if we want to send data to another system.

The final part of the library is the ccsds_xbee directory. It is user facing. The ccsds_xbee files are there for creating a CCSDS packet and adding data to it. When finished, the user can use a send function that will use the xbee files to send the packet through the XBee module. These files also implement reading through a function that uses the uart_xbee files to read incoming bytes, decide whether they are CCSDS packets for command or telemetry. There are functions for each of those options to extract the corresponding information. In addition, the ccsds_xbee file also contains an initialization function that should be called by the user.
There is one more file in this directory - add_tlm.h. This file is a utility piece so that users can easily add and extract data from byte arrays. It allows for the addition of any type of integer, and float/double.

## Usage
Clone this repo to the project folder. The only file that needs to be included is the ccsds_xbee.h file located in the ccsds_xbee directory.

TODO: write about what functions are here and how to use them!
