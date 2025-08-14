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

# Source files (recursively find .c, .cpp, .s files)
ROOT_DIR = .

# Detect platform and find source files
ifeq ($(OS),Windows_NT)
  # Windows specific settings for file search (using Windows-style paths)
  C_SOURCES   += $(shell dir /b /s $(ROOT_DIR)\\*.c)
  CPP_SOURCES += $(shell dir /b /s $(ROOT_DIR)\\*.cpp)
  ASM_SOURCES += $(shell dir /b /s $(ROOT_DIR)\\*.s)
else
  # Unix/Linux specific settings (using find command for Unix-based systems)
  C_SOURCES   += $(shell find ${ROOT_DIR} -name '*.c')
  CPP_SOURCES += $(shell find ${ROOT_DIR} -name '*.cpp')
  ASM_SOURCES += $(shell find ${ROOT_DIR} -name '*.s')
endif

# binaries
PREFIX =
# Check if we are on Windows
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
  HEX = $(CP) -O ihex
  BIN = $(CP) -O binary -S
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
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
endif
 
CPU = -m64
MCU = $(CPU)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS +=

# AS includes
AS_INCLUDES = 

# C includes
INCDIR = .
INCDIR += RS-232
INCDIR += gui
INCDIR += psf
INCDIR += osc

C_INC += $(foreach dir, $(INCDIR), -I $(dir))
INCLUDE_DIRS = $(C_INC)

# Compile flags for GCC
WARNINGS := -Wall
GCCFLAGS += -O0 -g $(WARNINGS)

CFLAGS_STD = -c -Os -w -std=gnu17 $(GCCFLAGS)
CXXFLAGS_STD = -c -Os -w -std=gnu++17 $(GCCFLAGS)

CFLAGS = $(MCU) $(C_DEFS) $(INCLUDE_DIRS) $(OPT) $(CFLAGS_STD)
CFLAGS += -Wno-implicit-function-declaration
CPPFLAGS = $(MCU) $(C_DEFS) $(INCLUDE_DIRS) $(OPT) $(CXXFLAGS_STD)

# Libraries
LIBDIR =
LIBS =
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

# build the application
OBJECTS = $(addprefix $(BUILD_CC_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

OBJECTS += $(addprefix $(BUILD_CPP_DIR)/,$(notdir $(CPP_SOURCES:.cpp=.o)))
vpath %.cpp $(sort $(dir $(CPP_SOURCES)))

# List of ASM program objects
OBJECTS += $(addprefix $(BUILD_ASM_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

# Compilation rules
$(BUILD_CC_DIR)/%.o: %.c Makefile | $(BUILD_CC_DIR)
	@echo " ${green} [compile:] ${YELLOW} $< ${NC}"
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_CC_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_CPP_DIR)/%.o: %.cpp Makefile | $(BUILD_CPP_DIR)
	@echo " ${green} [compile:] ${YELLOW} $< ${NC}"
	$(CXX) -c $(CPPFLAGS) -Wa,-a,-ad,-alms=$(BUILD_CPP_DIR)/$(notdir $(<:.cpp=.lst)) $< -o $@

$(BUILD_ASM_DIR)/%.o: %.s Makefile | $(BUILD_ASM_DIR)
	@echo " ${green} [compile:] ${YELLOW} $< ${NC}"
	$(AS) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_CC_DIR)/$(notdir $(<:.s=.lst)) $< -o $@

$(BUILD_APP_DIR)/$(TARGET).elf: $(OBJECTS) Makefile | $(BUILD_APP_DIR)
	@echo " ${green} [linking:] ${YELLOW} $@ ${NC} \n"
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@ --format=Berkeley
	cp $(BUILD_APP_DIR)/$(TARGET).elf ./

$(BUILD_APP_DIR)/%.hex: $(BUILD_APP_DIR)/%.elf | $(BUILD_APP_DIR)
	$(HEX) $< $@
	
$(BUILD_APP_DIR)/%.bin: $(BUILD_APP_DIR)/%.elf | $(BUILD_APP_DIR)
	$(BIN) $< $@	
	
# Create necessary directories
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

# Include dependency files
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***
