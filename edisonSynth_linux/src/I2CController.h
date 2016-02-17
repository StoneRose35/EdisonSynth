/*
 * I2CController.h
 *
 *  Created on: Feb 17, 2016
 *      Author: philipp
 */

#ifndef I2CCONTROLLER_H_
#define I2CCONTROLLER_H_

#define I2C_ADDRESS 42

#include <alsa/asoundlib.h>
#include "Voice.h"
#include "i2c.hpp"

class I2CController {
public:
	I2CController();
	void init_i2c(Voice ** ,char * );
	virtual ~I2CController();

private:
	void readerfunction();
	static void *static_thread_method(void *arg);
	Voice ** voices_i2cc;
	pthread_t i2c_controller_thread;
	char * should_be_running;
	mraa::I2c* i2cc;
};

#endif /* I2CCONTROLLER_H_ */
