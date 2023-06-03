# DHT22 Sensor library for the Raspberry Pico
This code contains all necessary functions to be able to connect a DHT22
sensor to your Pico.
## Features
- Any Pin can be nominated for sensor connection
- DHT communication is performed via a PIO program
    - CPU is not involved in performing the data collection
	- CPU load does not affect timings
- Communication failure is detectable and can be automatically handled
- Handles both positive and negative temperatures
- DHT22 start pulse width can be explicitly set

## Wiring
Power the sensor from the Pico 3v3 line, and connect ground.
Wire the data pin to the required Pico pin.
Add pullup of 10K between the sensor data pin and 3v3.

## Usage
Call the functions in this order:
1. Call dht22_init()
1. Call dht22_start()
1. Call dht22_get_data_blocking()

## Example program using the library
The example program (main.c) defines the sensor pin as GPIO26 and loops round measuring the temperature and humidity every 2.5 seconds.
It counts how many readings have been initiated and keeps track of any errors encountered (good for long term testing).
Debug is set ON so there's lots of debug ouput showing the locking system, IRQ handler and raw temperature data recovered from the sensor.
The start pulse is set to 2mS (datasheet states >1mS is required, and max polling rate is 2S).
Although all extraneous generated folders and files are not in this repo, the resulting **test.uf2**
file remains. This can be dragged onto the Pico in the normal way.

To build the uf2 file yourself:

Clone this repo, cd into your copy. Add pico C sdk folder location to path.

    cmake .
    make

## Pico SDK Build
### Standalone Program
Copy files dht22.c, dht22.h and dht22.pio into your project directory.
Create your normal **CMakeLists.txt**, add dht22.c into the sources list.
Also add *pico_generate_pio_header* (see dht22/CMakeLists.txt)

### Using the dht22 code as a standalone separate library
As supplied, the folder structure places the dht22.c, dht22.h and dht22.pio into a separate dht22 folder
The CMakeLists.txt in this directory builds this separate code as a standalone library, the parent folder
is a simple example of usage, and this folder contains its own CMakeLists.txt that pulls in the separately
built dht22 library.

## Design notes
For an overview of how the dht22 library works see the generated doxygen information in html/ that describes the design in more detail.
This documentation is generated from dht22/dht22.h

File dht22/dht22.pio also contains lots of comments about how the program works against the expected data stream
from the dht22 sensor. In addition there's a section there that shows timing decisions made.


