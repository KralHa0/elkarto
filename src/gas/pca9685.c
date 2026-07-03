#include "pca9685.h"
#define I2C1ClckSpeed 400000
#define I2C1Adress (0x40 << 1)

static I2C_HandleTypeDef hi2c1;

void pcaInit(){
    __HAL_RCC_I2C1_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = I2C1ClckSpeed;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&hi2c1);

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    gpio.Mode = GPIO_MODE_AF_OD;
    gpio.Pull = GPIO_PULLUP;
    gpio.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &gpio);

    HAL_I2C_IsDeviceReady(&hi2c1, I2C1Adress, 5, HAL_MAX_DELAY);

    uint8_t buf[2];

    buf[0] = 0x00; buf[1] = 0x10;
    HAL_I2C_Master_Transmit(&hi2c1, I2C1Adress, buf, 2, HAL_MAX_DELAY);

    buf[0] = 0xFE; buf[1] = 121;
    HAL_I2C_Master_Transmit(&hi2c1, I2C1Adress, buf, 2, HAL_MAX_DELAY);

    buf[0] = 0x01; buf[1] = 0x04;
    HAL_I2C_Master_Transmit(&hi2c1, I2C1Adress, buf, 2, HAL_MAX_DELAY);

    buf[0] = 0x00; buf[1] = 0x20;
    HAL_I2C_Master_Transmit(&hi2c1, I2C1Adress, buf, 2, HAL_MAX_DELAY);

    HAL_Delay(1);
}

void setServoAngle(uint16_t angle){
    uint16_t pulse = 205 + (angle * (410 - 205) / 180);
    uint8_t buf[5] = {0x06, 0, 0, pulse & 0xFF, pulse >> 8};
    HAL_I2C_Master_Transmit(&hi2c1, I2C1Adress, buf, 5, HAL_MAX_DELAY);
}

uint8_t pcaIsReady(void) {
    return HAL_I2C_IsDeviceReady(&hi2c1, I2C1Adress, 5, HAL_MAX_DELAY) == HAL_OK;
}
