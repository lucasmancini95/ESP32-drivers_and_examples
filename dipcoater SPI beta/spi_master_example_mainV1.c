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

/*
 This code displays some fancy graphics on the 320x240 LCD on an ESP-WROVER_KIT board.
 This example demonstrates the use of both spi_device_transmit as well as
 spi_device_queue_trans/spi_device_get_trans_result and pre-transmit callbacks.

 Some info about the ILI9341/ST7789V: It has an C/D line, which is connected to a GPIO here. It expects this
 line to be low for a command and high for data. We use a pre-transmit callback here to control that
 line: every transaction has as the user-definable argument the needed state of the D/C line and just
 before the transaction is sent, the callback will set this line to the correct state.
*/

#define PIN_NUM_MISO 25 // es P0 de la placa node MCU 1.1
#define PIN_NUM_MOSI 23 // es P15 de la placa node MCU 1.1
#define PIN_NUM_CLK  19 // es CMD de la placa node MCU 1.1
#define PIN_NUM_CS   22 // es SD1 de la placa node MCU 1.1

#define MAX_TRANSFER_SIZE 1000
#define SPI_CLOCK_SPEED 10000000 //10 MHz
#define TRANSACTION_Q_SZ   2          //Transaction queue size.

#define PIN_NUM_DC   21
#define PIN_NUM_RST  18
#define PIN_NUM_BCKL 5

#define BUFFER_SIZE  (0x100)
static uint8_t spi_tx_buf[BUFFER_SIZE];
static uint8_t spi_rx_buf[BUFFER_SIZE];


static void bufferInit(uint8_t *tx_buf, uint8_t *rx_buf){ //lleno cada posicion del buffer (uint8)con 0xAA == 10101010
	uint16_t i;
	uint8_t ch = 0;

	for(i = 0; i < BUFFER_SIZE; i++)
	{
		tx_buf[i] = ch++;
		rx_buf[i] = 0xAA;
	}
}

void bufferVerify(uint8_t *tx_buf, uint8_t *rx_buf){ //verifico si son iguales el buffer que recivi y el que envie

	uint16_t i;
	for(i = 0; i < BUFFER_SIZE; i++)
	{
		if(tx_buf[i] != rx_buf[i])
		{
      printf("los buffers NO son iguales /r/n");
			return;
		}
	}
  printf("los buffers SON iguales /r/n");
	return;
}

//Simple routine to generate some patterns and send them
static void test_send(spi_device_handle_t spi){
  esp_err_t ret;
  spi_transaction_t t;

  t.length = BUFFER_SIZE * 2 * 8;                  //Len is in bytes, transaction length is in bits.
  t.tx_buffer = spi_tx_buf;                    //Data
  t.rx_buffer = spi_rx_buf;
      ret=spi_device_polling_transmit(spi, &t);  //Transmit!
      //The polling transactions don’t rely on the interrupt,
      //the routine keeps polling the status bit of the SPI peripheral until the transaction is done.
      if(ret==ESP_OK) printf(" todo okkkkk\n");
}


void app_main(void){

    esp_err_t ret; // para chqueo de errores de la inicializacion del SPI
    spi_device_handle_t spi;
    spi_bus_config_t buscfg={ //Struct specifying how the host should be initialized

        .miso_io_num=PIN_NUM_MISO, //pin de MISO
        .mosi_io_num=PIN_NUM_MOSI, //pin de MOSI
        .sclk_io_num=PIN_NUM_CLK, //pin de CLK
        .quadwp_io_num=-1, //pin for WP (Write Protect) signal --> not used --> -1
        .quadhd_io_num=-1, //pin for HD (HolD) signal --> not used --> -1
        //se pone "-1" para indicar que no se usa
        .max_transfer_sz=  MAX_TRANSFER_SIZE//Maximum transfer size, in bytes. Defaults to 4094 if 0.
    };

    spi_device_interface_config_t devcfg={
        .clock_speed_hz=SPI_CLOCK_SPEED,           //Clock out at 10 MHz
        .mode=0,                                //SPI mode 0
        .spics_io_num=PIN_NUM_CS,               //CS GPIO pin for this device, or -1 if not used. (obs: CS == chip select)
        //.queue_size=TRANSACTION_Q_SZ             //Transaction queue size.
                                                //This sets how many transactions can be ‘in the air’ at the same time.
                                                // in  the air: queued using spi_device_queue_trans but not yet finished using spi_device_get_trans_result
      //  .pre_cb=test_transfer_callback,  //Callback to be called before a transmission is started.
    };



    //Initialize the SPI bus
    printf("SPI bus initialize --------------------------- /r/n");
    ret=spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    //inicializacion del bus spi, parametros:
      //HSPI_HOST: SPI2 //y tambien VSPI_HOST = SPI3
      //pointer to bus_config --> specifying how the host should be initialized
      //dma_chan: Either channel 1 or 2, or 0 in the case when no DMA is required.
      // Selecting a DMA channel for a SPI bus allows transfers on the bus to have sizes only limited by the amount of internal memory.
      // Selecting no DMA channel (by passing the value 0) limits the amount of bytes transfered to a maximum of 64. Set to 0 if only the SPI flash uses this bus.
    ESP_ERROR_CHECK(ret); // spi bus initialize error check
    printf("SPI bus initialize done --------------------------- /r/n");
    //
    //Free a SPI bus.
    // printf("Free SPI bus--------------------------- \r\n");
    // ret=spi_bus_free(HSPI_HOST); //free the HSPI_HOST--> SPI2
    // ESP_ERROR_CHECK(ret);
    // printf("Free SPI bus done--------------------------- \r\n");

    // //Allocate a device on a SPI bus.
    printf("Add device--------------------------- \r\n");
    ret=spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
    //HSPI_HOST: SPI2
    //devcfg es un spi_device_interface_config_t
    ESP_ERROR_CHECK(ret);
    printf("Add device done--------------------------- \r\n");

    //
    //
    // printf("Buffer init--------------------------- \r\n");
    // bufferInit(spi_tx_buf, spi_rx_buf);
    //
    // //Do nice stuff.
    //     printf("Send--------------------------- \r\n");
    // test_send(spi);
    //
    // bufferVerify(spi_tx_buf, spi_rx_buf);
    //
    // printf("\r\n");
    //
  	 printf("dszfvfzevr end\r\n");

      for (int i = 1000; i >= 0; i--) {
          printf("Restarting in %d seconds...\n", i);
          vTaskDelay(1000 / portTICK_PERIOD_MS);
      }
      printf("Restarting now.\n");
      fflush(stdout);
      esp_restart();
}
