CC = gcc
CPP = g++

EXE = augustus

SRC_DIR = src
BUILD_DIR = build
CACHE_DIR = $(BUILD_DIR)/cache
DEPS_DIR = deps
RESOURCES_DIR = resources

INCLUDE = include\
		  $(DEPS_DIR)/cglm/include\
		  $(DEPS_DIR)/cgltf\
		  $(DEPS_DIR)/VulkanMemoryAllocator/include\
		  $(DEPS_DIR)/stb\

INCLUDE_DIRS := $(foreach dir,$(INCLUDE),-I$(dir))

CCFLAGS = -Wall -pedantic -g -O2 -std=c99\
		  $(INCLUDE_DIRS)\
		  `pkg-config --cflags sdl2`\
		  `pkg-config --cflags lua`\
		  `pkg-config --cflags json-c`\
		  `pkg-config --cflags freetype2`\

CPPFLAGS = -Wall -pedantic -g -O2 -std=c++20 $(INCLUDE_DIRS)

LDFLAGS = -Ldeps/cglm/build/ -lcglm -lvulkan\
		  `pkg-config --libs sdl2`\
		  `pkg-config --libs lua`\
		  `pkg-config --libs json-c`\
		  `pkg-config --libs freetype2`\

SRC := $(shell find $(SRC_DIR)/ -type f \( -iname \*.c -o -iname \*.cpp \))
OBJ := $(addprefix $(CACHE_DIR)/, $(addsuffix .o,$(basename $(notdir $(SRC)))))

$(BUILD_DIR)/$(EXE): $(OBJ) | $(BUILD_DIR) $(CACHE_DIR) $(BUILD_DIR)/$(RESOURCES_DIR)
	@echo Linking: $@
	@$(CPP) -o $@ $^ $(LDFLAGS)

run: $(BUILD_DIR)/$(EXE)
	@(cd $(BUILD_DIR) && ./$(EXE))

$(CACHE_DIR)/%.o: $(SRC_DIR)/%.c | $(CACHE_DIR)
	@echo Building: $(notdir $<)
	@$(CC) $(CCFLAGS) -c $< -o $@

$(CACHE_DIR)/%.o: $(SRC_DIR)/%.cpp | $(CACHE_DIR)
	@echo Building: $(notdir $<)
	@$(CPP) $(CPPFLAGS) -c $< -o $@

$(BUILD_DIR):
	@echo Making \'$@\' directory
	@mkdir $@

$(CACHE_DIR): | $(BUILD_DIR)
	@echo Making $@ directory
	@mkdir -p $@

$(BUILD_DIR)/$(RESOURCES_DIR): | $(RESOURCES_DIR) $(BUILD_DIR)
	@echo Creating Symbolic Link: $(RESOURCES_DIR) to $(BUILD_DIR)/ output folder
	@ln -sf $(realpath $(RESOURCES_DIR)) $(abspath $(BUILD_DIR))

shaders:
	@(cd resources/shaders && make)

clean:
	@echo Deleting: $(BUILD_DIR)/
	@unlink $(BUILD_DIR)/$(RESOURCES_DIR)
	@rm -rf $(BUILD_DIR)/

	@echo Deleting: compiled shaders
	@(cd resources/shaders && make clean)

.PHONY: shaders clean run
