################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../app/src/appSX1272.c \
../app/src/main.c \
../app/src/printf-stdarg.c \
../app/src/stm32f0xx_it.c 

OBJS += \
./app/src/appSX1272.o \
./app/src/main.o \
./app/src/printf-stdarg.o \
./app/src/stm32f0xx_it.o 

C_DEPS += \
./app/src/appSX1272.d \
./app/src/main.d \
./app/src/printf-stdarg.d \
./app/src/stm32f0xx_it.d 


# Each subdirectory must supply rules for building sources it contributes
app/src/%.o: ../app/src/%.c app/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DSTM32F072xB -c -I"C:/Users/hmnz/source/repos/SACS/STM32/app/inc" -I"C:/Users/hmnz/source/repos/SACS/STM32/cmsis/core" -I"C:/Users/hmnz/source/repos/SACS/STM32/cmsis/device/inc" -I"C:/Users/hmnz/source/repos/SACS/STM32/bsp/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-app-2f-src

clean-app-2f-src:
	-$(RM) ./app/src/appSX1272.d ./app/src/appSX1272.o ./app/src/main.d ./app/src/main.o ./app/src/printf-stdarg.d ./app/src/printf-stdarg.o ./app/src/stm32f0xx_it.d ./app/src/stm32f0xx_it.o

.PHONY: clean-app-2f-src

