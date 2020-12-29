################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Core/BLE/ble.cpp \
../Core/BLE/bleuart.cpp 

OBJS += \
./Core/BLE/ble.o \
./Core/BLE/bleuart.o 

CPP_DEPS += \
./Core/BLE/ble.d \
./Core/BLE/bleuart.d 


# Each subdirectory must supply rules for building sources it contributes
Core/BLE/ble.o: ../Core/BLE/ble.cpp
	arm-none-eabi-g++ "$<" -mcpu=cortex-m33 -g3 -DUSE_HAL_DRIVER -DUSE_FULL_LL_DRIVER -DSTM32L562xx -DDEBUG -c -I../Core/Inc -I"C:/Users/cmklu/OneDrive/Documents/Projects/stm32l5_ble_uart_display/ble_uart_display/Core/BLE" -I../Drivers/STM32L5xx_HAL_Driver/Inc -I../Drivers/STM32L5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L5xx/Include -I../Drivers/CMSIS/Include -I../Drivers/BSP/STM32L562E-DK -I../Middlewares/ST/BlueNRG-MS/utils -I../Middlewares/ST/BlueNRG-MS/includes -I../Middlewares/ST/BlueNRG-MS/hci/hci_tl_patterns/Basic -I../BlueNRG_MS/Target -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -Wextra -Wconversion -std=gnu++17 -fstack-usage -MMD -MP -MF"Core/BLE/ble.d" -MT"$@" --specs=nano_c_standard_cpp.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Core/BLE/bleuart.o: ../Core/BLE/bleuart.cpp
	arm-none-eabi-g++ "$<" -mcpu=cortex-m33 -g3 -DUSE_HAL_DRIVER -DUSE_FULL_LL_DRIVER -DSTM32L562xx -DDEBUG -c -I../Core/Inc -I"C:/Users/cmklu/OneDrive/Documents/Projects/stm32l5_ble_uart_display/ble_uart_display/Core/BLE" -I../Drivers/STM32L5xx_HAL_Driver/Inc -I../Drivers/STM32L5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L5xx/Include -I../Drivers/CMSIS/Include -I../Drivers/BSP/STM32L562E-DK -I../Middlewares/ST/BlueNRG-MS/utils -I../Middlewares/ST/BlueNRG-MS/includes -I../Middlewares/ST/BlueNRG-MS/hci/hci_tl_patterns/Basic -I../BlueNRG_MS/Target -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -Wextra -Wconversion -std=gnu++17 -fstack-usage -MMD -MP -MF"Core/BLE/bleuart.d" -MT"$@" --specs=nano_c_standard_cpp.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

