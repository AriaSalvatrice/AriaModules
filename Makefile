# If RACK_DIR is not defined when calling the Makefile, default to two directories above
RACK_DIR ?= ../..

# FLAGS will be passed to both the C and C++ compiler
FLAGS += -Idep/include
CFLAGS +=
CXXFLAGS +=

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine, but they should be added to this plugin's build system.
LDFLAGS +=

# Add .cpp files to the build
SOURCES += $(wildcard src/*.cpp)

# Add files to the ZIP package when running `make dist`
# The compiled plugin and "plugin.json" are automatically added.
DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)




# Testing Prototype's makefile to try out QuickJS & LuaJIT stuff

include $(RACK_DIR)/arch.mk

QUICKJS ?= 0
LUAJIT ?= 1

# QuickJS
ifeq ($(QUICKJS), 1)
SOURCES += src/QuickJSEngine.cpp
quickjs := dep/lib/quickjs/libquickjs.a
DEPS += $(quickjs)
OBJECTS += $(quickjs)
QUICKJS_MAKE_FLAGS += prefix="$(DEP_PATH)"
ifdef ARCH_WIN
	QUICKJS_MAKE_FLAGS += CONFIG_WIN32=y
endif
$(quickjs):
	cd dep && git clone "https://github.com/JerrySievert/QuickJS.git"
	cd dep/QuickJS && git checkout 807adc8ca9010502853d471bd8331cdc1d376b94
	cd dep/QuickJS && $(MAKE) $(QUICKJS_MAKE_FLAGS)
	cd dep/QuickJS && $(MAKE) $(QUICKJS_MAKE_FLAGS) install
endif

# LuaJIT
ifeq ($(LUAJIT), 1)
SOURCES += src/LuaJITEngine.cpp
luajit := dep/lib/libluajit-5.1.a
OBJECTS += $(luajit)
DEPS += $(luajit)
$(luajit):
	$(WGET) "http://luajit.org/download/LuaJIT-2.0.5.tar.gz"
	$(SHA256) LuaJIT-2.0.5.tar.gz 874b1f8297c697821f561f9b73b57ffd419ed8f4278c82e05b48806d30c1e979
	cd dep && $(UNTAR) ../LuaJIT-2.0.5.tar.gz
	cd dep/LuaJIT-2.0.5 && $(MAKE) BUILDMODE=static PREFIX="$(DEP_PATH)" install
endif




# Include the Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk

ifdef ARCH_WIN
# extra dist target for Azure CI Windows build, as there is only 7zip available and no zip command
azure-win-dist: all
	rm -rf dist
	mkdir -p dist/$(SLUG)
	@# Strip and copy plugin binary
	cp $(TARGET) dist/$(SLUG)/
	$(STRIP) -s dist/$(SLUG)/$(TARGET)
	@# Copy distributables
	cp -R $(DISTRIBUTABLES) dist/$(SLUG)/
	@# Create ZIP package
	cd dist && 7z a -tzip -mx=9 $(SLUG)-$(VERSION)-$(ARCH).zip -r $(SLUG)
endif
