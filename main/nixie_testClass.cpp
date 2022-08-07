/*
	...


*/

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "lwip/ip4_addr.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/api.h"

#include "nixie_testClass.h"


int iAGlobalVariable;	// "global" variables work fine, but we want to get away from this concept of using global stuff!

int MyTestClass::iAStaticLocalVariable;		// "local" variables, i.e. class member variables, work fine in static member functions, however they themselves must also be static.
											// this leads to us having to declare these variables here again in cpp file. Not quite sure where they are placed now memory wise. But we can now acces them in all our class functions


MyTestClass::MyTestClass(/* args */)
{

	ESP_LOGI(TAG, "myTestClass instance created");

	iAGlobalVariable = 1;
	iAStaticLocalVariable = 1;

	// init the args
	arg.bfirstEverBool = true;
	arg.iYetAnotherInt = 100;

}

MyTestClass::~MyTestClass()
{
	ESP_LOGE(TAG, "myTestClass instance destroid");
}

void MyTestClass::freeRtosTask(void *arg)
{

	while (1)
	{
		const static char *TAG = "test_task";


		iAGlobalVariable++;
		iAStaticLocalVariable--;

		// lets cast the passed arguments to our argument type. We cast the void arg type to our type, because we know what is passed
		// as arg when the task is created
		myTestClass_arg_t* passedArgs = (myTestClass_arg_t*)arg;

		// manipulate
		passedArgs->bfirstEverBool = true;
		(passedArgs->iYetAnotherInt)++;

		//(*passedArgs).iYetAnotherInt

		ESP_LOGE(TAG, "myTestClass IS ACTUALLY WORKING, global: %i, static local: %i, passed as arg: %i",iAGlobalVariable, iAStaticLocalVariable, passedArgs->iYetAnotherInt);
		vTaskDelay((1000 / portTICK_PERIOD_MS));
	}
}
