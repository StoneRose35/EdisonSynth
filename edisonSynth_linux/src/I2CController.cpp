/*
 * I2CController.cpp
 *
 *  Created on: Feb 17, 2016
 *      Author: philipp
 */

#include <iostream>
#include "I2CController.h"
#include "constants.h"
#include "Voice.h"
#include "EdisonSynthesizer.h"
#include "i2c.hpp"
#include "common.hpp"

using namespace std;
I2CController::I2CController() {
	// TODO Auto-generated constructor stub

}

void I2CController::init_i2c(Voice ** vocs_addr,char * is_running)
{
	mraa::Result res = mraa::init();
	if (res!=mraa::SUCCESS && res !=mraa::ERROR_PLATFORM_ALREADY_INITIALISED){
		cout << "Error initializing mraa" << endl;
		cout << "returned code is " << res << endl;
	}
	should_be_running = is_running;
	i2cc=new mraa::I2c(1);
	i2cc->frequency(mraa::I2C_STD);
	voices_i2cc=vocs_addr;
	int status;
	status=pthread_create(&i2c_controller_thread,NULL,I2CController::static_thread_method,this);
	if(status ==-1)
	{
		cout << "Error creating i2c controller thread" << endl;
	}
}

void* I2CController::static_thread_method(void* args)
{
	static_cast<I2CController*>(args)->readerfunction();
	return NULL;
}

void I2CController::readerfunction()
{
	uint8_t bfr0[64];
	uint8_t bfr1[64];
	char buffer_used;
	buffer_used=0;
	while(*should_be_running==1)
	{
		i2cc->address(I2C_ADDRESS);
		if(buffer_used==0)
		{
			int rdcntr = i2cc->read(bfr0,64);
			for(int z=0;z<rdcntr;z++)
			{
				if((bfr1[z]>bfr0[z] && bfr1[z]-bfr0[z]>2) || (bfr0[z]>bfr1[z] && bfr0[z]-bfr1[z]>2) )
				{
					// set the corresponding voice parameter
					if(z==0)
					{
						for(int h=0;h<N_VOICES;h++)
					    {
							 voices_i2cc[h]->filter_cutoff_base=bfr0[z]*0.8982/2.0-48;
					    }
					}

					bfr1[z]=bfr0[z];
				}
				if(z==0)
				{

				}
			}
		}
		else
		{
			int rdcntr = i2cc->read(bfr1,64);
			for(int z=0;z<rdcntr;z++)
			{

				if((bfr1[z]>bfr0[z] && bfr1[z]-bfr0[z]>2) || (bfr0[z]>bfr1[z] && bfr0[z]-bfr1[z]>2) )
				{
					// set the corresponding voice parameter

					if(z==0)
					{
						for(int h=0;h<N_VOICES;h++)
					    {
							 voices_i2cc[h]->filter_cutoff_base=bfr1[z]*0.8982/2.0-48;
					    }
					}

					bfr0[z]=bfr1[z];
				}
			}
		}

		if(buffer_used==0)
		{
			buffer_used=1;
		}
		else
		{
			buffer_used=0;
		}
		usleep(50000);
	}
}

I2CController::~I2CController() {
	// TODO Auto-generated destructor stub
}

