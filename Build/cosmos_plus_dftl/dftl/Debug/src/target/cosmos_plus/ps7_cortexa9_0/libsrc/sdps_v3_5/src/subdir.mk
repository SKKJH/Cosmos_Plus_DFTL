################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/sdps_v3_5/src/xsdps.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/sdps_v3_5/src/xsdps_g.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/sdps_v3_5/src/xsdps_options.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/sdps_v3_5/src/xsdps_sinit.c 

OBJS += \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/sdps_v3_5/src/xsdps.o \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/sdps_v3_5/src/xsdps_g.o \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/sdps_v3_5/src/xsdps_options.o \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/sdps_v3_5/src/xsdps_sinit.o 

C_DEPS += \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/sdps_v3_5/src/xsdps.d \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/sdps_v3_5/src/xsdps_g.d \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/sdps_v3_5/src/xsdps_options.d \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/sdps_v3_5/src/xsdps_sinit.d 


# Each subdirectory must supply rules for building sources it contributes
src/target/cosmos_plus/ps7_cortexa9_0/libsrc/sdps_v3_5/src/xsdps.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/sdps_v3_5/src/xsdps.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../dftl_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/target/cosmos_plus/ps7_cortexa9_0/libsrc/sdps_v3_5/src/xsdps_g.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/sdps_v3_5/src/xsdps_g.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../dftl_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/target/cosmos_plus/ps7_cortexa9_0/libsrc/sdps_v3_5/src/xsdps_options.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/sdps_v3_5/src/xsdps_options.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../dftl_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/target/cosmos_plus/ps7_cortexa9_0/libsrc/sdps_v3_5/src/xsdps_sinit.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/sdps_v3_5/src/xsdps_sinit.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../dftl_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


