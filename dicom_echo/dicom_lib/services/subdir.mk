################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../dicom_lib/services/cmd_valid.c \
../dicom_lib/services/find.c \
../dicom_lib/services/get.c \
../dicom_lib/services/move.c \
../dicom_lib/services/naction.c \
../dicom_lib/services/ncreate.c \
../dicom_lib/services/ndelete.c \
../dicom_lib/services/neventreport.c \
../dicom_lib/services/nget.c \
../dicom_lib/services/nset.c \
../dicom_lib/services/private.c \
../dicom_lib/services/send.c \
../dicom_lib/services/srv1.c \
../dicom_lib/services/srv2.c \
../dicom_lib/services/srvcond.c \
../dicom_lib/services/storage.c \
../dicom_lib/services/verify.c 

OBJS += \
./dicom_lib/services/cmd_valid.o \
./dicom_lib/services/find.o \
./dicom_lib/services/get.o \
./dicom_lib/services/move.o \
./dicom_lib/services/naction.o \
./dicom_lib/services/ncreate.o \
./dicom_lib/services/ndelete.o \
./dicom_lib/services/neventreport.o \
./dicom_lib/services/nget.o \
./dicom_lib/services/nset.o \
./dicom_lib/services/private.o \
./dicom_lib/services/send.o \
./dicom_lib/services/srv1.o \
./dicom_lib/services/srv2.o \
./dicom_lib/services/srvcond.o \
./dicom_lib/services/storage.o \
./dicom_lib/services/verify.o 

C_DEPS += \
./dicom_lib/services/cmd_valid.d \
./dicom_lib/services/find.d \
./dicom_lib/services/get.d \
./dicom_lib/services/move.d \
./dicom_lib/services/naction.d \
./dicom_lib/services/ncreate.d \
./dicom_lib/services/ndelete.d \
./dicom_lib/services/neventreport.d \
./dicom_lib/services/nget.d \
./dicom_lib/services/nset.d \
./dicom_lib/services/private.d \
./dicom_lib/services/send.d \
./dicom_lib/services/srv1.d \
./dicom_lib/services/srv2.d \
./dicom_lib/services/srvcond.d \
./dicom_lib/services/storage.d \
./dicom_lib/services/verify.d 


# Each subdirectory must supply rules for building sources it contributes
dicom_lib/services/%.o: ../dicom_lib/services/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C Compiler'
	gcc -DINTSIZE=32 -DLONGSIZE=32 -DSHORTSIZE=16 -DDEBUG -DLINUX -DSHARED_MEMORY -DSEMAPHORE -DPSQL -DTIMEOFDAYARGS=2 -DUSEREGCOMP -DLITTLE_ENDIAN_ARCHITECTURE -I/usr/include -I/usr/local/include -I/usr/include/pgsql -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


