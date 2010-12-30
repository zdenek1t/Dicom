################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../dicom_lib/messages/dump.c \
../dicom_lib/messages/messages.c \
../dicom_lib/messages/msgcond.c \
../dicom_lib/messages/ref_item.c 

OBJS += \
./dicom_lib/messages/dump.o \
./dicom_lib/messages/messages.o \
./dicom_lib/messages/msgcond.o \
./dicom_lib/messages/ref_item.o 

C_DEPS += \
./dicom_lib/messages/dump.d \
./dicom_lib/messages/messages.d \
./dicom_lib/messages/msgcond.d \
./dicom_lib/messages/ref_item.d 


# Each subdirectory must supply rules for building sources it contributes
dicom_lib/messages/%.o: ../dicom_lib/messages/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DINTSIZE=32 -DLONGSIZE=32 -DSHORTSIZE=16 -DDEBUG -DLINUX -DSHARED_MEMORY -DSEMAPHORE -DPSQL -DTIMEOFDAYARGS=2 -DUSEREGCOMP -DLITTLE_ENDIAN_ARCHITECTURE -DCTN_NO_RUNT_PDVS -I/usr/include/pgsql -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


