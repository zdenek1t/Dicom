################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../dicom_lib/dulprotocol/dulcond.c \
../dicom_lib/dulprotocol/dulconstruct.c \
../dicom_lib/dulprotocol/dulfsm.c \
../dicom_lib/dulprotocol/dulparse.c \
../dicom_lib/dulprotocol/dulpresent.c \
../dicom_lib/dulprotocol/dulprotocol.c \
../dicom_lib/dulprotocol/dulsnoop.c 

OBJS += \
./dicom_lib/dulprotocol/dulcond.o \
./dicom_lib/dulprotocol/dulconstruct.o \
./dicom_lib/dulprotocol/dulfsm.o \
./dicom_lib/dulprotocol/dulparse.o \
./dicom_lib/dulprotocol/dulpresent.o \
./dicom_lib/dulprotocol/dulprotocol.o \
./dicom_lib/dulprotocol/dulsnoop.o 

C_DEPS += \
./dicom_lib/dulprotocol/dulcond.d \
./dicom_lib/dulprotocol/dulconstruct.d \
./dicom_lib/dulprotocol/dulfsm.d \
./dicom_lib/dulprotocol/dulparse.d \
./dicom_lib/dulprotocol/dulpresent.d \
./dicom_lib/dulprotocol/dulprotocol.d \
./dicom_lib/dulprotocol/dulsnoop.d 


# Each subdirectory must supply rules for building sources it contributes
dicom_lib/dulprotocol/%.o: ../dicom_lib/dulprotocol/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DINTSIZE=32 -DLONGSIZE=32 -DSHORTSIZE=16 -DDEBUG -DLINUX -DSHARED_MEMORY -DSEMAPHORE -DPSQL -DTIMEOFDAYARGS=2 -DUSEREGCOMP -DLITTLE_ENDIAN_ARCHITECTURE -DCTN_NO_RUNT_PDVS -I/usr/include/pgsql -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


