#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ssd1306.h"
#include "driver/gpio.h"
#include "driver/adc.h"

/*
 You have to set this config value with menuconfig
 CONFIG_INTERFACE
 for i2c
 CONFIG_MODEL
 CONFIG_SDA_GPIO
 CONFIG_SCL_GPIO
 CONFIG_RESET_GPIO
 for SPI
 CONFIG_CS_GPIO
 CONFIG_DC_GPIO
 CONFIG_RESET_GPIO
*/

 /***********************************************************************************
  * @brief  Defines
  **********************************************************************************/
#define TAG "SSD1306"
#define IMAGES 10
#define BLINK_GPIO 2
#define ADC_1_6 34
#define ADC_1_7 35

 /***********************************************************************************
  * @brief  Funciones
  **********************************************************************************/


/***********************************************************************************
  * @brief  Llamados a variables
  **********************************************************************************/
// !< Estructura de Oled
SSD1306_t dev;
// !< Funcion para caracterizar la calibracion
esp_adc_cal_characteristics_t adc1_chars;
// !< Tareas Axis
TaskHandle_t Task1_Oled_Execution = NULL;
//TaskHandle_t Tarea2_ShowAxisy = NULL;

/***********************************************************************************
  * @brief  Funciones
  **********************************************************************************/
/*******************************************************************************//**
  * @function Task1_Oled_Execution 
  * @brief  Tarea que ejecuta un timer. Se ejecuta cada 1 seg. 
  * @param  void *arg 
  * @retval void
  **********************************************************************************/
void Task1_Oled(void *arg)
{
    static int STATE = 0;
    static int sec = 59;
    static int min = 59;
    static int hour = 1;
    char string[10];
    
    while (1)
    {
        
        if (sec<=0){
            sec = 59;
            if (min <= 0){
                min = 59;
                if (hour <=0){
                    hour = 59;
                }
                else hour--;
            }
            else min--;
        }
        else sec--;
        string[0] = ' ';
        string[1] = hour/10 +'0';
        string[2] = hour%10 + '0'; 
        string[3] = ':';
        string[4] = min/10 + '0';
        string[5] = min%10 + '0';
        string[6] = ':';
        string[7] = sec/10 + '0';
        string[8] = sec%10 + '0';

        STATE = ~STATE;
        ssd1306_display_text(&dev, 1,string, 10, STATE);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}


/*******************************************************************************//**
  * @function app_main 
  * @brief  Main del programa
  * @param  void 
  * @retval void
  **********************************************************************************/

void app_main(void)
{

    // !< Funciones de inicializacion
    init_oled();

    // !< Funciones de inicializacion del ADC
    init_adc();

    // !< Creamos la pantalla
    xTaskCreate(Task1_Oled, "Task1_Oled", 4096, NULL, 10, &Task1_Oled_Execution);

    // !< Rutina de main  
    while(1) 
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    } 
}

