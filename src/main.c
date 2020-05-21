#include "main.h"
#include <stm32f1xx.h>
#include <time.h>
#include <LoRa.h>
#include <UART.h>
#include <DHT.h>

#define LED 13

// #define BAND    433E6
const long BAND = 4330E5;
#define DHT_PORT GPIOA
#define DHT_PIN 3
#define DHT_TYPE DHT11

#define LORA_MODE
#define UART_MODE

#define LEN1 8
#define LEN2 5
#define LEN3 7
#define LEN4 5
#define LEN5 14
const uint8_t buf1[LEN1] = "temp = ";
const uint8_t buf2[LEN2] = " C\r\n";
const uint8_t buf3[LEN3] = "Hum = ";
const uint8_t buf4[LEN4] = {' ', '%', '\r', '\n', 0};
const uint8_t buf5[LEN5] = "Scan error!\r\n";


#define USART 1
#ifdef UART_MODE
static const UARTInitStructure_t UARTInitStr = 
{
//   .bus_freq = 36000000,
  .bus_freq = 8000000,
  .baud = 9600,
  .data_bits = 8,
  .stop_bits = 1,
  .parity = 0,
};
#endif

uint32_t time = 10000;
int main() {
    // clock_init();
    RCC->CFGR &= ~RCC_CFGR_PPRE1;
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV16;

#ifdef LORA_MODE
    struct LoRaClass lora;
    LoRaClass_init(&lora);
    begin(&lora, BAND, 1);
    ms_delay(15);
#endif
#ifdef UART_MODE
    UART_init(USART, &UARTInitStr);
#endif

    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN; // enable the clock to GPIO
    GPIOC->CRH &= ~(GPIO_CRH_MODE13 | GPIO_CRH_CNF13); //для начала все сбрасываем в ноль
    GPIOC->CRH |= GPIO_CRH_MODE13_1;
    GPIOC->ODR &= ~GPIO_ODR_ODR13;
    // GPIOC->ODR |= GPIO_ODR_ODR13;

#ifdef LORA_MODE
    setTxPower(&lora, 14, RF_PACONFIG_PASELECT_PABOOST);
#endif
    for (int j = 0; j < 3; ++j) {
        GPIOC->ODR &= ~(1<<LED);
        ms_delay(500);
        GPIOC->ODR |= (1<<LED);
        ms_delay(500);
    }
    struct DHT_t dht = DHT_init(DHT_PORT, DHT_PIN, DHT_TYPE);
    DHT_begin(&dht);
    while(1) {
        int8_t er = DHT_check_sensor(&dht);
#ifdef LORA_MODE
        beginPacket(&lora, 0);
#endif
        ms_delay(10);
#ifdef LORA_MODE
        setTxPower(&lora, 14, RF_PACONFIG_PASELECT_PABOOST);
#endif
        uint8_t i = 0;
        if (er == 0) {
            int temp = DHT_get_temperature(&dht);
            int hum = DHT_get_humidity(&dht);
            for (i = 0; i < LEN1-1; ++i) {
#ifdef LORA_MODE
                write(&lora, buf1[i]);
#endif
#ifdef UART_MODE
                UART_print(USART, buf1[i]);
#endif
                // ms_delay(20);
            }
            if (temp > 9) {
#ifdef LORA_MODE
                write(&lora, ((uint8_t)(temp/10))+'0');
#endif
#ifdef UART_MODE
                UART_print(USART, (((uint8_t)(temp/10))+'0'));
#endif
            }
#ifdef LORA_MODE
            write(&lora, ((uint8_t)(temp%10))+'0');
#endif
#ifdef UART_MODE
            UART_print(USART, ((uint8_t)(temp%10))+'0');
#endif
            for (i = 0; i < LEN2-1; ++i) {
#ifdef LORA_MODE
                write(&lora, buf2[i]);
#endif
#ifdef UART_MODE
                UART_print(USART, buf2[i]);
#endif
                // ms_delay(20);
            }
            for (i = 0; i < LEN3-1; ++i) {
#ifdef LORA_MODE
                write(&lora, buf3[i]);
#endif
#ifdef UART_MODE
                UART_print(USART, buf3[i]);
#endif
                // ms_delay(20);
            }
            if (hum > 9) {
#ifdef LORA_MODE
                write(&lora, ((uint8_t)(hum/10))+'0');
#endif
#ifdef UART_MODE
                UART_print(USART, ((uint8_t)(hum/10))+'0');
#endif
            }
#ifdef LORA_MODE
            write(&lora, ((uint8_t)(hum%10))+'0');
#endif
#ifdef UART_MODE
            UART_print(USART, ((uint8_t)(hum%10))+'0');
#endif
            for (i = 0; i < LEN4; ++i) {
#ifdef LORA_MODE
                write(&lora, buf4[i]);
#endif
#ifdef UART_MODE
                if (i < LEN4-1)
                    UART_print(USART, buf4[i]);
#endif
                // ms_delay(20);
            }
            for (i = 0; i < 1; ++i) {
                GPIOC->ODR &= ~(1<<LED);
                ms_delay(1500);
                GPIOC->ODR |= (1<<LED);
                ms_delay(1500);
            }
            time = 10000;
        }
        else {
            UART_print(USART, '5');
            for (i = 0; i < LEN5; ++i) {
#ifdef LORA_MODE
                write(&lora, buf5[i]);
#endif
#ifdef UART_MODE
                UART_print(USART, buf5[i]);
#endif
                // ms_delay(20);
            }
            for (i = 0; i < 3; ++i) {
                GPIOC->ODR &= ~(1<<LED);
                ms_delay(500);
                GPIOC->ODR |= (1<<LED);
                ms_delay(500);
            }
            time = 2500;
        }
        ms_delay(10);
#ifdef LORA_MODE
        endPacket(&lora, 0);
#endif
        ms_delay(10000);
    }

    return 0;
}