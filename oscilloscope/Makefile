### Run make SILENT=0 for full print, SILENT=1 for silent mode (default)

SILENT ?= 1
ifeq (1,$(SILENT))
.SILENT:
endif

TARGET = application

# Debug build? (set to 1 for debug, 0 for release)
DEBUG = 0

# Optimization level and debug flags
OPT = -Og
OPT += -g3  # Debug output for peripheral registers

# Build paths
BUILD_DIR = build
BUILD_ASM_DIR = $(BUILD_DIR)/asm
BUILD_APP_DIR = $(BUILD_DIR)/app
BUILD_CC_DIR  = $(BUILD_DIR)/ccc
BUILD_CPP_DIR = $(BUILD_DIR)/cpp

# Source directories
SRC_DIRS =  main
SRC_DIRS += fonts
SRC_DIRS += RS-232
SRC_DIRS += gui
SRC_DIRS += color_utils
SRC_DIRS += glyphs
# SRC_DIRS += psf
SRC_DIRS += osc
SRC_DIRS += widgets

# Include directories
INC_DIRS =  main
INC_DIRS += fonts
INC_DIRS += RS-232
INC_DIRS += gui
INC_DIRS += color_utils
INC_DIRS += glyphs
# INC_DIRS += psf
INC_DIRS += osc
INC_DIRS += widgets

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
PREFIX =

# Compiler executables
ifeq ($(OS),Windows_NT)
  # Windows specific settings
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
  # Linux/Unix specific settings
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
 
CPU = -m64
MCU = $(CPU)

AS_DEFS = 

# C defines
C_DEFS +=

AS_INCLUDES = 

ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

# Compile flags for GCC
WARNINGS := -Wall
# WARNINGS += -Wextra
# WARNINGS += -Wshadow
# WARNINGS += -Wundef
# WARNINGS += -Wmaybe-uninitialized
# WARNINGS += -Wno-unused-function
# WARNINGS += -Wno-error=strict-prototypes
# WARNINGS += -Wno-error=cpp
# WARNINGS += -Wno-unused-parameter
# WARNINGS += -Wno-missing-field-initializers
# WARNINGS += -Wno-format-nonliteral
# WARNINGS += -Wno-cast-qual
# WARNINGS += -Wno-switch-default
# WARNINGS += -Wno-ignored-qualifiers
# WARNINGS += -Wno-error=pedantic
# WARNINGS += -Wno-sign-compare
# WARNINGS += -Wno-error=missing-prototypes
# WARNINGS += -Wpointer-arith -fno-strict-aliasing
# WARNINGS += -Wuninitialized
# WARNINGS += -Wunreachable-code
# WARNINGS += -Wreturn-type
# WARNINGS += -Wmultichar
# WARNINGS += -Wformat-security
# WARNINGS += -Wdouble-promotion
# WARNINGS += -Wclobbered
# WARNINGS += -Wdeprecated
# WARNINGS += -Wempty-body
# WARNINGS += -Wshift-negative-value
# WARNINGS += -Wtype-limits
# WARNINGS += -Wsizeof-pointer-memaccess
# WARNINGS += -Wpointer-arith

GCCFLAGS += -O0 -g $(WARNINGS)

CFLAGS_STD = -c -Os -w -std=gnu17 $(GCCFLAGS)
CXXFLAGS_STD = -c -Os -w -std=gnu++17 $(GCCFLAGS)

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) $(CFLAGS_STD) 
CPPFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) $(CXXFLAGS_STD) 

# Libraries
LIBDIR =
LIBS  = -lc
LIBS += -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# LDFLAGS setup
LDFLAGS +=  $(LIBDIR) $(LIBS)
LDFLAGS += -Wl,--start-group
LDFLAGS += -lgcc
LDFLAGS += -lstdc++
LDFLAGS += -Wl,--end-group

# Default action: build all
all: $(BUILD_APP_DIR)/$(TARGET).elf $(BUILD_APP_DIR)/$(TARGET).hex $(BUILD_APP_DIR)/$(TARGET).bin

## shell color beg ##
green=\033[0;32m
YELLOW=\033[1;33m
NC=\033[0m
## shell color end ##

# Object files
OBJECTS = $(addprefix $(BUILD_CC_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

OBJECTS += $(addprefix $(BUILD_CPP_DIR)/,$(notdir $(CPP_SOURCES:.cpp=.o)))
vpath %.cpp $(sort $(dir $(CPP_SOURCES)))

# List of ASM program objects
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

# Clean up
clean:
	-rm -fR $(BUILD_DIR)
	-rm -f $(TARGET).elf

# Dependencies
-include $(wildcard $(BUILD_DIR)/*.d)

