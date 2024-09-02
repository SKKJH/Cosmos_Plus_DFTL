################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/dmaps_v2_3/src/xdmaps.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/dmaps_v2_3/src/xdmaps_g.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/dmaps_v2_3/src/xdmaps_hw.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/dmaps_v2_3/src/xdmaps_selftest.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/dmaps_v2_3/src/xdmaps_sinit.c 

OBJS += \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/dmaps_v2_3/src/xdmaps.o \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/dmaps_v2_3/src/xdmaps_g.o \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/dmaps_v2_3/src/xdmaps_hw.o \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/dmaps_v2_3/src/xdmaps_selftest.o \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/dmaps_v2_3/src/xdmaps_sinit.o 

C_DEPS += \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/dmaps_v2_3/src/xdmaps.d \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/dmaps_v2_3/src/xdmaps_g.d \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/dmaps_v2_3/src/xdmaps_hw.d \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/dmaps_v2_3/src/xdmaps_selftest.d \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/dmaps_v2_3/src/xdmaps_sinit.d 


# Each subdirectory must supply rules for building sources it contributes
src/target/cosmos_plus/ps7_cortexa9_0/libsrc/dmaps_v2_3/src/xdmaps.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/dmaps_v2_3/src/xdmaps.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../DFTL_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/target/cosmos_plus/ps7_cortexa9_0/libsrc/dmaps_v2_3/src/xdmaps_g.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/dmaps_v2_3/src/xdmaps_g.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../DFTL_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/target/cosmos_plus/ps7_cortexa9_0/libsrc/dmaps_v2_3/src/xdmaps_hw.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/dmaps_v2_3/src/xdmaps_hw.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../DFTL_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/target/cosmos_plus/ps7_cortexa9_0/libsrc/dmaps_v2_3/src/xdmaps_selftest.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/dmaps_v2_3/src/xdmaps_selftest.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../DFTL_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/target/cosmos_plus/ps7_cortexa9_0/libsrc/dmaps_v2_3/src/xdmaps_sinit.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/dmaps_v2_3/src/xdmaps_sinit.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../DFTL_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


