################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/dftl/dftl_activeblock.cpp \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/dftl/dftl_block.cpp \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/dftl/dftl_bufferpool.cpp \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/dftl/dftl_external_interface.cpp \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/dftl/dftl_garbagecollector.cpp \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/dftl/dftl_global.cpp \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/dftl/dftl_hdma.cpp \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/dftl/dftl_meta.cpp \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/dftl/dftl_profile.cpp \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/dftl/dftl_request.cpp \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/dftl/dftl_request_gc.cpp \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/dftl/dftl_request_hil.cpp \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/dftl/dftl_request_meta.cpp \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/dftl/dftl_vnand.cpp 

OBJS += \
./src/ftl/dftl/dftl_activeblock.o \
./src/ftl/dftl/dftl_block.o \
./src/ftl/dftl/dftl_bufferpool.o \
./src/ftl/dftl/dftl_external_interface.o \
./src/ftl/dftl/dftl_garbagecollector.o \
./src/ftl/dftl/dftl_global.o \
./src/ftl/dftl/dftl_hdma.o \
./src/ftl/dftl/dftl_meta.o \
./src/ftl/dftl/dftl_profile.o \
./src/ftl/dftl/dftl_request.o \
./src/ftl/dftl/dftl_request_gc.o \
./src/ftl/dftl/dftl_request_hil.o \
./src/ftl/dftl/dftl_request_meta.o \
./src/ftl/dftl/dftl_vnand.o 

CPP_DEPS += \
./src/ftl/dftl/dftl_activeblock.d \
./src/ftl/dftl/dftl_block.d \
./src/ftl/dftl/dftl_bufferpool.d \
./src/ftl/dftl/dftl_external_interface.d \
./src/ftl/dftl/dftl_garbagecollector.d \
./src/ftl/dftl/dftl_global.d \
./src/ftl/dftl/dftl_hdma.d \
./src/ftl/dftl/dftl_meta.d \
./src/ftl/dftl/dftl_profile.d \
./src/ftl/dftl/dftl_request.d \
./src/ftl/dftl/dftl_request_gc.d \
./src/ftl/dftl/dftl_request_hil.d \
./src/ftl/dftl/dftl_request_meta.d \
./src/ftl/dftl/dftl_vnand.d 


