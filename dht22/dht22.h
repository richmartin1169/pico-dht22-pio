#include "pico/stdlib.h"

#ifndef DHT22_H
#define DHT22_h

/** \mainpage
*	## DHT22 Sensor library for the Raspberry Pico
*
*	This code contains all necessary functions to be able to connect a DHT22
*	sensor to your Pico.
*	## Features
*		- Any Pin can be nominated for sensor connection
*		- DHT communication is performed via a PIO program
*			- CPU is not involved in performing the data collection
*			- CPU load does not affect timings
*		- Communication failure is detectable and can be automatically handled
*		- Handles both positive and negative temperatures
*		- DHT22 start pulse width can be explicitly set
*
*
*	## Usage
*	Call the functions in this order:
*		-# Call dht22_init()
*		-# Call dht22_start()
*		-# Call dht22_get_data_blocking()
*
*
*
*	## Design
*	
*	dht22_init() initialises the PIO state machine and also initialises a semaphore.\n
*	The semaphore is a token system where a maximum of only one token is available. In this case it is used as a lock. It is created initially unlocked.
*	When the semaphore has been acquired by a part of the program, it cannot be acquired elsewhere and in trying to acquire one, the execution blocks until it does become
*	available. This can only happen by another part of the program giving back its token (releasing the semaphore token).\n
*	Within dht22_init() the PIO program is enabled and starts running but immediately blocks, waiting for a value to be written into its TX FIFO.\n
*	\n
*	dht22_start() causes the lock to be acquired and sends a value to the PIO TX FIFO which unblocks the PIO program, creating the start pulse. The value
*	sent to the FIFO determines the width of the start pulse.
*	This lock is now used to indicate that a sensor reading is in progress. dht22_start() then returns to the caller.\n
*	The PIO program however, is still running, listening for the start acknowledgement from the sensor, followed by 40 bits of data. After receipt of every
*	16 bits of data, these are pushed into the PIO RX FIFO which is 4*32bits deep.\n
*	When all 40 bits have been received (pushed into 3 words of RX FIFO), the PIO raises IRQ0, which has been configured to be routed to the wider Pico system, rather than just used
*	within the PIO subsystem. The PIO program now blocks, waiting for the IRQ to be cleared.\n
*	The IRQ0 handler pulls the 3 values from the PIO RX FIFO and places them into its own data structure. Then it clears the interrupt, allowing the
*	PIO program to resume, only for the PIO program to almost immediately block again, waiting for a new value to be pushed to its TX FIFO to create the next start pulse.
*	The IRQ handler finally releases the semaphore lock.\n
*	In this way, the semaphore lock is an indicator that a sensor readings is still in progress, or has finished.\n
*	\n
*	A subsequent call to dht22_get_data_blocking() immediately tries to acquire the lock. If data collection hasn't finished yet (maybe because dht22_get_data_blocking()
*	was called within a few mS of calling dht22_start()) then the lock will not be available, so execution blocks until it is available (which can only happen if
*	the PIO has collected all 40 bits of data, raised an IRQ and the IRQ handler releases the lock, indicating data collection has finished.)\n
*	If the CPU spends some time doing something else between calling dht22_start() and dht22_get_data_blocking() then the lock may be immediately available without any blocking.\n
*	A potential failure mode exists; what if the sensor data pulses somehow get corrupted? If the PIO program does not receive all 40 bits of data then it will hang forever
*	waiting for pulses or edges that will never appear.\n
*	And when dht22_get_data_blocking() is called and it waits for the lock to become available it will never acquire the lock (the IRQ handler will never be called which releases the
*	semaphore lock).\n
*	The software ensures that the wait for the lock in dht22_get_data_blocking() is time-limited to a #define value of 15mS. If the lock cannot be acquired within 15mS, there is an option available
*	to reset and restart the PIO program, thereby setting the PIO state machine back to its starting state. Operations can be resumed, without a need to call dht22_init().
*
*/






