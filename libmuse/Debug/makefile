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
all: liblibmuse.so

dependents:
	-cd /home/utnso/Documentos/common/Debug && $(MAKE) all

# Tool invocations
liblibmuse.so: $(OBJS) $(USER_OBJS) /home/utnso/Documentos/common/Debug/libcommon.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -shared -o "liblibmuse.so" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(LIBRARIES)$(OBJS)$(C_DEPS) liblibmuse.so
	-@echo ' '

.PHONY: all clean dependents
/home/utnso/Documentos/common/Debug/libcommon.so:

-include ../makefile.targets
