################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/demo/ampelguys.c \
../Core/Src/demo/ampelmann.c \
../Core/Src/demo/benchmark_mode.c \
../Core/Src/demo/boat_gauges.c \
../Core/Src/demo/bodmer_single_yinyang.c \
../Core/Src/demo/bodmer_spiral.c \
../Core/Src/demo/clock.c \
../Core/Src/demo/compas.c \
../Core/Src/demo/gfx_extra.c \
../Core/Src/demo/gif_mode.c \
../Core/Src/demo/horse_frames.c \
../Core/Src/demo/htp.c \
../Core/Src/demo/humidity_meter.c \
../Core/Src/demo/hvac.c \
../Core/Src/demo/mono.c \
../Core/Src/demo/power.c \
../Core/Src/demo/rainbow.c \
../Core/Src/demo/random_move.c \
../Core/Src/demo/rtc_print_mode.c \
../Core/Src/demo/sht21_mode.c \
../Core/Src/demo/smooth_clock.c \
../Core/Src/demo/tachometer.c \
../Core/Src/demo/textfading.c \
../Core/Src/demo/therm3.c \
../Core/Src/demo/thermostat.c \
../Core/Src/demo/three_orbiting_rotating_yinyang.c \
../Core/Src/demo/voltage_meter.c \
../Core/Src/demo/watchface.c 

OBJS += \
./Core/Src/demo/ampelguys.o \
./Core/Src/demo/ampelmann.o \
./Core/Src/demo/benchmark_mode.o \
./Core/Src/demo/boat_gauges.o \
./Core/Src/demo/bodmer_single_yinyang.o \
./Core/Src/demo/bodmer_spiral.o \
./Core/Src/demo/clock.o \
./Core/Src/demo/compas.o \
./Core/Src/demo/gfx_extra.o \
./Core/Src/demo/gif_mode.o \
./Core/Src/demo/horse_frames.o \
./Core/Src/demo/htp.o \
./Core/Src/demo/humidity_meter.o \
./Core/Src/demo/hvac.o \
./Core/Src/demo/mono.o \
./Core/Src/demo/power.o \
./Core/Src/demo/rainbow.o \
./Core/Src/demo/random_move.o \
./Core/Src/demo/rtc_print_mode.o \
./Core/Src/demo/sht21_mode.o \
./Core/Src/demo/smooth_clock.o \
./Core/Src/demo/tachometer.o \
./Core/Src/demo/textfading.o \
./Core/Src/demo/therm3.o \
./Core/Src/demo/thermostat.o \
./Core/Src/demo/three_orbiting_rotating_yinyang.o \
./Core/Src/demo/voltage_meter.o \
./Core/Src/demo/watchface.o 

C_DEPS += \
./Core/Src/demo/ampelguys.d \
./Core/Src/demo/ampelmann.d \
./Core/Src/demo/benchmark_mode.d \
./Core/Src/demo/boat_gauges.d \
./Core/Src/demo/bodmer_single_yinyang.d \
./Core/Src/demo/bodmer_spiral.d \
./Core/Src/demo/clock.d \
./Core/Src/demo/compas.d \
./Core/Src/demo/gfx_extra.d \
./Core/Src/demo/gif_mode.d \
./Core/Src/demo/horse_frames.d \
./Core/Src/demo/htp.d \
./Core/Src/demo/humidity_meter.d \
./Core/Src/demo/hvac.d \
./Core/Src/demo/mono.d \
./Core/Src/demo/power.d \
./Core/Src/demo/rainbow.d \
./Core/Src/demo/random_move.d \
./Core/Src/demo/rtc_print_mode.d \
./Core/Src/demo/sht21_mode.d \
./Core/Src/demo/smooth_clock.d \
./Core/Src/demo/tachometer.d \
./Core/Src/demo/textfading.d \
./Core/Src/demo/therm3.d \
./Core/Src/demo/thermostat.d \
./Core/Src/demo/three_orbiting_rotating_yinyang.d \
./Core/Src/demo/voltage_meter.d \
./Core/Src/demo/watchface.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/demo/%.o Core/Src/demo/%.su Core/Src/demo/%.cyclo: ../Core/Src/demo/%.c Core/Src/demo/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I"C:/Users/Zver/Downloads/ST.COM/STM32F407VET6_GC9A01_Clock/Core/Src" -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"../Core/Src/demo" -I"../Core/Src/display" -I"../Core/Src/display/fonts" -I"../Core/Src/gc9a01a" -I"../Core/Src/gpio" -I"../Core/Src/rtc" -I"../Core/Src/spim" -I"../Core/Src/timers" -O3 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-demo

