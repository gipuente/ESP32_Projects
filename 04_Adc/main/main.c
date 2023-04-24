#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ssd1306.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

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
//#define TAG "SSD1306"
#define IMAGES 10
#define BLINK_GPIO 2


/***********************************************************************************
  * @brief  Llamados a variables
  **********************************************************************************/
// !< Estructura de Oled
SSD1306_t dev;
// !< Funcion para caracterizar la calibracion
esp_adc_cal_characteristics_t adc1_chars;
// !< Tareas Axis
TaskHandle_t Task1_X = NULL;
TaskHandle_t Task2_Y = NULL;
TaskHandle_t Task3_Button = NULL;
// !< Tag para mandar mensajes x monitor
static const char *TAG = "ADC EXAMPLE";

enum button_state
{
    Button_bounce   = 0,
    Button_pushed   = 1,
    Button_relased  = 2,

};
/***********************************************************************************
  * @brief  Funciones
  **********************************************************************************/
// !< Tareas Freertos
void Task1_Adc_X(void *arg);
void Task2_Adc_Y(void *arg);
void Task3_PushButton(void *arg);

// !< Funciones de inicializacion
void init_oled(void);
void init_adc(void);

/*******************************************************************************//**
  * @function Task1_ADC_Y 
  * @brief  Tarea que ejecuta un timer. Se ejecuta cada 1 seg. 
  * @param  void *arg 
  * @retval void
  **********************************************************************************/
void Task2_Adc_Y(void *arg)
{
    static uint16_t axis_y_mV;
    char string[12] = " Axis Y: ";

    while (1)
    {
        axis_y_mV = esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_6), &adc1_chars);
        string[9]=  (char)  (axis_y_mV/1000)+'0';
        string[10]= (char)  ((axis_y_mV-((string[9]-'0')*1000))/100)+'0';
        string[11]= (char)  ((axis_y_mV-((string[9]-'0')*1000)-((string[10]-'0')*100))/10)+'0';
        string[12]= (char)  (axis_y_mV-((string[9]-'0')*1000)-((string[10]-'0')*100)-((string[11]-'0')*10))+'0';
        ESP_LOGI(TAG, "ADC1_CHANNEL_6: %d mV", axis_y_mV);
        ssd1306_display_text(&dev, 1,string, 13, 0);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}  
/*******************************************************************************//**
  * @function Task1_ADC_X 
  * @brief  Tarea que ejecuta un timer. Se ejecuta cada 1 seg. 
  * @param  void *arg 
  * @retval void
  **********************************************************************************/
void Task1_Adc_X(void *arg)
{
    static uint16_t axis_x_mV;
    char string[12] = " Axis X: ";

    while (1)
    {
        axis_x_mV = esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_3), &adc1_chars);
        string[9]=  (char) (axis_x_mV/1000)+'0';
        string[10]=  (char) ((axis_x_mV-((string[9]-'0')*1000))/100)+'0';
        string[11]= (char) ((axis_x_mV-((string[9]-'0')*1000)-((string[10]-'0')*100))/10)+'0';
        string[12]= (char) (axis_x_mV-((string[9]-'0')*1000)-((string[10]-'0')*100)-((string[11]-'0')*10))+'0';
        ESP_LOGI(TAG, "ADC1_CHANNEL_7: %d mV", axis_x_mV);
        ssd1306_display_text(&dev, 2,string, 13, 0);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

/*******************************************************************************//**
  * @function Task3_PushButton 
  * @brief  Tarea que procesa el rebote de un boton
  * @param  void *arg 
  * @retval void
  **********************************************************************************/
void Task3_PushButton(void *arg)
{
    static uint8_t state =  Button_relased;
    static int8_t bounce_counter = 0;

    while (1)
    {
        switch(state)
        {
            case Button_relased:
                if (gpio_get_level(GPIO_NUM_39) == 1) state = Button_bounce;
                else state = Button_relased;
                ssd1306_display_text(&dev, 3," Boton = Suelto", 15, 0);
                break;
            case Button_bounce:
                if (gpio_get_level(GPIO_NUM_39) == 1) bounce_counter++;
                else bounce_counter--;
                if (bounce_counter >= 15) state = Button_pushed;
                if (bounce_counter <= -15) state = Button_relased;
                ssd1306_display_text(&dev, 3," Boton = Rebote", 15, 0);
                break;
            case Button_pushed:
                if (gpio_get_level(GPIO_NUM_39) == 1) state = Button_bounce;
                else state = Button_pushed;
                ssd1306_display_text(&dev, 3," Boton = Press ", 15, 0);
                break;
            default:
                break;
        }
        ESP_LOGI(TAG, "Button State: %d ", gpio_get_level(GPIO_NUM_39));
        vTaskDelay(100 / portTICK_PERIOD_MS);
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

    // !< Inicializacion de modulos
    init_oled();
    init_adc();

    // !< Seteo el pin 36 como entrada
    gpio_set_direction(GPIO_NUM_39, GPIO_MODE_INPUT);

    // !< Creamos la pantalla
    xTaskCreate(Task1_Adc_X,        "Task1_Adc_X",      4096, NULL, 10, &Task1_X);
    xTaskCreate(Task2_Adc_Y,        "Task2_Adc_Y",      4096, NULL, 10, &Task2_Y);
    xTaskCreate(Task3_PushButton,   "Task3_PushButton", 4096, NULL, 10, &Task3_Button);

    // !< Rutina de main  
    while(1) 
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    } 
}



/*******************************************************************************//**
  * @function init_oled 
  * @brief  Inicializacion de una pantalla oled
  * @param  void 
  * @retval void
  **********************************************************************************/
void init_oled(void)
{
    // !< Inicializacion de comunicacion I2C: 
    // !< Argumentos: Estructura, GPIO SDA, GPIO SCL, GPIO RESET
    i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);

    // !< Girar pantalla OLED
    //dev._flip = true;

    // !< Inicializacion pantalla ssd1306
    ssd1306_init(&dev, 128, 64);

    // !< Seteamos contraste 0xff -> Contraste maximo
    ssd1306_contrast(&dev, 0xff);

    // !< Limpiamos la pantalla
    ssd1306_clear_screen(&dev, false);

} 

/*******************************************************************************//**
  * @function init_adc 
  * @brief  Inicializacion del adc
  * @param  void 
  * @retval void
  **********************************************************************************/
void init_adc(void)
{
    // !< Calibro el ADC 1
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);
    
    // !< Configuro la conversion de adc a 12 bits
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));

    // !< Configuro la atenuacion a 11db, mido el rango maximo
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_11));

} 