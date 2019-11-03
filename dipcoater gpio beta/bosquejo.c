



bool_t tmc_gpio_config(int pin, int config) {
	esp_err_t ret;
	gpio_config_t io_conf;

	GPIO_MODE_INPUT

	io_conf.intr_type = GPIO_PIN_INTR_DISABLE; 	//disable interrupt

	//set as output or input mode
	if(config == 0){
			io_conf.mode = GPIO_MODE_OUTPUT;
	}
	else if(config == 1){
			io_conf.mode = 	GPIO_MODE_INPUT;
	}

	//bit mask of the pins that you want to set,e.g.GPIO18/19
	io_conf.pin_bit_mask = (1ULL<<pin); //GPIO_OUTPUT_PIN_SEL/GPIO_INPUT_PIN_SEL;

	io_conf.pull_down_en = 0;	//disable pull-down mode
	io_conf.pull_up_en = 0;	//disable pull-up mode

	ret= gpio_config(&io_conf); //configure GPIO with the given settings

	if(ret == ESP_OK){
		return TRUE;
	}
	else if(ret == ESP_ERR_INVALID_ARG){
		return FALSE;
	}
}

bool_t tmc_gpio_write(int pin, bool_t value) {
	esp_err_t ret;
	ret=gpio_set_level((gpio_num_t)pin, value);

	if(ret == ESP_OK){
		return TRUE;
	}
	else if(ret == ESP_ERR_INVALID_ARG){
		return FALSE;
	}

}

bool_t tmc_gpio_read(int pin) {
	return (bool_t)gpio_get_level((gpio_num_t)pin);
}


typedef emum{GPIO_OUTPUT_OPC=0 , GPIO_INPUT_OPC=1}GPIO_IO_OPC_t;

int main(int argc, char const *argv[]) {
	int example_pin_1=18;
	int example_pin_2=19;
	GPIO_IO_OPC_t io_example_1=GPIO_OUTPUT_OPC;
	GPIO_IO_OPC_t io_example_2=GPIO_INPUT_OPC;

	bool_t tmc_gpio_config(example_pin_1, (int)io_example_1); //set del pin como output
	bool_t tmc_gpio_config(example_pin_1, (int)io_example_2);

	bool_t tmc_gpio_write(int pin, bool_t value);
	bool_t tmc_gpio_read(int pin, bool_t value);

	return 0;
}