# Each subdirectory must supply rules for building sources it contributes
src/ftl/dftl/dftl_activeblock.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/dftl/dftl_activeblock.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -DCOSMOS_PLUS=1 -DDFTL=1 -Wall -O0 -g3 -I../../dftl_bsp/ps7_cortexa9_0/include -I../../../../common -I../../../../fil -I../../../../ftl -I../../../../ftl/dftl -I../../../../hil -I../../../../hil/nvme -I../../../../target -I../../../../target/cosmos_plus -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/dftl/dftl_block.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/dftl/dftl_block.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -DCOSMOS_PLUS=1 -DDFTL=1 -Wall -O0 -g3 -I../../dftl_bsp/ps7_cortexa9_0/include -I../../../../common -I../../../../fil -I../../../../ftl -I../../../../ftl/dftl -I../../../../hil -I../../../../hil/nvme -I../../../../target -I../../../../target/cosmos_plus -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/dftl/dftl_bufferpool.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/dftl/dftl_bufferpool.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -DCOSMOS_PLUS=1 -DDFTL=1 -Wall -O0 -g3 -I../../dftl_bsp/ps7_cortexa9_0/include -I../../../../common -I../../../../fil -I../../../../ftl -I../../../../ftl/dftl -I../../../../hil -I../../../../hil/nvme -I../../../../target -I../../../../target/cosmos_plus -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/dftl/dftl_external_interface.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/dftl/dftl_external_interface.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -DCOSMOS_PLUS=1 -DDFTL=1 -Wall -O0 -g3 -I../../dftl_bsp/ps7_cortexa9_0/include -I../../../../common -I../../../../fil -I../../../../ftl -I../../../../ftl/dftl -I../../../../hil -I../../../../hil/nvme -I../../../../target -I../../../../target/cosmos_plus -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/dftl/dftl_garbagecollector.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/dftl/dftl_garbagecollector.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -DCOSMOS_PLUS=1 -DDFTL=1 -Wall -O0 -g3 -I../../dftl_bsp/ps7_cortexa9_0/include -I../../../../common -I../../../../fil -I../../../../ftl -I../../../../ftl/dftl -I../../../../hil -I../../../../hil/nvme -I../../../../target -I../../../../target/cosmos_plus -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/dftl/dftl_global.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/dftl/dftl_global.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -DCOSMOS_PLUS=1 -DDFTL=1 -Wall -O0 -g3 -I../../dftl_bsp/ps7_cortexa9_0/include -I../../../../common -I../../../../fil -I../../../../ftl -I../../../../ftl/dftl -I../../../../hil -I../../../../hil/nvme -I../../../../target -I../../../../target/cosmos_plus -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/dftl/dftl_hdma.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/dftl/dftl_hdma.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -DCOSMOS_PLUS=1 -DDFTL=1 -Wall -O0 -g3 -I../../dftl_bsp/ps7_cortexa9_0/include -I../../../../common -I../../../../fil -I../../../../ftl -I../../../../ftl/dftl -I../../../../hil -I../../../../hil/nvme -I../../../../target -I../../../../target/cosmos_plus -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/dftl/dftl_meta.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/dftl/dftl_meta.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -DCOSMOS_PLUS=1 -DDFTL=1 -Wall -O0 -g3 -I../../dftl_bsp/ps7_cortexa9_0/include -I../../../../common -I../../../../fil -I../../../../ftl -I../../../../ftl/dftl -I../../../../hil -I../../../../hil/nvme -I../../../../target -I../../../../target/cosmos_plus -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/dftl/dftl_profile.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/dftl/dftl_profile.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -DCOSMOS_PLUS=1 -DDFTL=1 -Wall -O0 -g3 -I../../dftl_bsp/ps7_cortexa9_0/include -I../../../../common -I../../../../fil -I../../../../ftl -I../../../../ftl/dftl -I../../../../hil -I../../../../hil/nvme -I../../../../target -I../../../../target/cosmos_plus -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/dftl/dftl_request.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/dftl/dftl_request.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -DCOSMOS_PLUS=1 -DDFTL=1 -Wall -O0 -g3 -I../../dftl_bsp/ps7_cortexa9_0/include -I../../../../common -I../../../../fil -I../../../../ftl -I../../../../ftl/dftl -I../../../../hil -I../../../../hil/nvme -I../../../../target -I../../../../target/cosmos_plus -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/dftl/dftl_request_gc.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/dftl/dftl_request_gc.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -DCOSMOS_PLUS=1 -DDFTL=1 -Wall -O0 -g3 -I../../dftl_bsp/ps7_cortexa9_0/include -I../../../../common -I../../../../fil -I../../../../ftl -I../../../../ftl/dftl -I../../../../hil -I../../../../hil/nvme -I../../../../target -I../../../../target/cosmos_plus -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/dftl/dftl_request_hil.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/dftl/dftl_request_hil.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -DCOSMOS_PLUS=1 -DDFTL=1 -Wall -O0 -g3 -I../../dftl_bsp/ps7_cortexa9_0/include -I../../../../common -I../../../../fil -I../../../../ftl -I../../../../ftl/dftl -I../../../../hil -I../../../../hil/nvme -I../../../../target -I../../../../target/cosmos_plus -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/dftl/dftl_request_meta.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/dftl/dftl_request_meta.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -DCOSMOS_PLUS=1 -DDFTL=1 -Wall -O0 -g3 -I../../dftl_bsp/ps7_cortexa9_0/include -I../../../../common -I../../../../fil -I../../../../ftl -I../../../../ftl/dftl -I../../../../hil -I../../../../hil/nvme -I../../../../target -I../../../../target/cosmos_plus -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/dftl/dftl_vnand.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/dftl/dftl_vnand.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -DCOSMOS_PLUS=1 -DDFTL=1 -Wall -O0 -g3 -I../../dftl_bsp/ps7_cortexa9_0/include -I../../../../common -I../../../../fil -I../../../../ftl -I../../../../ftl/dftl -I../../../../hil -I../../../../hil/nvme -I../../../../target -I../../../../target/cosmos_plus -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


