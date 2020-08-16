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
DISTRIBUTABLES += $(wildcard doc/LICENSE*)


include $(RACK_DIR)/arch.mk

# QuickJS integration. Thanks to Jerry Sievert & Cschol for their help with this.
# To use QuickJS in VCV, use this repository, or it won't build properly in the VCV library.
quickjs := dep/lib/quickjs/libquickjs.a
DEPS += $(quickjs)
OBJECTS += $(quickjs)
QUICKJS_MAKE_FLAGS += prefix="$(DEP_PATH)"
ifdef ARCH_WIN
ifneq (,$(findstring gcc,${CC}))
	CROSS_PREFIX=$(subst gcc,,${CC})
else
	CROSS_PREFIX=
endif
	QUICKJS_MAKE_FLAGS += CONFIG_WIN32=y
else ifdef ARCH_MAC
ifneq (,$(findstring clang,${CC}))
	CROSS_PREFIX=$(subst clang,,${CC})
else
	CROSS_PREFIX=
endif
	QUICKJS_MAKE_FLAGS += CONFIG_DARWIN=y
endif
$(quickjs):
	# Specifying the directory name explicitly to avoid inconsistent capitalization across systems
	cd dep && git clone "https://github.com/JerrySievert/QuickJS.git" QuickJS
	cd dep/QuickJS && git checkout b70d5344013836544631c361ae20569b978176c9
	cd dep/QuickJS && CROSS_PREFIX=$(CROSS_PREFIX) $(MAKE) $(QUICKJS_MAKE_FLAGS)
	cd dep/QuickJS && CROSS_PREFIX=$(CROSS_PREFIX) $(MAKE) $(QUICKJS_MAKE_FLAGS) install


# Include the Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk

ifdef ARCH_WIN
# extra dist target for Azure CI Windows build, as there is only 7zip available and no zip command
azure-win-dist: all
	mkdir -p dist/$(SLUG)
	@# Strip and copy plugin binary
	cp $(TARGET) dist/$(SLUG)/
	$(STRIP) -s dist/$(SLUG)/$(TARGET)
	@# Copy distributables
	cp -R $(DISTRIBUTABLES) dist/$(SLUG)/
	@# Create ZIP package
	cd dist && 7z a -tzip -mx=9 $(SLUG)-$(VERSION)-$(ARCH).zip -r $(SLUG)
endif
