################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../apps_send_image/send_image.c 

OBJS += \
./apps_send_image/send_image.o 

C_DEPS += \
./apps_send_image/send_image.d 


# Each subdirectory must supply rules for building sources it contributes
apps_send_image/%.o: ../apps_send_image/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DINTSIZE=32 -DLONGSIZE=32 -DSHORTSIZE=16 -DDEBUG -DLINUX -DSHARED_MEMORY -DSEMAPHORE -DPSQL -DTIMEOFDAYARGS=2 -DUSEREGCOMP -DLITTLE_ENDIAN_ARCHITECTURE -DCTN_NO_RUNT_PDVS -I/usr/include/pgsql -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


