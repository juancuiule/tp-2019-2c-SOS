################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../SUSE.c \
../configuracion.c \
../dispatcher.c \
../semaforos.c \
../utils.c 

OBJS += \
./SUSE.o \
./configuracion.o \
./dispatcher.o \
./semaforos.o \
./utils.o 

C_DEPS += \
./SUSE.d \
./configuracion.d \
./dispatcher.d \
./semaforos.d \
./utils.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


