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

## Usage
Call the functions in this order:
1. Call dht22_init()
1. Call dht22_start()
1. Call dht22_get_data_blocking()

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
File dht22/dht22.pio contains lots of comments about how the program works against the expected data stream
from the dht22 sensor. In addition there's a section there that shows timing decisions made.
Also see the generated doxygen information in html/ that describes usage in more detail.