clean-Core-2f-Src-2f-demo:
	-$(RM) ./Core/Src/demo/ampelguys.cyclo ./Core/Src/demo/ampelguys.d ./Core/Src/demo/ampelguys.o ./Core/Src/demo/ampelguys.su ./Core/Src/demo/ampelmann.cyclo ./Core/Src/demo/ampelmann.d ./Core/Src/demo/ampelmann.o ./Core/Src/demo/ampelmann.su ./Core/Src/demo/benchmark_mode.cyclo ./Core/Src/demo/benchmark_mode.d ./Core/Src/demo/benchmark_mode.o ./Core/Src/demo/benchmark_mode.su ./Core/Src/demo/boat_gauges.cyclo ./Core/Src/demo/boat_gauges.d ./Core/Src/demo/boat_gauges.o ./Core/Src/demo/boat_gauges.su ./Core/Src/demo/bodmer_single_yinyang.cyclo ./Core/Src/demo/bodmer_single_yinyang.d ./Core/Src/demo/bodmer_single_yinyang.o ./Core/Src/demo/bodmer_single_yinyang.su ./Core/Src/demo/bodmer_spiral.cyclo ./Core/Src/demo/bodmer_spiral.d ./Core/Src/demo/bodmer_spiral.o ./Core/Src/demo/bodmer_spiral.su ./Core/Src/demo/clock.cyclo ./Core/Src/demo/clock.d ./Core/Src/demo/clock.o ./Core/Src/demo/clock.su ./Core/Src/demo/compas.cyclo ./Core/Src/demo/compas.d ./Core/Src/demo/compas.o ./Core/Src/demo/compas.su ./Core/Src/demo/gfx_extra.cyclo ./Core/Src/demo/gfx_extra.d ./Core/Src/demo/gfx_extra.o ./Core/Src/demo/gfx_extra.su ./Core/Src/demo/gif_mode.cyclo ./Core/Src/demo/gif_mode.d ./Core/Src/demo/gif_mode.o ./Core/Src/demo/gif_mode.su ./Core/Src/demo/horse_frames.cyclo ./Core/Src/demo/horse_frames.d ./Core/Src/demo/horse_frames.o ./Core/Src/demo/horse_frames.su ./Core/Src/demo/htp.cyclo ./Core/Src/demo/htp.d ./Core/Src/demo/htp.o ./Core/Src/demo/htp.su ./Core/Src/demo/humidity_meter.cyclo ./Core/Src/demo/humidity_meter.d ./Core/Src/demo/humidity_meter.o ./Core/Src/demo/humidity_meter.su ./Core/Src/demo/hvac.cyclo ./Core/Src/demo/hvac.d ./Core/Src/demo/hvac.o ./Core/Src/demo/hvac.su ./Core/Src/demo/mono.cyclo ./Core/Src/demo/mono.d ./Core/Src/demo/mono.o ./Core/Src/demo/mono.su ./Core/Src/demo/power.cyclo ./Core/Src/demo/power.d ./Core/Src/demo/power.o ./Core/Src/demo/power.su ./Core/Src/demo/rainbow.cyclo ./Core/Src/demo/rainbow.d ./Core/Src/demo/rainbow.o ./Core/Src/demo/rainbow.su ./Core/Src/demo/random_move.cyclo ./Core/Src/demo/random_move.d ./Core/Src/demo/random_move.o ./Core/Src/demo/random_move.su ./Core/Src/demo/rtc_print_mode.cyclo ./Core/Src/demo/rtc_print_mode.d ./Core/Src/demo/rtc_print_mode.o ./Core/Src/demo/rtc_print_mode.su ./Core/Src/demo/sht21_mode.cyclo ./Core/Src/demo/sht21_mode.d ./Core/Src/demo/sht21_mode.o ./Core/Src/demo/sht21_mode.su ./Core/Src/demo/smooth_clock.cyclo ./Core/Src/demo/smooth_clock.d ./Core/Src/demo/smooth_clock.o ./Core/Src/demo/smooth_clock.su ./Core/Src/demo/tachometer.cyclo ./Core/Src/demo/tachometer.d ./Core/Src/demo/tachometer.o ./Core/Src/demo/tachometer.su ./Core/Src/demo/textfading.cyclo ./Core/Src/demo/textfading.d ./Core/Src/demo/textfading.o ./Core/Src/demo/textfading.su ./Core/Src/demo/therm3.cyclo ./Core/Src/demo/therm3.d ./Core/Src/demo/therm3.o ./Core/Src/demo/therm3.su ./Core/Src/demo/thermostat.cyclo ./Core/Src/demo/thermostat.d ./Core/Src/demo/thermostat.o ./Core/Src/demo/thermostat.su ./Core/Src/demo/three_orbiting_rotating_yinyang.cyclo ./Core/Src/demo/three_orbiting_rotating_yinyang.d ./Core/Src/demo/three_orbiting_rotating_yinyang.o ./Core/Src/demo/three_orbiting_rotating_yinyang.su ./Core/Src/demo/voltage_meter.cyclo ./Core/Src/demo/voltage_meter.d ./Core/Src/demo/voltage_meter.o ./Core/Src/demo/voltage_meter.su ./Core/Src/demo/watchface.cyclo ./Core/Src/demo/watchface.d ./Core/Src/demo/watchface.o ./Core/Src/demo/watchface.su

.PHONY: clean-Core-2f-Src-2f-demo

