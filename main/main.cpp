
#include <string.h>
#include <string>


#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "Main";

class Juice{
    public:
        int iNumber;
        std::string sWhatItDoes;

        void getWhatItDoes(){
            ESP_LOGI(TAG, "Number %i %s", iNumber, sWhatItDoes.c_str());
        }
        void getWhatItWants(){
            ESP_LOGE(TAG, "%i Wants chocolate", iNumber);
        }
};




extern "C" void app_main()
{

    Juice myJuice1;
    Juice myJuice2;

    myJuice1.iNumber = 1;
    myJuice1.sWhatItDoes = "It makes your head explode";

    myJuice2.iNumber = 2;
    myJuice2.sWhatItDoes = "hahahahahahahaaa, blblblblblb-hahahaha";


    while (1)
    {

        myJuice1.getWhatItDoes();
        vTaskDelay((1000 / portTICK_PERIOD_MS));
        myJuice2.getWhatItDoes();
        vTaskDelay((1000 / portTICK_PERIOD_MS));
        
    }
}