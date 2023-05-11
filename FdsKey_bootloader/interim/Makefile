BUILD_ARTIFACT_NAME := FdsKey

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += $(BUILD_ARTIFACT_NAME).elf

OBJCOPY_BIN += $(BUILD_ARTIFACT_NAME).bin

COMMIT_FILE := ../Core/Inc/commit.h
LINKER_SCRIPT := ../STM32G0B0CETX_FLASH.ld
SOURCES := $(shell find .. -type f -name '*.c' | sort -u)
OBJS := $(patsubst %.c,%.o,$(SOURCES))
ASM_SOURCES := $(shell find .. -type f -name '*.s' | sort -u)
ASM_OBJS := $(patsubst %.s,%.o,$(ASM_SOURCES))
H_DIRS= := $(shell find .. -type f -name '*.h' -exec dirname {} \; | sort -u)
INCLUDES := $(addprefix -I,$(H_DIRS))

# Main-build Target
all: $(EXECUTABLES) $(OBJCOPY_BIN)

# Tool invocations
$(EXECUTABLES): $(OBJS) $(ASM_OBJS)
	arm-none-eabi-gcc -o "$(EXECUTABLES)" $(OBJS) $(ASM_OBJS) $(LIBS) -mcpu=cortex-m0plus -T"$(LINKER_SCRIPT)" --specs=nosys.specs -Wl,--gc-sections -static --specs=nano.specs -mfloat-abi=soft -mthumb -Wl,--start-group -lc -lm -Wl,--end-group

$(OBJS): %.o: %.c pre-build
	arm-none-eabi-gcc "$<" $(INCLUDES) -mcpu=cortex-m0plus -std=gnu11 -DUSE_HAL_DRIVER -DSTM32G0B0xx -c -Ofast -ffunction-sections -fdata-sections -Wall -fstack-usage --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

$(ASM_OBJS): %.o: %.s
	arm-none-eabi-gcc -x assembler-with-cpp "$<" -mcpu=cortex-m0plus -c --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

$(OBJCOPY_BIN): $(EXECUTABLES)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "$(OBJCOPY_BIN)"

clean:
	rm -rf $(EXECUTABLES) $(OBJCOPY_BIN) $(COMMIT_FILE)
	find .. -name *.o -delete
	find .. -name *.d -delete

pre-build:
	echo "#define COMMIT \"$$(git rev-parse --short HEAD)$$(git diff-index --quiet HEAD -- || echo ' (dirty)')\"\n#define BUILD_DATE \"$$(TZ=UTC date +'%Y-%m-%d')\"\n#define BUILD_TIME \"$$(TZ=UTC date +'%H:%M:%S')\"\n#define INTERIM\n" > $(COMMIT_FILE)

.PHONY: all clean pre-build
