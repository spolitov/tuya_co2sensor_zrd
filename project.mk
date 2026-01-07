# Add inputs and outputs from these tool invocations to the build variables
OUT_DIR += /$(SRC_PATH)

OBJS += \
$(OUT_PATH)/$(SRC_PATH)/app_bootloader.o \
$(OUT_PATH)/$(SRC_PATH)/app_endpoint_cfg.o \
$(OUT_PATH)/$(SRC_PATH)/app_main.o \
$(OUT_PATH)/$(SRC_PATH)/app_utility.o \
$(OUT_PATH)/$(SRC_PATH)/app_zb.o \
$(OUT_PATH)/$(SRC_PATH)/app_zcl.o

# Each subdirectory must supply rules for building sources it contributes
$(OUT_PATH)/$(SRC_PATH)/%.o: $(SRC_PATH)/%.c 
	@echo 'Building file: $<'
	@$(CC) $(GCC_FLAGS) $(INCLUDE_PATHS) -c -o"$@" "$<"

