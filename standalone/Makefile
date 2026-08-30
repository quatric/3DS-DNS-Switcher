.SUFFIXES:
ifeq ($(strip $(DEVKITARM)),)
$(error Set DEVKITARM to your devkitARM path)
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITARM)/3ds_rules

TARGET := 3DS-DNS-Switcher
BUILD := build
SOURCES := source
INCLUDES := ../include
ARCH := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
CFLAGS := -g -Wall -Wextra -O2 -mword-relocations -ffunction-sections $(ARCH)
CFLAGS += $(INCLUDE) -D__3DS__
CXXFLAGS := $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++17
ASFLAGS := -g $(ARCH)
LDFLAGS := -specs=3dsx.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)
LIBS := -lctru -lm
LIBDIRS := $(CTRULIB) $(PORTLIBS)

MAKEROM ?= makerom
BANNERTOOL ?= bannertool
RSF := $(CURDIR)/app.rsf
APP_TITLE := 3DS DNS Switcher
APP_PRODUCT_CODE := CTR-P-CTAP
APP_UNIQUE_ID := 0xFF3FF
BANNER_IMG := $(CURDIR)/assets/banner-v3.png
BANNER_AUDIO := $(CURDIR)/assets/banner-jingle-0020.wav
ICON_IMG := $(CURDIR)/assets/icon-v2.png
CIA_TARGET := 3DS-DNS-Switcher.cia

ifneq ($(BUILD),$(notdir $(CURDIR)))
export OUTPUT := $(CURDIR)/$(TARGET)
export TOPDIR := $(CURDIR)
export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR := $(CURDIR)/$(BUILD)
CFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
export LD := $(CXX)
export OFILES := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export INCLUDE := $(foreach dir,$(INCLUDES),-I $(CURDIR)/$(dir)) $(foreach dir,$(LIBDIRS),-I $(dir)/include) -I $(CURDIR)/$(BUILD)
export LIBPATHS := $(foreach dir,$(LIBDIRS),-L $(dir)/lib)

.PHONY: all clean cia $(BUILD)
all: $(BUILD)
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

cia: $(BUILD)
	@echo building banner and smdh ...
	$(BANNERTOOL) makesmdh -s "DNS Switcher" -l "$(APP_TITLE)" -p "quatric" -i $(ICON_IMG) -o $(TARGET).smdh
	$(BANNERTOOL) makebanner -i $(BANNER_IMG) -a $(BANNER_AUDIO) -o $(TARGET).bnr
	@echo building $(CIA_TARGET) ...
	$(MAKEROM) -f cia -o $(CIA_TARGET) -elf $(TARGET).elf -rsf $(RSF) \
		-icon $(TARGET).smdh -banner $(TARGET).bnr \
		-DAPP_TITLE="$(APP_TITLE)" -DAPP_PRODUCT_CODE="$(APP_PRODUCT_CODE)" -DAPP_UNIQUE_ID="$(APP_UNIQUE_ID)"
	@echo built ... $(CIA_TARGET)

clean:
	rm -rf $(BUILD) $(TARGET).elf $(TARGET).3dsx $(TARGET).smdh $(TARGET).bnr $(TARGET).map $(CIA_TARGET)
else
DEPENDS := $(OFILES:.o=.d)
$(OUTPUT).3dsx: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)
%.o: %.cpp
	@echo $(notdir $<)
	@$(CXX) -MMD -MP -MF $(DEPSDIR)/$*.d $(CXXFLAGS) -c $< -o $@
-include $(DEPENDS)
endif
