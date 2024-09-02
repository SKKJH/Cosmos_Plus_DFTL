################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/win/cosmos_win.c 

OBJS += \
./src/target/win/cosmos_win.o 

C_DEPS += \
./src/target/win/cosmos_win.d 


# Each subdirectory must supply rules for building sources it contributes
src/target/win/cosmos_win.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/win/cosmos_win.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../dftl_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


