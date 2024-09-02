################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/common/list.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/common/random.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/common/statistics.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/common/util.c 

OBJS += \
./src/common/list.o \
./src/common/random.o \
./src/common/statistics.o \
./src/common/util.o 

C_DEPS += \
./src/common/list.d \
./src/common/random.d \
./src/common/statistics.d \
./src/common/util.d 


# Each subdirectory must supply rules for building sources it contributes
src/common/list.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/common/list.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -DCOSMOS_PLUS=1 -DDFTL=1 -Wall -O2 -I../../dftl_bsp/ps7_cortexa9_0/include -I../../../../common -I../../../../fil -I../../../../ftl -I../../../../ftl/dftl -I../../../../hil -I../../../../hil/nvme -I../../../../target -I../../../../target/cosmos_plus -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/common/random.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/common/random.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -DCOSMOS_PLUS=1 -DDFTL=1 -Wall -O2 -I../../dftl_bsp/ps7_cortexa9_0/include -I../../../../common -I../../../../fil -I../../../../ftl -I../../../../ftl/dftl -I../../../../hil -I../../../../hil/nvme -I../../../../target -I../../../../target/cosmos_plus -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/common/statistics.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/common/statistics.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -DCOSMOS_PLUS=1 -DDFTL=1 -Wall -O2 -I../../dftl_bsp/ps7_cortexa9_0/include -I../../../../common -I../../../../fil -I../../../../ftl -I../../../../ftl/dftl -I../../../../hil -I../../../../hil/nvme -I../../../../target -I../../../../target/cosmos_plus -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/common/util.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/common/util.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -DCOSMOS_PLUS=1 -DDFTL=1 -Wall -O2 -I../../dftl_bsp/ps7_cortexa9_0/include -I../../../../common -I../../../../fil -I../../../../ftl -I../../../../ftl/dftl -I../../../../hil -I../../../../hil/nvme -I../../../../target -I../../../../target/cosmos_plus -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