/**
*	\brief Describes the status of the sensor
*	
*	Used in various return values
*
*/
typedef enum dht22_status {
	INIT_OK,
	INIT_ALREADY,
	NOT_INIT,
	STARTED_OK,
	STARTED_ALREADY,
	DATA_NONE,
	DATA_RAW,
	DATA_OK,
	DATA_CHECKSUM_FAIL,
	NOT_ENOUGH_DATA_IN_FIFO,
	PIO_RESET
} dht22_status_t;


/**
*	@brief Reverse of dht22_status
*
*	A simple string array containing the text for each dht22_status enum entry
*/
extern const char * status_text[];


/**
*	\brief Start pulse length
*
*	The DHT22 requires a start pulse of at least 1mS.
*	The 40 collected data bits are at most 4.8mS long, so an option here allows you
*	to choose how long a start pulse you are comfortable with given your timing requirements in your program
*/
typedef enum dht22_start_ms {
	DHT22_1MS=300,
	DHT22_2MS=600,
	DHT22_3MS=900,
	DHT22_4MS=1200,
	DHT22_5MS=1500
} dht22_start_ms_t;


/**
*	\brief Position of the raw data within the array
*
*
*/
typedef enum dht22_raw {
	HUMIDITY=0,
	TEMP=1,
	CHECKSUM=3
} dht22_raw_t;


/**
*	\brief Data structure used to hold both raw and converted data
*
*	When the data has been fully collected from the sensor, the raw data section of this strcuture is filled.
*	When dht22_get_data_blocking() is called, the raw data is converted into the temperature and humidity values
*	and the checksum calculated to test data validity
*/
typedef struct dht22data {
	uint16_t raw_data[3]; ///<Raw data from the sensor. Use enum dht22_raw to decode.
	uint8_t calculated_checksum; ///<Checksum calculated from the sensor raw data
	dht22_status_t status; ///<Status of the data structure. See dht22_status_t
	float temperature; ///<The temperature in degrees Celcius (decoded from the raw data when dht22_get_data_blocking() is called)
	float humidity; ///<The relative humidity (decoded from the raw data when dht22_get_data_blocking() is called)
} dht22data_t;


/**
*	\brief Initialise the sensor code
*
*	This must be called before all other functions in order to initialise the pin and the PIO state machine
*
*	\param pin The GPIO number of the pin used to connect to the DHT22 sensor
*	\param debugOn When set to true enables a good level of tracing through the program
*	\return A status code
*/
dht22_status_t dht22_init(uint8_t pin, bool debugOn);


/**
*	\brief Starts the data collection
*
*	After dht22_init has been called, this function must be called to start the sensor measurement
*
*	\param start_pulse_width The desired start pulse width. The DHT22 documentation states >1mS, but
*	the options available here are from 1 to 5mS in 1mS steps.
*	\return A status code
*/
dht22_status_t dht22_start(dht22_start_ms_t start_pulse_width);


/**
*	\brief Collect measureed sensor data
*
*	Once dht22_start has been called, this function can be called immediately after or some time later.
*	If the data collection has not yet finished, this function will block until it is finished, then return the data
*
*	\param	reset_on_failure if true, and if the sensor does not finish sending data after 15mS, a reset and restart of the PIO state
*	machine is performed.The returned status is set to reflect this.
*	\return A data structure containing the raw data, calculated checksum and decoded data.
*/
dht22data_t dht22_get_data_blocking(bool reset_on_failure);


/**
*	\brief Resets and restarts the PIO state machine
*
*	Occasionally data from the sensor may be malformed, interrupted or lost. This can reult in a PIO program
*	that is locked waiting for edges or pulses that will never arrive. This function allows a complete reset and restart
*	of the PIO state machine to allow it to be used again. dht22_init is not required to be called again, only dht22_start() and dht22_get_data_blocking().
*/
void dht22_reset();

#endif
