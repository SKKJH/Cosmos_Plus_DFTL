################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/coresightps_dcc_v1_4/src/xcoresightpsdcc.c 

OBJS += \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/coresightps_dcc_v1_4/src/xcoresightpsdcc.o 

C_DEPS += \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/coresightps_dcc_v1_4/src/xcoresightpsdcc.d 


# Each subdirectory must supply rules for building sources it contributes
src/target/cosmos_plus/ps7_cortexa9_0/libsrc/coresightps_dcc_v1_4/src/xcoresightpsdcc.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/coresightps_dcc_v1_4/src/xcoresightpsdcc.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../dftl_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


