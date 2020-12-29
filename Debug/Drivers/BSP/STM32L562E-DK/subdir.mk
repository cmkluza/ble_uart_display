################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/STM32L562E-DK/stm32l562e_dk_bus.c 

C_DEPS += \
./Drivers/BSP/STM32L562E-DK/stm32l562e_dk_bus.d 

OBJS += \
./Drivers/BSP/STM32L562E-DK/stm32l562e_dk_bus.o 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/STM32L562E-DK/stm32l562e_dk_bus.o: ../Drivers/BSP/STM32L562E-DK/stm32l562e_dk_bus.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DUSE_FULL_LL_DRIVER -DSTM32L562xx -DDEBUG -c -I../Core/Inc -I"C:/Users/cmklu/OneDrive/Documents/Projects/stm32l5_ble_uart_display/ble_uart_display/Core/BLE" -I../Drivers/STM32L5xx_HAL_Driver/Inc -I../Drivers/STM32L5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L5xx/Include -I../Drivers/CMSIS/Include -I../Drivers/BSP/STM32L562E-DK -I../Middlewares/ST/BlueNRG-MS/utils -I../Middlewares/ST/BlueNRG-MS/includes -I../Middlewares/ST/BlueNRG-MS/hci/hci_tl_patterns/Basic -I../BlueNRG_MS/Target -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Drivers/BSP/STM32L562E-DK/stm32l562e_dk_bus.d" -MT"$@" --specs=nano_c_standard_cpp.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

