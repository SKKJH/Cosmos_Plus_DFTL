################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_activeblock.cpp \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_block.cpp \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_bufferpool.cpp \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_external_interface.cpp \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_garbagecollector.cpp \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_global.cpp \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_hdma.cpp \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_meta.cpp \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_profile.cpp \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_request.cpp \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_request_gc.cpp \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_request_hil.cpp \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_request_meta.cpp \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_striping.cpp \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_vnand.cpp 

OBJS += \
./src/ftl/ioub/ioub_activeblock.o \
./src/ftl/ioub/ioub_block.o \
./src/ftl/ioub/ioub_bufferpool.o \
./src/ftl/ioub/ioub_external_interface.o \
./src/ftl/ioub/ioub_garbagecollector.o \
./src/ftl/ioub/ioub_global.o \
./src/ftl/ioub/ioub_hdma.o \
./src/ftl/ioub/ioub_meta.o \
./src/ftl/ioub/ioub_profile.o \
./src/ftl/ioub/ioub_request.o \
./src/ftl/ioub/ioub_request_gc.o \
./src/ftl/ioub/ioub_request_hil.o \
./src/ftl/ioub/ioub_request_meta.o \
./src/ftl/ioub/ioub_striping.o \
./src/ftl/ioub/ioub_vnand.o 

CPP_DEPS += \
./src/ftl/ioub/ioub_activeblock.d \
./src/ftl/ioub/ioub_block.d \
./src/ftl/ioub/ioub_bufferpool.d \
./src/ftl/ioub/ioub_external_interface.d \
./src/ftl/ioub/ioub_garbagecollector.d \
./src/ftl/ioub/ioub_global.d \
./src/ftl/ioub/ioub_hdma.d \
./src/ftl/ioub/ioub_meta.d \
./src/ftl/ioub/ioub_profile.d \
./src/ftl/ioub/ioub_request.d \
./src/ftl/ioub/ioub_request_gc.d \
./src/ftl/ioub/ioub_request_hil.d \
./src/ftl/ioub/ioub_request_meta.d \
./src/ftl/ioub/ioub_striping.d \
./src/ftl/ioub/ioub_vnand.d 


# Each subdirectory must supply rules for building sources it contributes
src/ftl/ioub/ioub_activeblock.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_activeblock.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../dftl_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/ioub/ioub_block.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_block.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../dftl_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/ioub/ioub_bufferpool.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_bufferpool.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../dftl_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/ioub/ioub_external_interface.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_external_interface.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../dftl_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/ioub/ioub_garbagecollector.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_garbagecollector.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../dftl_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/ioub/ioub_global.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_global.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../dftl_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/ioub/ioub_hdma.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_hdma.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../dftl_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/ioub/ioub_meta.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_meta.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../dftl_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/ioub/ioub_profile.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_profile.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../dftl_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/ioub/ioub_request.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_request.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../dftl_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/ioub/ioub_request_gc.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_request_gc.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../dftl_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/ioub/ioub_request_hil.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_request_hil.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../dftl_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/ioub/ioub_request_meta.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_request_meta.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../dftl_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/ioub/ioub_striping.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_striping.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../dftl_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/ioub/ioub_vnand.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/ioub/ioub_vnand.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../dftl_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


