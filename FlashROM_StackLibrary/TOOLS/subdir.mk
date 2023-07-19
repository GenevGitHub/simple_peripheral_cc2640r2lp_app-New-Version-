################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CFG_SRCS += \
../TOOLS/app_ble.cfg 


# Each subdirectory must supply rules for building sources it contributes
configPkg/linker.cmd: ../TOOLS/app_ble.cfg
	@echo 'Building file: $<'
	@echo 'Invoking: XDCtools'
	"/xs" --xdcpath="/source;/kernel/tirtos/packages;" xdc.tools.configuro -o configPkg -t ti.targets.arm.elf.M3 -p ti.platforms.simplelink:CC2640R2F -r release -c --compileOptions "$<"
	@echo 'Finished building: $<'
	@echo ' '


