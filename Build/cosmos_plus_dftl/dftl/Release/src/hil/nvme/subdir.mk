################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/hil/nvme/host_lld.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/hil/nvme/nvme_admin_cmd.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/hil/nvme/nvme_identify.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/hil/nvme/nvme_io_cmd.c \
C:/Users/User/Desktop/Cosmos_SSD-IOUB/hil/nvme/nvme_main.c 

OBJS += \
./src/hil/nvme/host_lld.o \
./src/hil/nvme/nvme_admin_cmd.o \
./src/hil/nvme/nvme_identify.o \
./src/hil/nvme/nvme_io_cmd.o \
./src/hil/nvme/nvme_main.o 

C_DEPS += \
./src/hil/nvme/host_lld.d \
./src/hil/nvme/nvme_admin_cmd.d \
./src/hil/nvme/nvme_identify.d \
./src/hil/nvme/nvme_io_cmd.d \
./src/hil/nvme/nvme_main.d 


# Each subdirectory must supply rules for building sources it contributes
src/hil/nvme/host_lld.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/hil/nvme/host_lld.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -DCOSMOS_PLUS=1 -DDFTL=1 -Wall -O2 -I../../dftl_bsp/ps7_cortexa9_0/include -I../../../../common -I../../../../fil -I../../../../ftl -I../../../../ftl/dftl -I../../../../hil -I../../../../hil/nvme -I../../../../target -I../../../../target/cosmos_plus -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/hil/nvme/nvme_admin_cmd.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/hil/nvme/nvme_admin_cmd.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -DCOSMOS_PLUS=1 -DDFTL=1 -Wall -O2 -I../../dftl_bsp/ps7_cortexa9_0/include -I../../../../common -I../../../../fil -I../../../../ftl -I../../../../ftl/dftl -I../../../../hil -I../../../../hil/nvme -I../../../../target -I../../../../target/cosmos_plus -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/hil/nvme/nvme_identify.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/hil/nvme/nvme_identify.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -DCOSMOS_PLUS=1 -DDFTL=1 -Wall -O2 -I../../dftl_bsp/ps7_cortexa9_0/include -I../../../../common -I../../../../fil -I../../../../ftl -I../../../../ftl/dftl -I../../../../hil -I../../../../hil/nvme -I../../../../target -I../../../../target/cosmos_plus -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/hil/nvme/nvme_io_cmd.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/hil/nvme/nvme_io_cmd.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -DCOSMOS_PLUS=1 -DDFTL=1 -Wall -O2 -I../../dftl_bsp/ps7_cortexa9_0/include -I../../../../common -I../../../../fil -I../../../../ftl -I../../../../ftl/dftl -I../../../../hil -I../../../../hil/nvme -I../../../../target -I../../../../target/cosmos_plus -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/hil/nvme/nvme_main.o: C:/Users/User/Desktop/Cosmos_SSD-IOUB/hil/nvme/nvme_main.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 g++ compiler'
	arm-none-eabi-g++ -DCOSMOS_PLUS=1 -DDFTL=1 -Wall -O2 -I../../dftl_bsp/ps7_cortexa9_0/include -I../../../../common -I../../../../fil -I../../../../ftl -I../../../../ftl/dftl -I../../../../hil -I../../../../hil/nvme -I../../../../target -I../../../../target/cosmos_plus -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


