/* STM32 main-context I/O owner: I2C1, TIM3 microsecond counter and USART2. */
#include "platform_io.h"
#include "app_config.h"
#include "board.h"
#include "stm32g0xx_hal.h"
#include "stm32g0xx_ll_usart.h"
#include <stddef.h>
#include <stdio.h>

static I2C_HandleTypeDef sensor_bus;
static TIM_HandleTypeDef pulse_timer;
static UART_HandleTypeDef diagnostic_uart;
static char telemetry_buffer[256];
static uint16_t telemetry_length;
static uint16_t telemetry_position;
static bool is_initialized;
static bool is_bus_busy;

/* Transfers run only in the single cooperative main context, never in ISRs. */
static bool platform_io_claim(uint8_t address, const uint8_t *bytes, uint16_t length)
{
    if (!is_initialized || is_bus_busy || (address < 8U) ||
        (address > 119U) || (bytes == NULL) || (length == 0U) || (length > 128U))
    {
        return false;
    }
    is_bus_busy = true;
    return true;
}

void platform_io_log(const char *message)
{
    uint16_t length = 0U;
    if ((message == NULL) || !is_initialized || (telemetry_position < telemetry_length))
    {
        return;
    }
    while ((length < 160U) && (message[length] != '\0'))
    {
        length++;
    }
    if (HAL_UART_Transmit(&diagnostic_uart, (uint8_t *)message, length, 20U) != HAL_OK)
    {
        /* Do not recurse into logging when the diagnostic transport fails. */
        return;
    }
}

bool platform_io_init(void)
{
    GPIO_InitTypeDef gpio = {0};
    if (is_initialized)
    {
        return true;
    }
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    gpio.Pin = APP_LED_GPIO_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_WritePin(APP_LED_GPIO_PORT, APP_LED_GPIO_PIN, (GPIO_PinState)board_led_level(false));
    HAL_GPIO_Init(APP_LED_GPIO_PORT, &gpio);
    __HAL_RCC_USART2_CLK_ENABLE();
    gpio.Pin = APP_UART_TX_PIN | APP_UART_RX_PIN;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = GPIO_AF1_USART2;
    HAL_GPIO_Init(APP_UART_GPIO_PORT, &gpio);
    diagnostic_uart.Instance = USART2;
    diagnostic_uart.Init.BaudRate = APP_UART_BAUD;
    diagnostic_uart.Init.WordLength = UART_WORDLENGTH_8B;
    diagnostic_uart.Init.StopBits = UART_STOPBITS_1;
    diagnostic_uart.Init.Parity = UART_PARITY_NONE;
    diagnostic_uart.Init.Mode = UART_MODE_TX_RX;
    diagnostic_uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    diagnostic_uart.Init.OverSampling = UART_OVERSAMPLING_16;
    diagnostic_uart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    diagnostic_uart.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    if (HAL_UART_Init(&diagnostic_uart) != HAL_OK)
    {
        return false;
    }
    is_initialized = true;
    __HAL_RCC_I2C1_CLK_ENABLE();
    gpio.Pin = APP_I2C_SCL_PIN | APP_I2C_SDA_PIN;
    gpio.Mode = GPIO_MODE_AF_OD;
    gpio.Pull = GPIO_PULLUP;
    gpio.Alternate = GPIO_AF6_I2C1;
    HAL_GPIO_Init(APP_I2C_GPIO_PORT, &gpio);
    sensor_bus.Instance = I2C1;
    /* Cube polling example timing at the unchanged 56 MHz peripheral clock. */
    sensor_bus.Init.Timing = APP_I2C_TIMING;
    sensor_bus.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    sensor_bus.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    sensor_bus.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    sensor_bus.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    sensor_bus.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if ((HAL_I2C_Init(&sensor_bus) != HAL_OK) ||
        (HAL_I2CEx_ConfigAnalogFilter(&sensor_bus, I2C_ANALOGFILTER_ENABLE) != HAL_OK) ||
        (HAL_I2CEx_ConfigDigitalFilter(&sensor_bus, 0U) != HAL_OK))
    {
        platform_io_log("FAULT I2C init\r\n");
        is_initialized = false;
        return false;
    }
    __HAL_RCC_TIM3_CLK_ENABLE();
    pulse_timer.Instance = TIM3;
    pulse_timer.Init.Prescaler = (HAL_RCC_GetPCLK1Freq() / 1000000U) - 1U;
    pulse_timer.Init.CounterMode = TIM_COUNTERMODE_UP;
    pulse_timer.Init.Period = 65535U;
    pulse_timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    pulse_timer.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if ((HAL_TIM_Base_Init(&pulse_timer) != HAL_OK) ||
        (HAL_TIM_Base_Start(&pulse_timer) != HAL_OK))
    {
        platform_io_log("FAULT TIM3 init\r\n");
        is_initialized = false;
        return false;
    }
    gpio.Pin = APP_DHT_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_OD;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_WritePin(APP_DHT_GPIO_PORT, APP_DHT_PIN, GPIO_PIN_SET);
    HAL_GPIO_Init(APP_DHT_GPIO_PORT, &gpio);
    platform_io_log("BOOT sensor-v1 I2C1 PB8/PB9 DHT PB4 internal-pullups\r\n");
    return true;
}

