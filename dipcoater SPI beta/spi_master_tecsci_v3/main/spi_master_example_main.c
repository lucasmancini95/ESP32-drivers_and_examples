/* SPI Master example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"


#define PIN_NUM_MISO 27//14//25
#define PIN_NUM_MOSI 14//23
#define PIN_NUM_CLK  12//13//19
#define PIN_NUM_CS    15//22

#define BUFFER_SIZE  7//(0x100)//255


//FUCIONES del SPI
spi_device_handle_t tmc_spi_init(){

	esp_err_t ret;
	spi_device_handle_t spi_dev;
	spi_bus_config_t buscfg={
			.miso_io_num=PIN_NUM_MISO,
			.mosi_io_num=PIN_NUM_MOSI,
			.sclk_io_num=PIN_NUM_CLK,
			.quadwp_io_num=-1,
			.quadhd_io_num=-1,
			//.max_transfer_sz=PARALLEL_LINES*320*2+8
	};
	spi_device_interface_config_t devcfg={
			.clock_speed_hz=13000,
			.mode=0,                                //SPI mode 0
			.spics_io_num=PIN_NUM_CS,               //CS pin
			.queue_size=7,                          //We want to be able to queue 7 transactions at a time
			//.pre_cb=lcd_spi_pre_transfer_callback,  //Specify pre-transfer callback to handle D/C line
	};


	//Initialize the SPI bus
	ret=spi_bus_initialize(HSPI_HOST, &buscfg, 1);
	ESP_ERROR_CHECK(ret);

	ret=spi_bus_add_device(HSPI_HOST, &devcfg, &spi_dev);
	ESP_ERROR_CHECK(ret);

	return spi_dev;
}

void tmc_spi_readWriteArray(spi_device_handle_t spi, uint8_t *tx, uint8_t *rx, size_t length){

	spi_transaction_t t;
	esp_err_t ret;
	size_t i;

	//LENGTH set
	t.length = length;  /*transaction length is in bits.*/


	/*This is to enable the use of the static structure inside the spi_transaction_t (up to: 32 bits == 4 bytes)*/
	//TX set
	if(length<=32){ /*If the data to be transferred is 32 bits or less, it can be stored in the transaction struct itself.*/
		 							/*For transmitted data, use the tx_data member for this and set the SPI_TRANS_USE_TXDATA*/
		t.flags=SPI_TRANS_USE_TXDATA; //flag to enable
		for(i=0;i<(length/8);i++){
			t.tx_data[i]=tx[i];
		}
	}
	else if (length>32) { //void *tx_buffer
		t.tx_buffer = tx;
	}

	//TX set
	//t.tx_buffer = tx;

	//RX set
	t.rx_buffer = rx;
	//t.rxlength = length;

	ret=spi_device_polling_transmit(spi, &t);
	ESP_ERROR_CHECK(ret);
}



//FUCIONES DEL EJEMPLO
static void bufferInit(uint8_t *tx_buf, uint8_t *rx_buf)
{
	uint16_t i;

	for(i = 0; i < BUFFER_SIZE; i++)
	{
		tx_buf[i] = i;
		rx_buf[i] = 0xAA;
	}
}

void bufferVerify(uint8_t *tx_buf, uint8_t *rx_buf)
{
	uint16_t i;
	for(i = 0; i < BUFFER_SIZE; i++)
	{
    printf("-----------------\n");
    printf("tx :%d -----------------------------",tx_buf[i]); // hexa:  %#08x
    printf("rx: %d\n",rx_buf[i]);
    printf("-----------------");
	}

	return;
}



void app_main(void){

		static uint8_t spi_tx_buf[BUFFER_SIZE];
		static uint8_t spi_rx_buf[BUFFER_SIZE];

		spi_device_handle_t spi_dev;

		spi_dev= tmc_spi_init();//inicializacion del spi bus y device

    bufferInit(spi_tx_buf, spi_rx_buf);//es propio del ejemplo --> es la carga del tx

		tmc_spi_readWriteArray(spi_dev,spi_tx_buf,spi_rx_buf, BUFFER_SIZE *8); //transmicion y recepcion de datos

    bufferVerify(spi_tx_buf, spi_rx_buf);//es propio del ejemplo --> es la verificacion del tx y rx

}
