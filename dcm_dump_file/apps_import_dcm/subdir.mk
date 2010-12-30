################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../apps_import_dcm/import_dcm.c 

OBJS += \
./apps_import_dcm/import_dcm.o 

C_DEPS += \
./apps_import_dcm/import_dcm.d 


# Each subdirectory must supply rules for building sources it contributes
apps_import_dcm/%.o: ../apps_import_dcm/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DINTSIZE=32 -DLONGSIZE=32 -DSHORTSIZE=16 -DDEBUG -DLINUX -DSHARED_MEMORY -DSEMAPHORE -DUSLEEP -DPSQL -DTIMEOFDAYARGS=2 -DUSEREGCOMP -D_SEM_SEMUN_UNDEFINED -DLITTLE_ENDIAN_ARCHITECTURE -I"D:\cygwin\usr\include" -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