bool platform_io_telemetry(const char *record)
{
    uint16_t length = 0U;
    if (!is_initialized || (record == NULL) || (telemetry_position < telemetry_length))
    {
        return false;
    }
    while ((length < sizeof(telemetry_buffer)) && (record[length] != '\0'))
    {
        length++;
    }
    if (length >= sizeof(telemetry_buffer))
    {
        return false;
    }
    for (uint16_t index = 0U; index < length; index++)
    {
        telemetry_buffer[index] = record[index];
    }
    telemetry_length = length;
    telemetry_position = 0U;
    return true;
}

void platform_io_poll(void)
{
    if ((telemetry_position < telemetry_length) &&
        (__HAL_UART_GET_FLAG(&diagnostic_uart, UART_FLAG_TXE) != RESET))
    {
        LL_USART_TransmitData8(USART2, (uint8_t)telemetry_buffer[telemetry_position]);
        telemetry_position++;
    }
}

void platform_io_led(bool on)
{
    HAL_GPIO_WritePin(APP_LED_GPIO_PORT, APP_LED_GPIO_PIN, (GPIO_PinState)board_led_level(on));
}

uint32_t platform_io_now_ms(void)
{
    return HAL_GetTick();
}

bool platform_io_read(uint8_t address, uint8_t reg, uint8_t *bytes, uint16_t length)
{
    HAL_StatusTypeDef status;
    if (!platform_io_claim(address, bytes, length))
    {
        return false;
    }
    status = HAL_I2C_Mem_Read(&sensor_bus, (uint16_t)((uint16_t)address << 1U),
                              reg, I2C_MEMADD_SIZE_8BIT, bytes, length, APP_I2C_TIMEOUT_MS);
    is_bus_busy = false;
    return status == HAL_OK;
}

bool platform_io_write(uint8_t address, uint8_t reg, uint8_t value)
{
    HAL_StatusTypeDef status;
    if (!platform_io_claim(address, &value, 1U))
    {
        return false;
    }
    status = HAL_I2C_Mem_Write(&sensor_bus, (uint16_t)((uint16_t)address << 1U),
                               reg, I2C_MEMADD_SIZE_8BIT, &value, 1U, APP_I2C_TIMEOUT_MS);
    is_bus_busy = false;
    return status == HAL_OK;
}

bool platform_io_transmit(uint8_t address, uint8_t *bytes, uint16_t length)
{
    HAL_StatusTypeDef status;
    if (!platform_io_claim(address, bytes, length))
    {
        return false;
    }
    status = HAL_I2C_Master_Transmit(&sensor_bus, (uint16_t)((uint16_t)address << 1U),
                                     bytes, length, APP_I2C_TIMEOUT_MS);
    is_bus_busy = false;
    return status == HAL_OK;
}

