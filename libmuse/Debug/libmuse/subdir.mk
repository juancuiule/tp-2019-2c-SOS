################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libmuse/libmuse.c \
../libmuse/network.c \
../libmuse/utils.c 

OBJS += \
./libmuse/libmuse.o \
./libmuse/network.o \
./libmuse/utils.o 

C_DEPS += \
./libmuse/libmuse.d \
./libmuse/network.d \
./libmuse/utils.d 


# Each subdirectory must supply rules for building sources it contributes
libmuse/%.o: ../libmuse/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


