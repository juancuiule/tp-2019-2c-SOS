################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/filesystem.c \
../src/main.c \
../src/sac_op.c \
../src/server.c \
../src/utils.c 

OBJS += \
./src/filesystem.o \
./src/main.o \
./src/sac_op.o \
./src/server.o \
./src/utils.o 

C_DEPS += \
./src/filesystem.d \
./src/main.d \
./src/sac_op.d \
./src/server.d \
./src/utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2019-2c-SOS/common" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


