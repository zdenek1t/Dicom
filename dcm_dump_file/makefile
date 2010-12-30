################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include dicom_lib/utility/subdir.mk
-include dicom_lib/uid/subdir.mk
-include dicom_lib/thread/subdir.mk
-include dicom_lib/snp/subdir.mk
-include dicom_lib/objects/subdir.mk
-include dicom_lib/messages/subdir.mk
-include dicom_lib/lst/subdir.mk
-include dicom_lib/info_entity/subdir.mk
-include dicom_lib/condition/subdir.mk
-include apps_dcm_dump_file/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: dcm_dump_file

# Tool invocations
dcm_dump_file: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"D:\cygwin\lib" -L../lib64 -L/usr/lib/pgsql/ -o"dcm_dump_file" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) dcm_dump_file
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
