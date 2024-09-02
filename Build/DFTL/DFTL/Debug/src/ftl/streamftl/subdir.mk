################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/streamftl/StreamFTL_map.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/streamftl/dump.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/streamftl/error.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/streamftl/init.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/streamftl/streamftl.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/streamftl/streamftl_activeblock.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/streamftl/streamftl_bufferpool.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/streamftl/streamftl_main.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/streamftl/streamftl_request.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/streamftl/streamftl_util.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/streamftl/streamftl_vnand.c 

OBJS += \
./src/ftl/streamftl/StreamFTL_map.o \
./src/ftl/streamftl/dump.o \
./src/ftl/streamftl/error.o \
./src/ftl/streamftl/init.o \
./src/ftl/streamftl/streamftl.o \
./src/ftl/streamftl/streamftl_activeblock.o \
./src/ftl/streamftl/streamftl_bufferpool.o \
./src/ftl/streamftl/streamftl_main.o \
./src/ftl/streamftl/streamftl_request.o \
./src/ftl/streamftl/streamftl_util.o \
./src/ftl/streamftl/streamftl_vnand.o 

C_DEPS += \
./src/ftl/streamftl/StreamFTL_map.d \
./src/ftl/streamftl/dump.d \
./src/ftl/streamftl/error.d \
./src/ftl/streamftl/init.d \
./src/ftl/streamftl/streamftl.d \
./src/ftl/streamftl/streamftl_activeblock.d \
./src/ftl/streamftl/streamftl_bufferpool.d \
./src/ftl/streamftl/streamftl_main.d \
./src/ftl/streamftl/streamftl_request.d \
./src/ftl/streamftl/streamftl_util.d \
./src/ftl/streamftl/streamftl_vnand.d 


# Each subdirectory must supply rules for building sources it contributes
src/ftl/streamftl/StreamFTL_map.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/streamftl/StreamFTL_map.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../DFTL_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/streamftl/dump.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/streamftl/dump.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../DFTL_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/streamftl/error.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/streamftl/error.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../DFTL_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/streamftl/init.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/streamftl/init.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../DFTL_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/streamftl/streamftl.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/streamftl/streamftl.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../DFTL_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/streamftl/streamftl_activeblock.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/streamftl/streamftl_activeblock.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../DFTL_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/streamftl/streamftl_bufferpool.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/streamftl/streamftl_bufferpool.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../DFTL_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/streamftl/streamftl_main.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/streamftl/streamftl_main.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../DFTL_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/streamftl/streamftl_request.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/streamftl/streamftl_request.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../DFTL_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/streamftl/streamftl_util.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/streamftl/streamftl_util.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../DFTL_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/streamftl/streamftl_vnand.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/streamftl/streamftl_vnand.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../DFTL_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


