/*******************************************************************************//**
    * @file Main.h
    * @author Ivan Gabriel Puente
    * @date 26/02/2023
    * @brief Archivo principal main
  **********************************************************************************/

 /***********************************************************************************
  * @brief  Librerias asociadas
  **********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ssd1306.h"

 /***********************************************************************************
  * @brief  Defines
  **********************************************************************************/
#define TAG "SSD1306"
#define IMAGES 10
#define BLINK_GPIO 2


/***********************************************************************************
  * @brief  Llamados a variables
  **********************************************************************************/
SSD1306_t dev;
TaskHandle_t Task1_Oled = NULL;
TaskHandle_t Tarea2_Timer = NULL;

/***********************************************************************************
  * @brief  Funciones
  **********************************************************************************/
/*******************************************************************************//**
  * @function Task1_Oled_Execution 
  * @brief  Tarea que ejecuta un timer. Se ejecuta cada 1 seg. 
  * @param  void *arg 
  * @retval void
  **********************************************************************************/
void Task1_Oled_Execution(void *arg)
{
    int STATE = 0;
    int sec = 59;
    int min = 59;
    int hour = 1;
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
        ssd1306_display_text(&dev, 0,string, 10, STATE);
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
    SSD1306_t dev;

#if CONFIG_I2C_INTERFACE
    // !< Mensajes x monitor
    ESP_LOGI(TAG, "INTERFACE is i2c");
    ESP_LOGI(TAG, "CONFIG_SDA_GPIO=%d",CONFIG_SDA_GPIO);
    ESP_LOGI(TAG, "CONFIG_SCL_GPIO=%d",CONFIG_SCL_GPIO);
    ESP_LOGI(TAG, "CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
    // !< Inicializacion de comunicacion I2C: 
    // !< Argumentos: Estructura, GPIO SDA, GPIO SCL, GPIO RESET
    i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);
#endif // CONFIG_I2C_INTERFACE

#if CONFIG_SPI_INTERFACE
    ESP_LOGI(TAG, "INTERFACE is SPI");
    ESP_LOGI(TAG, "CONFIG_MOSI_GPIO=%d",CONFIG_MOSI_GPIO);
    ESP_LOGI(TAG, "CONFIG_SCLK_GPIO=%d",CONFIG_SCLK_GPIO);
    ESP_LOGI(TAG, "CONFIG_CS_GPIO=%d",CONFIG_CS_GPIO);
    ESP_LOGI(TAG, "CONFIG_DC_GPIO=%d",CONFIG_DC_GPIO);
    ESP_LOGI(TAG, "CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
    // !< Inicializacion de comunicacion I2C: 
    // !< Argumentos: Estructura, GPIO MOSI, GPIO SCLK, GPIO CS, GPIO DC, GPIO RESET
    spi_master_init(&dev, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_RESET_GPIO);
#endif // CONFIG_SPI_INTERFACE

#if CONFIG_FLIP
    // !< Girar pantalla OLED
    dev._flip = true;
    ESP_LOGW(TAG, "Flip upside down");
#endif

#if CONFIG_SSD1306_128x64
    ESP_LOGI(TAG, "Panel is 128x64");
    // !< Inicializacion pantalla ssd1306
    ssd1306_init(&dev, 128, 64);
#endif // CONFIG_SSD1306_128x64
#if CONFIG_SSD1306_128x32
    ESP_LOGE(TAG, "Panel is 128x32. This demo cannot be run.");
    while(1) { vTaskDelay(1); }
#endif // CONFIG_SSD1306_128x32

    // !< Seteamos contraste 0xff -> Contraste maximo
    ssd1306_contrast(&dev, 0xff);

    // Generamos un buffer liberando espacio en memora: 128 x (8 Paginas de 8 bits) = 8192 bits
    uint8_t *buffer = (uint8_t *)malloc(8*128); // 8 page 128 pixel
    if (buffer == NULL) {
        ESP_LOGE(TAG, "malloc failed");
        while(1) { vTaskDelay(1); }
    }
    // Generamos un buffer de segmento? liberando espacio en memora: 32 x (8 Paginas de 8 bits) = 8192 bits
    uint8_t *segmentImage = (uint8_t *)malloc(IMAGES*8*32); // 10 image 8 page 32 pixel
    if (segmentImage == NULL) {
        ESP_LOGE(TAG, "malloc failed");
        while(1) { vTaskDelay(1); }
    }

    // Convert from segmentDisplay to segmentImage
    for (int imageIndex=0;imageIndex<IMAGES;imageIndex++) {
        ssd1306_clear_screen(&dev, false);
        ssd1306_bitmaps(&dev, 0, 8, segmentDisplay[imageIndex], 32, 48, false);
        vTaskDelay(200 / portTICK_PERIOD_MS);

        // Get from internal buffer to local buffer
        // buffer is [8][128] 8 page 128 pixel
        ssd1306_get_buffer(&dev, buffer);

        // Save from buffer to segmentImage
        // segmentImage is [10][8][32] 10 image 8 page 32 pixel
        int segmentImageIndex = imageIndex * 256;
        for (int page=0;page<8;page++) {
            //ESP_LOGI(TAG, "segmentImageIndex+page*32=%d", segmentImageIndex+page*32);
            memcpy(&segmentImage[segmentImageIndex+page*32], &buffer[page*128], 32);
            //ESP_LOGI(TAG, "page=%d", page);
            //ESP_LOG_BUFFER_HEXDUMP(TAG, &buffer[page*128], 32, ESP_LOG_INFO);
        }

#if 0
        ssd1306_clear_screen(&dev, false);
        for (int page=0;page<8;page++) {
            ssd1306_display_image(&dev, page, 0, &segmentImage[segmentImageIndex+page*32], 32);
            //ESP_LOGI(TAG, "page=%d", page);
            //ESP_LOG_BUFFER_HEXDUMP(TAG, &segmentImage[segmentImageIndex+page*32], 32, ESP_LOG_INFO);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
#endif
    }

    // I don't use this anymore
    free(buffer);

    int digit1 = 0;
    int digit2 = 0;
    int digit3 = 0;
    int digit4 = 0;

    // !< Limpiamos la pantalla
    ssd1306_clear_screen(&dev, false);

    // !< Creamos la pantalla
    xTaskCreate(Task1_Oled_Execution, "Task1_Oled_Execution", 4096, NULL, 10, &Task1_Oled);

    // !< Rutina de main  
    while(1) 
    {
        // !< Dormimos eternamente
        vTaskDelay(8);
    } 
}