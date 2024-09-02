################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps_endpoint.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps_g.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps_hw.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps_intr.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps_sinit.c 

OBJS += \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps.o \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps_endpoint.o \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps_g.o \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps_hw.o \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps_intr.o \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps_sinit.o 

C_DEPS += \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps.d \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps_endpoint.d \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps_g.d \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps_hw.d \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps_intr.d \
./src/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps_sinit.d 


# Each subdirectory must supply rules for building sources it contributes
src/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../dftl_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps_endpoint.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps_endpoint.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../dftl_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps_g.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps_g.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../dftl_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps_hw.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps_hw.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../dftl_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps_intr.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps_intr.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../dftl_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps_sinit.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/target/cosmos_plus/ps7_cortexa9_0/libsrc/usbps_v2_4/src/xusbps_sinit.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../dftl_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


