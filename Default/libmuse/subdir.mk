################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libmuse/client.c \
../libmuse/libmuse.c \
../libmuse/network.c \
../libmuse/utils.c 

OBJS += \
./libmuse/client.o \
./libmuse/libmuse.o \
./libmuse/network.o \
./libmuse/utils.o 

C_DEPS += \
./libmuse/client.d \
./libmuse/libmuse.d \
./libmuse/network.d \
./libmuse/utils.d 


# Each subdirectory must supply rules for building sources it contributes
libmuse/%.o: ../libmuse/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


