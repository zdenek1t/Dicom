################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../dicom_lib/tbl/tbl_psql.c \
../dicom_lib/tbl/tblcond.c 

OBJS += \
./dicom_lib/tbl/tbl_psql.o \
./dicom_lib/tbl/tblcond.o 

C_DEPS += \
./dicom_lib/tbl/tbl_psql.d \
./dicom_lib/tbl/tblcond.d 


# Each subdirectory must supply rules for building sources it contributes
dicom_lib/tbl/%.o: ../dicom_lib/tbl/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DINTSIZE=32 -DLONGSIZE=32 -DSHORTSIZE=16 -DDEBUG -DLINUX -DSHARED_MEMORY -DSEMAPHORE -DPSQL -DTIMEOFDAYARGS=2 -DUSEREGCOMP -DLITTLE_ENDIAN_ARCHITECTURE -DCTN_NO_RUNT_PDVS -I/usr/include/pgsql -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


