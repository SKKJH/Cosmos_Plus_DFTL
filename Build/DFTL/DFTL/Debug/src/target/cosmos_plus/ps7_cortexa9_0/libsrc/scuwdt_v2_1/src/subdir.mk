################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/scuwdt_v2_1/src/xscuwdt.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/scuwdt_v2_1/src/xscuwdt_g.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/scuwdt_v2_1/src/xscuwdt_selftest.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/scuwdt_v2_1/src/xscuwdt_sinit.c 

OBJS += \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/scuwdt_v2_1/src/xscuwdt.o \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/scuwdt_v2_1/src/xscuwdt_g.o \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/scuwdt_v2_1/src/xscuwdt_selftest.o \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/scuwdt_v2_1/src/xscuwdt_sinit.o 

C_DEPS += \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/scuwdt_v2_1/src/xscuwdt.d \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/scuwdt_v2_1/src/xscuwdt_g.d \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/scuwdt_v2_1/src/xscuwdt_selftest.d \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/scuwdt_v2_1/src/xscuwdt_sinit.d 


# Each subdirectory must supply rules for building sources it contributes
src/target/cosmos_plus/ps7_cortexa9_0/libsrc/scuwdt_v2_1/src/xscuwdt.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/scuwdt_v2_1/src/xscuwdt.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../DFTL_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/target/cosmos_plus/ps7_cortexa9_0/libsrc/scuwdt_v2_1/src/xscuwdt_g.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/scuwdt_v2_1/src/xscuwdt_g.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../DFTL_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/target/cosmos_plus/ps7_cortexa9_0/libsrc/scuwdt_v2_1/src/xscuwdt_selftest.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/scuwdt_v2_1/src/xscuwdt_selftest.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../DFTL_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/target/cosmos_plus/ps7_cortexa9_0/libsrc/scuwdt_v2_1/src/xscuwdt_sinit.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/scuwdt_v2_1/src/xscuwdt_sinit.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../DFTL_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


