################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
LD_SRCS += \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/GreedyFTL-3.0.0/lscript.ld 

C_SRCS += \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/GreedyFTL-3.0.0/address_translation.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/GreedyFTL-3.0.0/data_buffer.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/GreedyFTL-3.0.0/ftl_config.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/GreedyFTL-3.0.0/garbage_collection.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/GreedyFTL-3.0.0/request_allocation.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/GreedyFTL-3.0.0/request_schedule.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/GreedyFTL-3.0.0/request_transform.c 

OBJS += \
./src/ftl/GreedyFTL-3.0.0/address_translation.o \
./src/ftl/GreedyFTL-3.0.0/data_buffer.o \
./src/ftl/GreedyFTL-3.0.0/ftl_config.o \
./src/ftl/GreedyFTL-3.0.0/garbage_collection.o \
./src/ftl/GreedyFTL-3.0.0/request_allocation.o \
./src/ftl/GreedyFTL-3.0.0/request_schedule.o \
./src/ftl/GreedyFTL-3.0.0/request_transform.o 

C_DEPS += \
./src/ftl/GreedyFTL-3.0.0/address_translation.d \
./src/ftl/GreedyFTL-3.0.0/data_buffer.d \
./src/ftl/GreedyFTL-3.0.0/ftl_config.d \
./src/ftl/GreedyFTL-3.0.0/garbage_collection.d \
./src/ftl/GreedyFTL-3.0.0/request_allocation.d \
./src/ftl/GreedyFTL-3.0.0/request_schedule.d \
./src/ftl/GreedyFTL-3.0.0/request_transform.d 


# Each subdirectory must supply rules for building sources it contributes
src/ftl/GreedyFTL-3.0.0/address_translation.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/GreedyFTL-3.0.0/address_translation.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../DFTL_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/GreedyFTL-3.0.0/data_buffer.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/GreedyFTL-3.0.0/data_buffer.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../DFTL_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/GreedyFTL-3.0.0/ftl_config.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/GreedyFTL-3.0.0/ftl_config.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../DFTL_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/GreedyFTL-3.0.0/garbage_collection.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/GreedyFTL-3.0.0/garbage_collection.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../DFTL_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/GreedyFTL-3.0.0/request_allocation.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/GreedyFTL-3.0.0/request_allocation.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../DFTL_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/GreedyFTL-3.0.0/request_schedule.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/GreedyFTL-3.0.0/request_schedule.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../DFTL_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ftl/GreedyFTL-3.0.0/request_transform.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/ftl/GreedyFTL-3.0.0/request_transform.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../DFTL_bsp/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


