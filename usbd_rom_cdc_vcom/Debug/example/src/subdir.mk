################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../example/src/cdc_desc.c \
../example/src/cdc_main.c \
../example/src/cdc_vcom.c \
../example/src/cr_startup_lpc43xx.c \
../example/src/sysinit.c 

OBJS += \
./example/src/cdc_desc.o \
./example/src/cdc_main.o \
./example/src/cdc_vcom.o \
./example/src/cr_startup_lpc43xx.o \
./example/src/sysinit.o 

C_DEPS += \
./example/src/cdc_desc.d \
./example/src/cdc_main.d \
./example/src/cdc_vcom.d \
./example/src/cr_startup_lpc43xx.d \
./example/src/sysinit.d 


# Each subdirectory must supply rules for building sources it contributes
example/src/%.o: ../example/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DDEBUG -D__CODE_RED -D__USE_LPCOPEN -DCORE_M4 -D__MULTICORE_NONE -DNO_BOARD_LIB -I"C:\Users\a.guerin\Documents\projet\FW scanner\test1\lpc_chip_43xx\inc" -I"C:\Users\a.guerin\Documents\projet\FW scanner\test1\usbd_rom_cdc_vcom\example\inc" -I"C:\Users\a.guerin\Documents\projet\FW scanner\test1\lpc_chip_43xx\inc\usbd" -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fsingle-precision-constant -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


