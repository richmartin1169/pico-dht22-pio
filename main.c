#include "dht22.h"
#include "pico/stdlib.h"
#include <stdio.h>

void printData(dht22data_t*);


int main() {

	dht22data_t data;
	int c=0;
	int err=0;
    
    // console
    stdio_init_all();
    
    sleep_ms(3000);
    puts("Starting");
    
    if (dht22_init(26, true) == INIT_OK) {
    	puts("Init ok");
    
    	while (1) {
    	
			printf("\n\n*** %i ***\n", c);
			
			if (dht22_start(DHT22_2MS) == STARTED_OK) {
				puts("Started ok");
				data=dht22_get_data_blocking(true);
				if (data.status==DATA_OK)
					printData(&data);
				else
					err++;
				
			} else {
				puts("Start failed");
				err++;
			}
			
			printf("Errors: %u", err);
			data.temperature=0.0;
			data.humidity=0.0;
			data.status=DATA_NONE;
			sleep_ms(2500);
			c++;
    	}
    
    
    } else
    	puts("Init failed");
    
}


void printData(dht22data_t *d) {
	puts("\nCollected Data:");
	printf("Status: %s\n",status_text[d->status]);
	printf("Temp: %.1f\n", d->temperature);
	printf("Humidity: %.1f\n", d->humidity);
}
