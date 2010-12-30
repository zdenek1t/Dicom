################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../dicom_lib/manage/control.c \
../dicom_lib/manage/delete.c \
../dicom_lib/manage/insert.c \
../dicom_lib/manage/mancond.c \
../dicom_lib/manage/select.c \
../dicom_lib/manage/set.c 

OBJS += \
./dicom_lib/manage/control.o \
./dicom_lib/manage/delete.o \
./dicom_lib/manage/insert.o \
./dicom_lib/manage/mancond.o \
./dicom_lib/manage/select.o \
./dicom_lib/manage/set.o 

C_DEPS += \
./dicom_lib/manage/control.d \
./dicom_lib/manage/delete.d \
./dicom_lib/manage/insert.d \
./dicom_lib/manage/mancond.d \
./dicom_lib/manage/select.d \
./dicom_lib/manage/set.d 


# Each subdirectory must supply rules for building sources it contributes
dicom_lib/manage/%.o: ../dicom_lib/manage/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DINTSIZE=32 -DLONGSIZE=64 -DSHORTSIZE=16 -DDEBUG -DLINUX -DSHARED_MEMORY -DSEMAPHORE -DPSQL -DTIMEOFDAYARGS=2 -DUSEREGCOMP -DLITTLE_ENDIAN_ARCHITECTURE -I/usr/include/pgsql -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


