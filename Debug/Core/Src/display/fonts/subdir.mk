################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/display/fonts/f16f.c \
../Core/Src/display/fonts/f24f.c \
../Core/Src/display/fonts/f32f.c \
../Core/Src/display/fonts/f6x8m.c \
../Core/Src/display/fonts/font.c 

OBJS += \
./Core/Src/display/fonts/f16f.o \
./Core/Src/display/fonts/f24f.o \
./Core/Src/display/fonts/f32f.o \
./Core/Src/display/fonts/f6x8m.o \
./Core/Src/display/fonts/font.o 

C_DEPS += \
./Core/Src/display/fonts/f16f.d \
./Core/Src/display/fonts/f24f.d \
./Core/Src/display/fonts/f32f.d \
./Core/Src/display/fonts/f6x8m.d \
./Core/Src/display/fonts/font.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/display/fonts/%.o Core/Src/display/fonts/%.su Core/Src/display/fonts/%.cyclo: ../Core/Src/display/fonts/%.c Core/Src/display/fonts/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I"C:/Users/Zver/Downloads/ST.COM/STM32F407VET6_GC9A01_Clock/Core/Src" -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"../Core/Src/demo" -I"../Core/Src/display" -I"../Core/Src/display/fonts" -I"../Core/Src/gc9a01a" -I"../Core/Src/gpio" -I"../Core/Src/rtc" -I"../Core/Src/spim" -I"../Core/Src/timers" -O3 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-display-2f-fonts

clean-Core-2f-Src-2f-display-2f-fonts:
	-$(RM) ./Core/Src/display/fonts/f16f.cyclo ./Core/Src/display/fonts/f16f.d ./Core/Src/display/fonts/f16f.o ./Core/Src/display/fonts/f16f.su ./Core/Src/display/fonts/f24f.cyclo ./Core/Src/display/fonts/f24f.d ./Core/Src/display/fonts/f24f.o ./Core/Src/display/fonts/f24f.su ./Core/Src/display/fonts/f32f.cyclo ./Core/Src/display/fonts/f32f.d ./Core/Src/display/fonts/f32f.o ./Core/Src/display/fonts/f32f.su ./Core/Src/display/fonts/f6x8m.cyclo ./Core/Src/display/fonts/f6x8m.d ./Core/Src/display/fonts/f6x8m.o ./Core/Src/display/fonts/f6x8m.su ./Core/Src/display/fonts/font.cyclo ./Core/Src/display/fonts/font.d ./Core/Src/display/fonts/font.o ./Core/Src/display/fonts/font.su

.PHONY: clean-Core-2f-Src-2f-display-2f-fonts

