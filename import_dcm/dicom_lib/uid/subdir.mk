################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../dicom_lib/uid/uid.c \
../dicom_lib/uid/uidcond.c 

OBJS += \
./dicom_lib/uid/uid.o \
./dicom_lib/uid/uidcond.o 

C_DEPS += \
./dicom_lib/uid/uid.d \
./dicom_lib/uid/uidcond.d 


# Each subdirectory must supply rules for building sources it contributes
dicom_lib/uid/%.o: ../dicom_lib/uid/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DINTSIZE=32 -DLONGSIZE=64 -DSHORTSIZE=16 -DDEBUG -DLINUX -DSHARED_MEMORY -DSEMAPHORE -DPSQL -DTIMEOFDAYARGS=2 -DUSEREGCOMP -DLITTLE_ENDIAN_ARCHITECTURE -I/usr/include/pgsql -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


