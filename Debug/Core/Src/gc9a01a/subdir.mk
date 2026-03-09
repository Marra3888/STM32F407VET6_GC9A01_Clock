################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/gc9a01a/gc9a01a.c 

OBJS += \
./Core/Src/gc9a01a/gc9a01a.o 

C_DEPS += \
./Core/Src/gc9a01a/gc9a01a.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/gc9a01a/%.o Core/Src/gc9a01a/%.su Core/Src/gc9a01a/%.cyclo: ../Core/Src/gc9a01a/%.c Core/Src/gc9a01a/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I"C:/Users/Zver/Downloads/ST.COM/STM32F407VET6_GC9A01_Clock/Core/Src" -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"../Core/Src/demo" -I"../Core/Src/display" -I"../Core/Src/display/fonts" -I"../Core/Src/gc9a01a" -I"../Core/Src/gpio" -I"../Core/Src/rtc" -I"../Core/Src/spim" -I"../Core/Src/timers" -O3 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-gc9a01a

clean-Core-2f-Src-2f-gc9a01a:
	-$(RM) ./Core/Src/gc9a01a/gc9a01a.cyclo ./Core/Src/gc9a01a/gc9a01a.d ./Core/Src/gc9a01a/gc9a01a.o ./Core/Src/gc9a01a/gc9a01a.su

.PHONY: clean-Core-2f-Src-2f-gc9a01a

