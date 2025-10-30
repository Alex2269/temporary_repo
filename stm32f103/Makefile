### run make SILENT=0 , with  all print.
### SILENT=1 , default silent mode

SILENT ?= 1
ifeq (1,$(SILENT))
.SILENT:
endif

TARGET = application

# Debug build?
DEBUG = 1
# Optimization
OPT = -Og
OPT += -g3

ENABLE_SEMIHOSTING ?= 0

# Build paths
BUILD_DIR = build
BUILD_ASM_DIR = $(BUILD_DIR)/asm
BUILD_APP_DIR = $(BUILD_DIR)/app
BUILD_CC_DIR  = $(BUILD_DIR)/ccc
BUILD_CPP_DIR = $(BUILD_DIR)/cpp

# Source directories
SRC_DIRS =  Core
SRC_DIRS += Drivers/cmsis_cm3
SRC_DIRS += Startup
SRC_DIRS += USB_DEVICE

# Include directories
INC_DIRS =  Core
INC_DIRS += Drivers/cmsis_cm3
INC_DIRS += Startup
INC_DIRS += USB_DEVICE


# Find source files and include dirs cross-platform
ifeq ($(OS),Windows_NT)
  # Windows: use Powershell for find equivalent
  C_SOURCES   = $(shell powershell -Command "Get-ChildItem -Path $(SRC_DIRS) -Recurse -Include *.c | ForEach-Object { $_.FullName }" 2>nul)
  CPP_SOURCES = $(shell powershell -Command "Get-ChildItem -Path $(SRC_DIRS) -Recurse -Include *.cpp | ForEach-Object { $_.FullName }" 2>nul)
  ASM_SOURCES = $(shell powershell -Command "Get-ChildItem -Path $(SRC_DIRS) -Recurse -Include *.s | ForEach-Object { $_.FullName }" 2>nul)
  C_INC       = $(shell powershell -Command "Get-ChildItem -Path $(INC_DIRS) -Recurse -Include *.h* | ForEach-Object { $_.DirectoryName } | Sort-Object -Unique" 2>nul)
else
  # Unix/Linux
  C_SOURCES   = $(foreach dir, $(SRC_DIRS), $(shell find $(dir) -type f -name '*.c'))
  CPP_SOURCES = $(foreach dir, $(SRC_DIRS), $(shell find $(dir) -type f -name '*.cpp'))
  ASM_SOURCES = $(foreach dir, $(SRC_DIRS), $(shell find $(dir) -type f -name '*.s'))
  C_INC       = $(shell find $(INC_DIRS) -type f \( -name '*.h' -o -name '*.hpp' \) -exec dirname {} \; | sort -u)
endif

# Format include flags
C_INCLUDES = $(addprefix -I,$(C_INC))

# Toolchain prefix
PREFIX = arm-none-eabi-

# Compiler executables
ifeq ($(OS),Windows_NT)
  ifdef GCC_PATH
    CC  = $(GCC_PATH)/$(PREFIX)gcc.exe
    CXX = $(GCC_PATH)/$(PREFIX)g++.exe
    AS  = $(GCC_PATH)/$(PREFIX)gcc.exe -x assembler-with-cpp
    CP  = $(GCC_PATH)/$(PREFIX)objcopy.exe
    SZ  = $(GCC_PATH)/$(PREFIX)size.exe
  else
    CC  = $(PREFIX)gcc.exe
    CXX = $(PREFIX)g++.exe
    AS  = $(PREFIX)gcc.exe -x assembler-with-cpp
    CP  = $(PREFIX)objcopy.exe
    SZ  = $(PREFIX)size.exe
  endif
else
ifdef GCC_PATH
  CC  = $(GCC_PATH)/$(PREFIX)gcc
  CXX = $(GCC_PATH)/$(PREFIX)g++
  AS  = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
  CP  = $(GCC_PATH)/$(PREFIX)objcopy
  SZ  = $(GCC_PATH)/$(PREFIX)size
else
  CC  = $(PREFIX)gcc
  CXX = $(PREFIX)g++
  AS  = $(PREFIX)gcc -x assembler-with-cpp
  CP  = $(PREFIX)objcopy
  SZ  = $(PREFIX)size
endif
endif

HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# Compiler flags
#######################################
CPU = -mcpu=cortex-m3
FPU =
FLOAT-ABI = -mfloat-abi=soft
MCU = $(CPU) $(FPU) $(FLOAT-ABI) -mthumb

AS_DEFS = 
C_DEFS =

ifeq (1,$(ENABLE_SEMIHOSTING))
C_DEFS += -D SEMIHOSTING_ENABLE=1
endif

# C defines
C_DEFS +=
# C_DEFS += '-D __align(value)=__attribute__((aligned(value)))'
# C_DEFS += '-D __forceinline=__attribute__((always_inline))'
# C_DEFS += '-D __irq=__attribute__((interrupt("IRQ")))'
# C_DEFS += '-D __packed=__attribute__((__packed__))'
# C_DEFS += '-D __weak=__attribute__((weak))'

# C_DEFS += -D USE_HAL_DRIVER
# C_DEFS += -D USE_FULL_LL_DRIVER
# C_DEFS += -D USE_STDPERIPH_DRIVER
# C_DEFS += -D CALL_ARM_SYSTEM_INIT
# C_DEFS += -D ARM_MATH_CM3
##
# C_DEFS += -D USE_HAL_DRIVER
C_DEFS += -D USE_FULL_LL_DRIVER
##
C_DEFS += -D STM32F103xB

AS_INCLUDES = 

ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

GCCFLAGS += -Wall
GCCFLAGS += -Wimplicit-function-declaration
GCCFLAGS += -fdata-sections
GCCFLAGS += -ffunction-sections
GCCFLAGS += -nostdlib
GCCFLAGS += -fno-threadsafe-statics
GCCFLAGS += --param max-inline-insns-single=500
GCCFLAGS += -fno-rtti
GCCFLAGS += -fno-exceptions
GCCFLAGS += -fno-use-cxa-atexit

CFLAGS_STD = -c -Os -w -std=gnu17 $(GCCFLAGS)
CXXFLAGS_STD = -c -Os -w -std=gnu++17 $(GCCFLAGS)

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) $(CFLAGS_STD) 
CPPFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) $(CXXFLAGS_STD) 

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

# Find linker script (can be overwritten if needed)
LDSCRIPT = $(shell find . -name '*_flash.ld' | head -n 1)

FLASH_SIZE=65536
RAM_SIZE=20480

LIBDIR =
LIBS = -lc -lm

ifeq (1,$(ENABLE_SEMIHOSTING))
LDFLAGS += --specs=rdimon.specs
LIBS += -lrdimon
else
LDFLAGS += --specs=nano.specs
LIBS += -lnosys
endif

LDFLAGS += -Wl,--defsym=LD_FLASH_OFFSET=0
LDFLAGS += -Wl,--defsym=LD_MAX_SIZE=$(FLASH_SIZE)
LDFLAGS += -Wl,--defsym=LD_MAX_DATA_SIZE=$(RAM_SIZE)
LDFLAGS += $(MCU) -T$(LDSCRIPT)
LDFLAGS += -Wl,-gc-sections,--print-memory-usage,-Map=$(BUILD_CC_DIR)/$(TARGET).map
LDFLAGS += -Wl,--check-sections
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,--entry=Reset_Handler
LDFLAGS += -Wl,--unresolved-symbols=report-all
LDFLAGS += -Wl,--warn-common
LDFLAGS += -Wl,--default-script=$(LDSCRIPT)
LDFLAGS += -Wl,--start-group $(LIBDIR) $(LIBS) -Wl,--end-group
LDFLAGS += -lgcc -lstdc++

ifeq (1,$(ENABLE_SEMIHOSTING))
  # nothing
else
  LDFLAGS += -u _printf_float -u _scanf_float
endif

# Default target
all: $(BUILD_APP_DIR)/$(TARGET).elf $(BUILD_APP_DIR)/$(TARGET).hex $(BUILD_APP_DIR)/$(TARGET).bin

