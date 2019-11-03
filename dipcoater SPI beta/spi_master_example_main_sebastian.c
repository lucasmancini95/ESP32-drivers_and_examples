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
#include "soc/gpio_struct.h"
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

#define PIN_NUM_MISO 25
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  19
#define PIN_NUM_CS   22

#define BUFFER_SIZE  (0x100)
static uint8_t spi_tx_buf[BUFFER_SIZE];
static uint8_t spi_rx_buf[BUFFER_SIZE];

static void bufferInit(uint8_t *tx_buf, uint8_t *rx_buf)
{
	uint16_t i;
	uint8_t ch = 0;

	for(i = 0; i < BUFFER_SIZE; i++)
	{
		tx_buf[i] = ch++;
		rx_buf[i] = 0xAA;
	}
}

static uint8_t bufferVerify(uint8_t *tx_buf, uint8_t *rx_buf)
{
	uint16_t i;
	for(i = 0; i < BUFFER_SIZE; i++)
	{
		if(tx_buf[i] != rx_buf[i])
		{
			return 1;
		}
	}
	return 0;
}

void appSPIRun(spi_device_handle_t spi)
{
    esp_err_t ret;

    spi_transaction_t t;

    t.length = BUFFER_SIZE * 2 * 8;                  //Len is in bytes, transaction length is in bits.
    t.tx_buffer = spi_tx_buf;                    //Data
    ret = spi_device_polling_transmit(spi, &t);  //Transmit!

    assert(ret==ESP_OK);            //Should have had no issues.
}

void app_main()
{
    esp_err_t ret;

    spi_device_handle_t spi;

    spi_bus_config_t buscfg={
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = BUFFER_SIZE * 2,
    };

    printf("bus cfg\r\n");
    //Initialize the SPI bus
    ret=spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    ESP_ERROR_CHECK(ret);

    spi_device_interface_config_t devcfg={
        .clock_speed_hz = 10*1000*1000,           //Clock out at 10 MHz
        .mode = 0,                                //SPI mode 0
        .spics_io_num = PIN_NUM_CS,               //CS pin
    };

    printf("device cfg\r\n");
    //Attach the LCD to the SPI bus
    ret=spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);

	printf("Init\r\n");
    bufferInit(spi_tx_buf, spi_rx_buf);

	printf("TXRX ... ");
    appSPIRun(spi);

	if((bufferVerify(spi_tx_buf, spi_rx_buf) == 0))
	{
		printf("OK");
	}
	else
	{
		printf("ERROR");
	}
	printf("\r\n");

	printf("end\r\n");

    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
