################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../dicom_lib/snp/decode.c \
../dicom_lib/snp/dlroutines.c \
../dicom_lib/snp/snp.c \
../dicom_lib/snp/snpcond.c 

OBJS += \
./dicom_lib/snp/decode.o \
./dicom_lib/snp/dlroutines.o \
./dicom_lib/snp/snp.o \
./dicom_lib/snp/snpcond.o 

C_DEPS += \
./dicom_lib/snp/decode.d \
./dicom_lib/snp/dlroutines.d \
./dicom_lib/snp/snp.d \
./dicom_lib/snp/snpcond.d 


# Each subdirectory must supply rules for building sources it contributes
dicom_lib/snp/%.o: ../dicom_lib/snp/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DINTSIZE=32 -DLONGSIZE=32 -DSHORTSIZE=16 -DDEBUG -DLINUX -DSHARED_MEMORY -DSEMAPHORE -DPSQL -DTIMEOFDAYARGS=2 -DUSEREGCOMP -DLITTLE_ENDIAN_ARCHITECTURE -DCTN_NO_RUNT_PDVS -I/usr/include/pgsql -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


