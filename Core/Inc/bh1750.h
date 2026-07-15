#ifndef BH1750_H
#define BH1750_H

#include "stm32l4xx_hal.h"

extern I2C_HandleTypeDef hi2c1;

#define BH1750_I2C_ADDR           0x23 << 1
#define BH1750_CMD_POWER_ON       0x01
#define BH1750_CMD_RESET          0x07
#define BH1750_CMD_CONT_HRES      0x10

static inline void BH1750_Init(void)
{
	uint8_t cmd;
	cmd = BH1750_CMD_POWER_ON;
	HAL_I2C_Master_Transmit(&hi2c1, BH1750_I2C_ADDR, &cmd, 1, HAL_MAX_DELAY);

	cmd = BH1750_CMD_RESET;
	HAL_I2C_Master_Transmit(&hi2c1, BH1750_I2C_ADDR, &cmd, 1, HAL_MAX_DELAY);

	cmd = BH1750_CMD_CONT_HRES;
	HAL_I2C_Master_Transmit(&hi2c1, BH1750_I2C_ADDR, &cmd, 1, HAL_MAX_DELAY);

	HAL_Delay(180);
}

static HAL_StatusTypeDef BH1750_ReadLux(float *lux)
{
	uint8_t data[2] = {0};
	HAL_StatusTypeDef status = HAL_I2C_Master_Receive(&hi2c1, BH1750_I2C_ADDR, data, 2, 100);
	if (status != HAL_OK)
	{
		return status;
	}
	uint16_t raw = ((uint16_t)data[0] << 8) | data[1];
	*lux = (float)raw / 1.2f;
	return HAL_OK;
}

static inline void BH1750_Recover(void)
{
    HAL_I2C_DeInit(&hi2c1);
    HAL_Delay(10);
    HAL_I2C_Init(&hi2c1);
    HAL_Delay(10);
    BH1750_Init();
}

#endif /* BH1750_H */
