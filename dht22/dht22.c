

#include "pico/stdlib.h"
#include "pico/sync.h"
#include "hardware/pio.h"
#include "hardware/irq.h"
#include <stdio.h>
#include "dht22.pio.h"
#include "dht22.h"

/// DHT timeout in mS (could be as early as from start of START command)
#define DHT22_TIMEOUT 15

void pio_irq(void);

PIO pio;
uint sm, offset, dht22_pin;
bool is_init=false;
semaphore_t in_progress;
dht22data_t data;
bool debug=false;


const char * status_text[]={
	"INIT_OK",
	"INIT_ALREADY",
	"NOT_INIT",
	"STARTED_OK",
	"STARTED_ALREADY",
	"DATA_NONE",
	"DATA_RAW",
	"DATA_OK",
	"DATA_CHECKSUM_FAIL",
	"NOT_ENOUGH_DATA_IN_FIFO",
	"PIO_RESET"
	};

void pio_irq(void) {
	if (debug) printf("IRQ0: Raised. In handler\n");
	int v=pio_sm_get_rx_fifo_level(pio, sm);
	if (debug) printf("IRQ0: Rx FIFO level: %i\n",v);
	if (v==3) {
		data.raw_data[HUMIDITY]=pio_sm_get_blocking(pio, sm);
		data.raw_data[TEMP]=pio_sm_get_blocking(pio, sm);
		data.raw_data[CHECKSUM]=pio_sm_get_blocking(pio, sm);
		data.status=DATA_RAW;
	} else
		pio_sm_clear_fifos(pio, sm);
		
	sem_release(&in_progress);
	pio_interrupt_clear(pio0, 0);
}



dht22_status_t dht22_init(uint8_t pin, bool debugOn) {
	if (!is_init) {
		debug=debugOn;
		// IRQ setup
		irq_set_enabled(PIO0_IRQ_0, true);
		irq_set_exclusive_handler(PIO0_IRQ_0, pio_irq);
		
    	// PIO setup
		pio=pio0;
		offset=pio_add_program(pio, &dht22pio_program);
		sm=pio_claim_unused_sm(pio, true);
		dht22pio_program_init(pio, sm, offset, pin);
		dht22_pin=pin;
		
		
		sem_init(&in_progress, 0, 1);
		if (!sem_available(&in_progress))
			sem_release(&in_progress);
		is_init=true;
		data.status=DATA_NONE;
		if (debug) printf("INIT: PIO is configured and running, locks and IRQ are enabled\n");
		return INIT_OK;
    } else
    	return INIT_ALREADY;
    

}



dht22_status_t dht22_start(dht22_start_ms_t low_ms) {
	uint16_t val;
	if (is_init) {
		if (sem_available(&in_progress)) {
			if (debug) printf("START: Unlocked\n");
			pio_sm_put_blocking(pio, sm, low_ms);
			sem_acquire_blocking(&in_progress);
			return STARTED_OK;
		} else {
			if (debug) printf("START: Not able to start due to locking. Presume a start command has already been issued and a conversion is in progress?\n");
			return STARTED_ALREADY;
		}
	} else
		return NOT_INIT;
}


dht22data_t dht22_get_data_blocking(bool reset_on_failure) {
	uint8_t raw_rh_hi, raw_rh_low, raw_temp_hi, raw_temp_low, calcd_cs;
	float negativeTemp;
	if (debug) printf("getDataBlocking: Waiting for lock to become available\n");
	if (sem_acquire_timeout_ms(&in_progress, DHT22_TIMEOUT)) {
		
		if (debug) printf("getDataBlocking: Lock acquired successfully\n");
		if (data.status == DATA_RAW) {
			raw_rh_hi=(data.raw_data[HUMIDITY]&(0xff<<8))>>8;
			raw_rh_low=data.raw_data[HUMIDITY]&0xff;
			raw_temp_hi=(data.raw_data[TEMP]&(0xff<<8))>>8;
			raw_temp_low=data.raw_data[TEMP]&0xff;
			if (debug) printf("temp lo byte: %u\n", raw_temp_low);
			if (debug) printf("temp hi byte: %u\n", raw_temp_hi);
			data.raw_data[CHECKSUM]=(data.raw_data[CHECKSUM]&(0xff));
			calcd_cs=raw_rh_hi+raw_rh_low+raw_temp_hi+raw_temp_low;
			data.calculated_checksum=calcd_cs;
			if (calcd_cs==data.raw_data[CHECKSUM]) {
				if (raw_temp_hi>0x7f)
					negativeTemp=-1.0f;
				else
					negativeTemp=1.0f;
				data.temperature=negativeTemp*((raw_temp_hi&0x7f)*256+raw_temp_low)/10.f;
				data.humidity=(data.raw_data[HUMIDITY])/10.f;
				data.status=DATA_OK;
				if (debug) printf("getDataBlocking: Data checksum is OK\n");
			} else {
				data.status=DATA_CHECKSUM_FAIL;
				if (debug) printf("getDataBlocking: Data checksum has FAILED\n");
			}
			
		} else
			if (debug) printf("getDataBlocking: data structure does not have DATA_RAW status\n");
			
	} else {
		if (debug) printf("getDataBlocking: Lock could not be acquired, timed out.\n");
		if (reset_on_failure) {
			dht22_reset();
			printf("PIO has been reset\n");
		}
		data.status=PIO_RESET;
	}
	sem_release(&in_progress);
	return data;
}

void dht22_reset() {
	pio_sm_clear_fifos(pio, sm);
	dht22pio_program_init(pio, sm, offset, dht22_pin);
}








