################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: liblibSUSE.so

# Tool invocations
liblibSUSE.so: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -shared -o "liblibSUSE.so" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(LIBRARIES)$(OBJS)$(C_DEPS) liblibSUSE.so
	sudo rm -rf /usr/include/libSUSE
	-@echo ' '
	
install: all
	sudo cp liblibSUSE.so /usr/lib
	sudo mkdir /usr/include/libSUSE
	sudo cp ../libSUSE.h /usr/include/libSUSE
	sudo cp ../utils.h /usr/include/libSUSE

.PHONY: all clean dependents

-include ../makefile.targets
