################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Drivers/STM32F4xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/timers/subdir.mk
-include Core/Src/spim/subdir.mk
-include Core/Src/rtc/subdir.mk
-include Core/Src/gpio/subdir.mk
-include Core/Src/gc9a01a/subdir.mk
-include Core/Src/display/fonts/subdir.mk
-include Core/Src/display/subdir.mk
-include Core/Src/demo/subdir.mk
-include Core/Src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := STM32F407VET6_GC9A01_Clock
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
STM32F407VET6_GC9A01_Clock.elf \

MAP_FILES += \
STM32F407VET6_GC9A01_Clock.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
STM32F407VET6_GC9A01_Clock.list \

OBJCOPY_HEX += \
STM32F407VET6_GC9A01_Clock.hex \

OBJCOPY_BIN += \
STM32F407VET6_GC9A01_Clock.bin \


# All Target
all: main-build

# Main-build Target
main-build: STM32F407VET6_GC9A01_Clock.elf secondary-outputs

# Tool invocations
STM32F407VET6_GC9A01_Clock.elf STM32F407VET6_GC9A01_Clock.map: $(OBJS) $(USER_OBJS) C:\Users\Zver\Downloads\ST.COM\STM32F407VET6_GC9A01_Clock\STM32F407VETX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "STM32F407VET6_GC9A01_Clock.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"C:\Users\Zver\Downloads\ST.COM\STM32F407VET6_GC9A01_Clock\STM32F407VETX_FLASH.ld" --specs=nosys.specs -Wl,-Map="STM32F407VET6_GC9A01_Clock.map" -Wl,--gc-sections -static -u _printf_float -u _scanf_float --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

STM32F407VET6_GC9A01_Clock.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "STM32F407VET6_GC9A01_Clock.list"
	@echo 'Finished building: $@'
	@echo ' '

STM32F407VET6_GC9A01_Clock.hex: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "STM32F407VET6_GC9A01_Clock.hex"
	@echo 'Finished building: $@'
	@echo ' '

STM32F407VET6_GC9A01_Clock.bin: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "STM32F407VET6_GC9A01_Clock.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) STM32F407VET6_GC9A01_Clock.bin STM32F407VET6_GC9A01_Clock.elf STM32F407VET6_GC9A01_Clock.hex STM32F407VET6_GC9A01_Clock.list STM32F407VET6_GC9A01_Clock.map default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX) $(OBJCOPY_BIN)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
