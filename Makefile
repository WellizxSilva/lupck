# Detect OS and set platform-specific variables
ifeq ($(OS),Windows_NT)
    PLATFORM = windows
    TARGET   = lupck.exe
    # Windows commands
    MKDIR    = mkdir -p $(1)
    RM       = rm -rf $(1)
    CP       = cp
    # Paths for Windows (adjust to your Mingw/MSVC paths)
    LUA_ROOT = V:/lua-5.4.8/lua-5.4.8
    LDFLAGS_PLATFORM = -static -static-libgcc -static-libstdc++
else
    PLATFORM = linux
    TARGET   = lupck
    MKDIR    = mkdir -p $(1)
    RM       = rm -rf $(1)
    CP       = cp
    # Paths for Linux (standard locations) <If your Lua installation is in a non-standard location, adjust this path>
    LUA_ROOT = /usr/include/lua5.4
    # Linux needs -ldl for loading dynamic libs and -lm for math
    LDFLAGS_PLATFORM = -ldl -lm
endif

# Compiler settings
CXX       = g++
ifeq ($(PLATFORM),windows)
    CXXFLAGS  = -Wall -O2 --std=c++20 -Iinclude -I$(LUA_ROOT)/include
else
    CXXFLAGS  = -Wall -O2 --std=c++20 -Iinclude -I$(LUA_ROOT)
endif
LDFLAGS   = -L"$(LUA_ROOT)/lib" $(LDFLAGS_PLATFORM)

# Detect correct Lua library name automatically
ifeq ($(wildcard $(LUA_ROOT)/lib/liblua.a),$(LUA_ROOT)/lib/liblua.a)
    LUA_LIB_NAME = -llua
    LDFLAGS += $(LUA_LIB_NAME)
else ifeq ($(wildcard $(LUA_ROOT)/lib/liblua54.a),$(LUA_ROOT)/lib/liblua54.a)
    LUA_LIB_NAME = -llua54
    LDFLAGS += $(LUA_LIB_NAME)
else ifeq ($(wildcard $(LUA_ROOT)/lib/liblua5.4.a),$(LUA_ROOT)/lib/liblua5.4.a)
    LUA_LIB_NAME = -llua5.4
    LDFLAGS += $(LUA_LIB_NAME)
else
    # Fallback for system-installed lua via pkg-config (Linux)
    LUA_LIB_NAME = $(shell pkg-config --libs lua5.4 2>/dev/null || echo "-llua")
    LDFLAGS += $(LUA_LIB_NAME) -lstdc++ -lgcc_s
endif
# Source files
SRC     = $(wildcard src/compiler/*.cpp src/core/*.cpp src/loader/*.cpp src/package/*.cpp src/runtime/*.cpp src/vm/*.cpp src/utils/*.cpp)
OBJDIR  = build
DISTDIR = dist
OBJ     = $(patsubst src/%,$(OBJDIR)/%,$(SRC:.cpp=.o))

# Embedded Lua configuration
PYTHON       = python
EMBED_SCRIPT = scripts/tools/embed_lua.py
EMBED_HEADER = include/lupck_embedded_lua.h
LUA_SRC      = lua/lupck/init.lua
EMBED       ?= 0

# Default target
all: $(TARGET)

# Object compilation (conditional dependency on EMBED_HEADER)
ifeq ($(EMBED),1)
$(OBJDIR)/%.o: src/%.cpp $(EMBED_HEADER)
else
$(OBJDIR)/%.o: src/%.cpp
endif
	@$(call MKDIR,$(dir $@))
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link final binary
$(TARGET): $(OBJ)
	@$(call MKDIR,$(DISTDIR))
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo [SUCCESS] Built for $(PLATFORM): $(TARGET)

# Generate embedded Lua header only if EMBED=1
ifeq ($(EMBED),1)
generate_embed: $(EMBED_HEADER)

$(EMBED_HEADER): $(LUA_SRC)
	@echo "[EMBED] Generating embedded Lua header..."
	$(PYTHON) $(EMBED_SCRIPT)
else
generate_embed:
	@echo "[EMBED] Skipping embedding (EMBED=0)"
endif

# Utility rules
clean:
	@echo Cleaning project...
	@$(call RM,$(OBJDIR))
	@$(call RM,$(DISTDIR))

install: all
	@echo Installing lupck...
	@$(call MKDIR,$(BINDIR))
	$(CP) $(TARGET) $(BINDIR)/$(TARGET)
	@echo [SUCCESS] lupck installed to $(BINDIR)

.PHONY: all clean generate_embed install
