# Executable names:
EXE = raytracer

# Add all object files needed for compiling:
EXE_OBJ = main.o
OBJS = main.o image/lodepng.o vector/vector3d.o parser/parser.o image/PNG.o acceleration/BVH.o \
acceleration/SafeQueue.o scene/Object.o scene/raytracer.o bsdf/math_utils.o acceleration/SafeProgressBar.o \
scene/Material.o acceleration/Profiler.o macros.o bsdf/BDF.o bsdf/microfacets.o scene/Camera.o parser/ParserTree.o


# Optimization level:
OPT = -O3

# Compiler/linker config and object/depfile directory:
CXX = clang++
LD = clang++
OBJS_DIR = .objs


# -MMD and -MP asks clang++ to generate a .d file listing the headers used in the source code for use in the Make process.
#   -MMD: "Write a depfile containing user headers"
#   -MP : "Create phony target for each dependency (other than main file)"
#   (https://clang.llvm.org/docs/ClangCommandLineReference.html)
DEPFILE_FLAGS = -MMD -MP

# Provide lots of helpful warning/errors:
WARNINGS = -pedantic -Wall -Werror -Wfatal-errors -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function

# Flags for compile:
CXXFLAGS += -std=c++17 -stdlib=libc++ $(OPT) $(WARNINGS) $(DEPFILE_FLAGS) -g -c

# Flags for linking:
LDFLAGS += -std=c++17 -stdlib=libc++ -lc++abi

# Rule for `all` (first/default rule):
all: $(EXE)

# Rule for linking the final executable:
# - $(EXE) depends on all object files in $(OBJS)
# - `patsubst` function adds the directory name $(OBJS_DIR) before every object file
$(EXE): output_msg $(patsubst %.o, $(OBJS_DIR)/%.o, $(OBJS))
	$(LD) $(filter-out $<, $^) $(LDFLAGS) -o $@

# Ensure .objs/ exists:
$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)
	@mkdir -p $(OBJS_DIR)/acceleration
	mkdir -p $(OBJS_DIR)/bsdf
	mkdir -p $(OBJS_DIR)/image
	mkdir -p $(OBJS_DIR)/parser
	mkdir -p $(OBJS_DIR)/scene
	mkdir -p $(OBJS_DIR)/vector

# Rules for compiling source code.
# - Every object file is required by $(EXE)
# - Generates the rule requiring the .cpp file of the same name
$(OBJS_DIR)/%.o: src/%.cpp | $(OBJS_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

output_msg: ; $(CLANG_VERSION_MSG)

# Standard C++ Makefile rules:
clean:
	rm -rf $(EXE) $(OBJS_DIR) *.o *.d

tidy: clean
	rm -rf doc

.PHONY: all tidy clean output_msg