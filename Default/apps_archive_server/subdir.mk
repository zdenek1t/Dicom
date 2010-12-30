################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../apps_archive_server/archive_server.c \
../apps_archive_server/association.c \
../apps_archive_server/cget.c \
../apps_archive_server/find.c \
../apps_archive_server/move.c \
../apps_archive_server/parse.c \
../apps_archive_server/queue.c \
../apps_archive_server/requests.c \
../apps_archive_server/sscond.c 

OBJS += \
./apps_archive_server/archive_server.o \
./apps_archive_server/association.o \
./apps_archive_server/cget.o \
./apps_archive_server/find.o \
./apps_archive_server/move.o \
./apps_archive_server/parse.o \
./apps_archive_server/queue.o \
./apps_archive_server/requests.o \
./apps_archive_server/sscond.o 

C_DEPS += \
./apps_archive_server/archive_server.d \
./apps_archive_server/association.d \
./apps_archive_server/cget.d \
./apps_archive_server/find.d \
./apps_archive_server/move.d \
./apps_archive_server/parse.d \
./apps_archive_server/queue.d \
./apps_archive_server/requests.d \
./apps_archive_server/sscond.d 


# Each subdirectory must supply rules for building sources it contributes
apps_archive_server/%.o: ../apps_archive_server/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DINTSIZE=32 -DLONGSIZE=32 -DSHORTSIZE=16 -DDEBUG -DLINUX -DSHARED_MEMORY -DSEMAPHORE -DPSQL -DTIMEOFDAYARGS=2 -DUSEREGCOMP -DLITTLE_ENDIAN_ARCHITECTURE -DCTN_NO_RUNT_PDVS -I/usr/include/pgsql -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