## shell color ##
green=\033[0;32m
YELLOW=\033[1;33m
NC=\033[0m
##-------------##

# Object files
OBJECTS = $(addprefix $(BUILD_CC_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

OBJECTS += $(addprefix $(BUILD_CPP_DIR)/,$(notdir $(CPP_SOURCES:.cpp=.o)))
vpath %.cpp $(sort $(dir $(CPP_SOURCES)))

OBJECTS += $(addprefix $(BUILD_ASM_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

# Build rules

$(BUILD_CC_DIR)/%.o: %.c Makefile | $(BUILD_CC_DIR)
	@echo " ${green} [compile:] ${YELLOW} $< ${NC}"
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_CC_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_CPP_DIR)/%.o: %.cpp Makefile | $(BUILD_CPP_DIR)
	@echo " ${green} [compile:] ${YELLOW} $< ${NC}"
	$(CXX) -c $(CPPFLAGS) -Wa,-a,-ad,-alms=$(BUILD_CPP_DIR)/$(notdir $(<:.cpp=.lst)) $< -o $@

$(BUILD_ASM_DIR)/%.o: %.s Makefile | $(BUILD_ASM_DIR)
	@echo " ${green} [compile:] ${YELLOW} $< ${NC}"
	$(AS) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_ASM_DIR)/$(notdir $(<:.s=.lst)) $< -o $@

$(BUILD_APP_DIR)/$(TARGET).elf: $(OBJECTS) Makefile | $(BUILD_APP_DIR)
	@echo " ${green} [linking:] ${YELLOW} $@ ${NC}"
	@echo "\n"
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@ --format=Berkeley
#	$(SZ) $@ --format=SysV --radix=16

$(BUILD_APP_DIR)/%.hex: $(BUILD_APP_DIR)/%.elf | $(BUILD_APP_DIR)
	$(HEX) $< $@
	
$(BUILD_APP_DIR)/%.bin: $(BUILD_APP_DIR)/%.elf | $(BUILD_APP_DIR)
	$(BIN) $< $@	
	
# Create build folders
$(BUILD_CC_DIR):
	mkdir -p $@
$(BUILD_CPP_DIR):
	mkdir -p $@
$(BUILD_APP_DIR):
	mkdir -p $@
$(BUILD_ASM_DIR):
	mkdir -p $@

# Clean
clean:
ifeq ($(OS),Windows_NT)
	del /q /s $(BUILD_DIR)
else
	rm -rf $(BUILD_DIR)
endif

# Debug
OPENOCD_SCRIPT    = sc_openocd.cfg
GDB_PORT          = 3333
OPENOCD_INTERFACE = interface/cmsis-dap.cfg
OPENOCD_TARGET    = target/stm32f1x.cfg
ADAPTER_SPEED     = adapter speed 1800

ifeq (1,$(ENABLE_SEMIHOSTING))
debug:
	$(shell  echo 'gdb_port $(GDB_PORT)' > $(OPENOCD_SCRIPT))
	$(shell  echo 'source [find $(OPENOCD_INTERFACE)]' >> $(OPENOCD_SCRIPT))
	$(shell  echo 'source [find $(OPENOCD_TARGET)]' >> $(OPENOCD_SCRIPT))
	$(shell  echo 'init' >> $(OPENOCD_SCRIPT))
	$(shell  echo 'arm semihosting enable' >> $(OPENOCD_SCRIPT))
	$(shell  echo $(ADAPTER_SPEED) >> $(OPENOCD_SCRIPT))
	sh tmux.run $(BUILD_APP_DIR)/$(TARGET).elf
else
debug:
	gdb-multiarch \
	--eval-command "load" \
	-iex ' tar ext | openocd \
	-f $(OPENOCD_INTERFACE) \
	-f $(OPENOCD_TARGET) \
	-c "adapter_khz 1800" \
	-c "interface hla" \
	-c "gdb_port pipe" ' \
	-iex "monitor halt" \
	$(BUILD_APP_DIR)/$(TARGET).elf
endif

# Flash commands
flash:
	st-flash --reset write $(BUILD_APP_DIR)/$(TARGET).bin 0x8000000

flash_daplink:
	openocd -f $(OPENOCD_INTERFACE) -f $(OPENOCD_TARGET) -c "set FLASH_SIZE $(FLASH_SIZE)" -c "program $(BUILD_APP_DIR)/$(TARGET).bin 0x08000000 verify reset exit"

stm32flash:
	stm32flash -w $(BUILD_APP_DIR)/$(TARGET).bin -v -g 0x0 /dev/ttyUSB0

# Dependencies
-include $(wildcard $(BUILD_DIR)/*.d)

