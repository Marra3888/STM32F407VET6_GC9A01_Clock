################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/display/dispcolor.c 

OBJS += \
./Core/Src/display/dispcolor.o 

C_DEPS += \
./Core/Src/display/dispcolor.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/display/%.o Core/Src/display/%.su Core/Src/display/%.cyclo: ../Core/Src/display/%.c Core/Src/display/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I"C:/Users/Zver/Downloads/ST.COM/STM32F407VET6_GC9A01_Clock/Core/Src" -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"../Core/Src/demo" -I"../Core/Src/display" -I"../Core/Src/display/fonts" -I"../Core/Src/gc9a01a" -I"../Core/Src/gpio" -I"../Core/Src/rtc" -I"../Core/Src/spim" -I"../Core/Src/timers" -O3 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-display

clean-Core-2f-Src-2f-display:
	-$(RM) ./Core/Src/display/dispcolor.cyclo ./Core/Src/display/dispcolor.d ./Core/Src/display/dispcolor.o ./Core/Src/display/dispcolor.su

.PHONY: clean-Core-2f-Src-2f-display

