################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/cli_op.c \
../src/confg.c \
../src/main.c \
../src/utils.c 

OBJS += \
./src/cli_op.o \
./src/confg.o \
./src/main.o \
./src/utils.o 

C_DEPS += \
./src/cli_op.d \
./src/confg.d \
./src/main.d \
./src/utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2019-2c-SOS/common" -O0 -g3 -Wall -c -fmessage-length=0 -DFUSE_USE_VERSION=27 -D_FILE_OFFSET_BITS=64 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


