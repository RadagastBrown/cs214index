################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../index/index.c \
../index/sorted-list.c \
../index/tokenizer.c 

OBJS += \
./index/index.o \
./index/sorted-list.o \
./index/tokenizer.o 

C_DEPS += \
./index/index.d \
./index/sorted-list.d \
./index/tokenizer.d 


# Each subdirectory must supply rules for building sources it contributes
index/%.o: ../index/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


