OUT_DIR := spirv

SHADERS := $(shell find . -type f \( -iname \*.vert -o -iname \*.frag \))
SPIRV := $(addprefix $(OUT_DIR)/, $(addsuffix .spirv, $(SHADERS)))

all: $(SPIRV)

$(OUT_DIR)/%.frag.spirv: %.frag | $(OUT_DIR)
	glslc $< -o $@

$(OUT_DIR)/%.vert.spirv: %.vert | $(OUT_DIR)
	glslc $< -o $@

$(OUT_DIR):
	@echo Making \'$@\' directory
	@mkdir $@

clean:
	rm -f *.spirv
	rm -rf $(OUT_DIR)/

.PHONY: clean