void platform_io_scan(void)
{
    char line[96];
    uint32_t count = 0U;
    (void)snprintf(line, sizeof(line), "BUS clock=%lu SCL=%u SDA=%u\r\n",
        (unsigned long)HAL_RCC_GetPCLK1Freq(),
        (unsigned int)HAL_GPIO_ReadPin(APP_I2C_GPIO_PORT, APP_I2C_SCL_PIN),
        (unsigned int)HAL_GPIO_ReadPin(APP_I2C_GPIO_PORT, APP_I2C_SDA_PIN));
    platform_io_log(line);
    for (uint16_t address = 8U; address <= 119U; address++)
    {
        if (HAL_I2C_IsDeviceReady(&sensor_bus, (uint16_t)(address << 1U), 1U, 2U) == HAL_OK)
        {
            count++;
            (void)snprintf(line, sizeof(line), "BUS ACK 0x%02x\r\n", (unsigned int)address);
            platform_io_log(line);
        }
    }
    (void)snprintf(line, sizeof(line), "BUS sweep complete ACKs=%lu\r\n", (unsigned long)count);
    platform_io_log(line);
}

/* Wait for a single transition; 16-bit elapsed arithmetic tolerates timer wrap. */
static bool platform_io_wait_level(GPIO_PinState level, uint16_t timeout_us)
{
    uint16_t start = (uint16_t)__HAL_TIM_GET_COUNTER(&pulse_timer);
    while (HAL_GPIO_ReadPin(APP_DHT_GPIO_PORT, APP_DHT_PIN) != level)
    {
        if ((uint16_t)((uint16_t)__HAL_TIM_GET_COUNTER(&pulse_timer) - start) >= timeout_us)
        {
            return false;
        }
    }
    return true;
}

bool platform_io_dht_frame(uint8_t frame[5])
{
    const uint16_t pulse_timeout_us = 120U;
    const uint16_t bit_one_threshold_us = 48U;
    uint16_t start;
    if ((frame == NULL) || !is_initialized)
    {
        return false;
    }
    for (uint8_t index = 0U; index < 5U; index++)
    {
        frame[index] = 0U;
    }
    HAL_GPIO_WritePin(APP_DHT_GPIO_PORT, APP_DHT_PIN, GPIO_PIN_RESET);
    start = (uint16_t)__HAL_TIM_GET_COUNTER(&pulse_timer);
    while ((uint16_t)((uint16_t)__HAL_TIM_GET_COUNTER(&pulse_timer) - start) < 1100U)
    {
        /* Aosong section 6 requires a start low lasting at least 1 ms. */
    }
    HAL_GPIO_WritePin(APP_DHT_GPIO_PORT, APP_DHT_PIN, GPIO_PIN_SET);
    if (!platform_io_wait_level(GPIO_PIN_RESET, pulse_timeout_us) ||
        !platform_io_wait_level(GPIO_PIN_SET, pulse_timeout_us) ||
        !platform_io_wait_level(GPIO_PIN_RESET, pulse_timeout_us))
    {
        return false;
    }
    for (uint8_t bit = 0U; bit < 40U; bit++)
    {
        if (!platform_io_wait_level(GPIO_PIN_SET, pulse_timeout_us))
        {
            return false;
        }
        start = (uint16_t)__HAL_TIM_GET_COUNTER(&pulse_timer);
        if (!platform_io_wait_level(GPIO_PIN_RESET, pulse_timeout_us))
        {
            return false;
        }
        frame[bit / 8U] = (uint8_t)(frame[bit / 8U] << 1U);
        if ((uint16_t)((uint16_t)__HAL_TIM_GET_COUNTER(&pulse_timer) - start) >= bit_one_threshold_us)
        {
            frame[bit / 8U] |= 1U;
        }
    }
    return true;
}

/* ---- firmgen_telemetry: generated UART accessor ---- */
/* Added by FirmGen so live telemetry can transmit on this already-initialised
 * UART. `diagnostic_uart` is `static`, i.e. addressable only from inside this file,
 * so a getter here is the only way to hand it to another translation unit.
 * Telemetry only transmits; it never configures the peripheral. Removing
 * telemetry removes this block again. */
UART_HandleTypeDef *firmgen_uart_handle(void)
{
    return &diagnostic_uart;
}
/* ---- firmgen_telemetry: end generated UART accessor ---- */
