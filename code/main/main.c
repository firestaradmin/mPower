/* 
 * Kite_V1 core board demo
 */
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"


#include "key.h"
#include "led.h"
#include "lcd_st7789.h"

#include "ug_log.h"

/* LVGL */
#include "ug_log.h"
#include "lvgl.h"
#include "lv_port_disp.h"


void vOtherFunction( void );
void vTask_ug_tick( void * pvParameters );
void vTask_adc_task(void *pvParameters);


SemaphoreHandle_t xSemaphore = NULL;
static char buf[200];



// adc
static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channelV = ADC_CHANNEL_4; 
static const adc_channel_t channelA = ADC_CHANNEL_5; 
static const adc_bits_width_t width = ADC_WIDTH_BIT_13;
static const adc_unit_t unit = ADC_UNIT_1;
static const adc_atten_t atten = ADC_ATTEN_DB_11;
#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64          //Multisampling



void create_ui();
void vOtherFunction( void );
void gpio_init(void);
// void change_size_task(ug_task_t *p);
SemaphoreHandle_t ugLog_mutex;

void ugLogLock()
{
    xSemaphoreTake(ugLog_mutex, portMAX_DELAY );
    
}
void ugLogUnlock()
{
    xSemaphoreGive(ugLog_mutex);
}

void ug_log_mutex_init( )
{
    ugLog_mutex = xSemaphoreCreateMutex();
    if( ugLog_mutex == NULL )
    {
        printf("ug log mutex err!\r\n");
    }
}

void app_main(void)
{
	printf("mPower demo!\n");
    ug_log_mutex_init();
    ug_log_init(ug_log_mutexWay_externel, printf, ugLogLock, ugLogUnlock);
    led_init();
	key_init();
    lv_init();
    lv_port_disp_init();

    create_ui();

	vOtherFunction();
    UGINFO("every thing willl be ok!\r\n");


    while(1)
	{
        lv_timer_handler();
	}




}




void gpio_init(void)
{
	gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = ((1ULL<<6)) | ((1ULL<<2)) | ((1ULL<<1)) | ((1ULL<<3));
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    gpio_set_level(1, 0);
    gpio_set_level(2, 1);
    gpio_set_level(3, 0);
    gpio_set_level(6, 1);




    // adc
    adc1_config_width(width);
    adc1_config_channel_atten(channelV, atten);
    adc1_config_channel_atten(channelA, atten);
    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, adc_chars);
    // print_char_val_type(val_type);
}


void create_ui()
{
    lv_obj_t * btn = lv_btn_create(lv_scr_act());                   /*Add a button to the current screen*/
    lv_obj_set_pos(btn, 10, 10);                                    /*Set its position*/
    lv_obj_set_size(btn, 100, 50);                                  /*Set its size*/
    lv_obj_add_event_cb(btn, NULL, LV_EVENT_CLICKED, NULL); /*Assign a callback to the button*/

    lv_obj_t * label = lv_label_create(btn);                        /*Add a label to the button*/
    lv_label_set_text(label, "Button");                             /*Set the labels text*/
    lv_obj_center(label); 
}

// void change_size_task(ug_task_t *p)
// {
//     // static int adv = 2;
//     // ug_color_t color;
//     // if(text1->coords.x2 >= 159 || text1->coords.x1 <= 0){
//     //     adv *= -1;
//     //     color = ug_color_make(text1->coords.x1, text1->coords.x2, text1->coords.x2);
//     //     ug_label_set_color(text1, &color);
//     // }

// 	// ug_obj_move(text1,2,0);

//     static int a = 0;
//     // if(a == 0){
//     //     ug_label_set_text(text1, FA_GRIN_SQUINT"LXG");
//     //     a = 1;
//     // }
//     // else {
//     //     ug_label_set_text(text1, FA_GRIN_STARS"GGNB");
//     //     a = 0;
//     // }
//     if( xSemaphoreTake( xSemaphore, ( TickType_t ) 1000 ) == pdTRUE )
//     {

//         ug_label_set_text(text1, buf);

//         xSemaphoreGive( xSemaphore );
//     }
// }


// Function that creates a task.
void vOtherFunction( void )
{

    TaskHandle_t xHandle = NULL;
    TaskHandle_t xHandle2 = NULL;
    xSemaphore = xSemaphoreCreateBinary();

    if( xSemaphore != NULL )
    {
        xSemaphoreGive( xSemaphore );
        printf("success xSemaphoreGive( xSemaphore );\r\n");
        // The semaphore was created successfully.
        // The semaphore can now be used.
    }
 // Create the task, storing the handle.  Note that the passed parameter ucParameterToPass
 // must exist for the lifetime of the task, so in this case is declared static.  If it was just an
 // an automatic stack variable it might no longer exist, or at least have been corrupted, by the time
 // the new task attempts to access it.
	xTaskCreate( vTask_ug_tick, "vTask_ug_tick", 1024, NULL, 9, &xHandle );
	configASSERT( xHandle );
	// xTaskCreate( vTask_adc_task, "vTask_adc_task", 4096, NULL, 8, &xHandle2 );
	// configASSERT( xHandle2 );

	// Use the handle to delete the task.
	// if( xHandle != NULL )
	// {
	// 	vTaskDelete( xHandle );
	// }
}



// Task to be created.
void vTask_ug_tick( void * pvParameters )
{
	for( ;; )
	{
		// Task code goes here.
		lv_tick_inc(10);
		vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}


// Task to be created.
void vTask_adc_task( void * pvParameters )
{
    //Continuously sample ADC1
    uint32_t adc_reading = 0;
    uint32_t adc_readingA = 0;
    uint32_t voltage = 0;
    float act_voltage = 0;
    float act_voltage_f = 0;
    uint32_t voltageA = 0;
    float act_voltageA = 0;
    float act_voltageA_f = 0;
    float act_watt = 0;
    uint32_t R1 = 9900;
    uint32_t R2 = 91000;
    int i = 0;
    while (1) {
        adc_reading = 0;
        voltage = 0;
        //Multisampling
        for (i = 0; i < NO_OF_SAMPLES; i++) {
            if (unit == ADC_UNIT_1) {
                adc_reading += adc1_get_raw((adc1_channel_t)channelV);
                adc_readingA += adc1_get_raw((adc1_channel_t)channelA);
            }
        }
        adc_reading /= NO_OF_SAMPLES;
        adc_readingA /= NO_OF_SAMPLES;
        //Convert adc_reading to voltage in mV
        voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        voltageA = esp_adc_cal_raw_to_voltage(adc_readingA, adc_chars);
        if(voltage>=12)
            voltage -= 12;
        else
            voltage = 0;
        if(voltageA>=36)
            voltageA -= 36;
        else
            voltageA = 0;
        act_voltage = (float)(voltage) * (float)(R1 + R2) / (float)R1;
        act_voltage_f = act_voltage/1000;
        act_voltageA = (float)voltageA/0.01/50;
        act_voltageA_f = act_voltageA/1000;
        act_watt = act_voltage_f * act_voltageA_f;
        // printf("Raw: %d\tVoltage: %dmV\n", adc_reading, act_voltage);
        if( xSemaphoreTake( xSemaphore, ( TickType_t ) 1000 ) == pdTRUE )
        {
            memset(buf, 0, 200);
            snprintf(buf, 200, "%.2fV%.2fA\n%.3fW",act_voltage_f,act_voltageA_f,act_watt);

            xSemaphoreGive( xSemaphore );
        }




        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


