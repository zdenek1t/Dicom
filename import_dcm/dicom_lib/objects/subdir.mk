################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../dicom_lib/objects/dcm.c \
../dicom_lib/objects/dcm1.c \
../dicom_lib/objects/dcmcond.c \
../dicom_lib/objects/dcmdict.c \
../dicom_lib/objects/dcmsupport.c 

OBJS += \
./dicom_lib/objects/dcm.o \
./dicom_lib/objects/dcm1.o \
./dicom_lib/objects/dcmcond.o \
./dicom_lib/objects/dcmdict.o \
./dicom_lib/objects/dcmsupport.o 

C_DEPS += \
./dicom_lib/objects/dcm.d \
./dicom_lib/objects/dcm1.d \
./dicom_lib/objects/dcmcond.d \
./dicom_lib/objects/dcmdict.d \
./dicom_lib/objects/dcmsupport.d 


# Each subdirectory must supply rules for building sources it contributes
dicom_lib/objects/%.o: ../dicom_lib/objects/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DINTSIZE=32 -DLONGSIZE=64 -DSHORTSIZE=16 -DDEBUG -DLINUX -DSHARED_MEMORY -DSEMAPHORE -DPSQL -DTIMEOFDAYARGS=2 -DUSEREGCOMP -DLITTLE_ENDIAN_ARCHITECTURE -I/usr/include/pgsql -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


