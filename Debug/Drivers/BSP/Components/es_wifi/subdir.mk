################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/Components/es_wifi/es_wifi.c \
../Drivers/BSP/Components/es_wifi/es_wifi_io.c 

OBJS += \
./Drivers/BSP/Components/es_wifi/es_wifi.o \
./Drivers/BSP/Components/es_wifi/es_wifi_io.o 

C_DEPS += \
./Drivers/BSP/Components/es_wifi/es_wifi.d \
./Drivers/BSP/Components/es_wifi/es_wifi_io.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/Components/es_wifi/%.o Drivers/BSP/Components/es_wifi/%.su Drivers/BSP/Components/es_wifi/%.cyclo: ../Drivers/BSP/Components/es_wifi/%.c Drivers/BSP/Components/es_wifi/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L4S5xx -c -I/Users/wafiq/STM32CubeIDE/workspace_2.1.1/Testing_projects/HIVE_MQ/Drivers/BSP/B-L4S5I-IOT01A -I/Users/wafiq/STM32CubeIDE/workspace_2.1.1/Testing_projects/HIVE_MQ/Middlewares/ST/AI/Inc -I/Users/wafiq/STM32CubeIDE/workspace_2.1.1/Testing_projects/HIVE_MQ/X-CUBE-AI/App -I/Users/wafiq/STM32CubeIDE/workspace_2.1.1/Testing_projects/HIVE_MQ/Drivers/BSP/Components/es_wifi -I/Users/wafiq/STM32CubeIDE/workspace_2.1.1/Testing_projects/HIVE_MQ/Middlewares/Third_Party/mbedTLS/include -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-Components-2f-es_wifi

clean-Drivers-2f-BSP-2f-Components-2f-es_wifi:
	-$(RM) ./Drivers/BSP/Components/es_wifi/es_wifi.cyclo ./Drivers/BSP/Components/es_wifi/es_wifi.d ./Drivers/BSP/Components/es_wifi/es_wifi.o ./Drivers/BSP/Components/es_wifi/es_wifi.su ./Drivers/BSP/Components/es_wifi/es_wifi_io.cyclo ./Drivers/BSP/Components/es_wifi/es_wifi_io.d ./Drivers/BSP/Components/es_wifi/es_wifi_io.o ./Drivers/BSP/Components/es_wifi/es_wifi_io.su

.PHONY: clean-Drivers-2f-BSP-2f-Components-2f-es_wifi

