################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/B-L4S5I-IOT01A/wifi.c 

OBJS += \
./Drivers/BSP/B-L4S5I-IOT01A/wifi.o 

C_DEPS += \
./Drivers/BSP/B-L4S5I-IOT01A/wifi.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/B-L4S5I-IOT01A/%.o Drivers/BSP/B-L4S5I-IOT01A/%.su Drivers/BSP/B-L4S5I-IOT01A/%.cyclo: ../Drivers/BSP/B-L4S5I-IOT01A/%.c Drivers/BSP/B-L4S5I-IOT01A/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L4S5xx -c -I/Users/wafiq/STM32CubeIDE/workspace_2.1.1/Testing_projects/HIVE_MQ/Drivers/BSP/B-L4S5I-IOT01A -I/Users/wafiq/STM32CubeIDE/workspace_2.1.1/Testing_projects/HIVE_MQ/Middlewares/ST/AI/Inc -I/Users/wafiq/STM32CubeIDE/workspace_2.1.1/Testing_projects/HIVE_MQ/X-CUBE-AI/App -I/Users/wafiq/STM32CubeIDE/workspace_2.1.1/Testing_projects/HIVE_MQ/Drivers/BSP/Components/es_wifi -I/Users/wafiq/STM32CubeIDE/workspace_2.1.1/Testing_projects/HIVE_MQ/Middlewares/Third_Party/mbedTLS/include -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-B-2d-L4S5I-2d-IOT01A

clean-Drivers-2f-BSP-2f-B-2d-L4S5I-2d-IOT01A:
	-$(RM) ./Drivers/BSP/B-L4S5I-IOT01A/wifi.cyclo ./Drivers/BSP/B-L4S5I-IOT01A/wifi.d ./Drivers/BSP/B-L4S5I-IOT01A/wifi.o ./Drivers/BSP/B-L4S5I-IOT01A/wifi.su

.PHONY: clean-Drivers-2f-BSP-2f-B-2d-L4S5I-2d-IOT01A

