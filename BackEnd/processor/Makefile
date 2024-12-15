
# make MODE=DEBUG
# make
# make MODE=DEBUG clean
# make clean


#executing file is in ./build/$(OUTFILE_NAME) or in ./debug/$(OUTFILE_NAME)

noop=
space = $(noop) $(noop)

MODE ?= RELEAZE #exist 3 modes: DEBUG/REALIZE/SECURITY
# in DEBUG:
# 	CFLAGS = $(CDEBFLAGS)
# 	OUT_O_DIR = debug
# 	EXTRA_FLAGS = $(SANITIZER_FLAGS)
# endif
# in SECURITY:
# ifeq ($(MODE),SECURITY)
# 	EXTRA_FLAGS = $(SECURITY_FLAGS)
# endif


ifeq ($(origin CC),default) # "CC" Variable has makefile default value. If Makefile launch ocurred without "CC" flag, "CC = c" would be defaultly set.
# So we need set value singly
	CC = g++
endif


CFLAGS ?= -O2 -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal       \
	-Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch     \
	-Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions                   \
	-Wcast-qual -Wconversion -Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 \
	-Wignored-qualifiers -Wlogical-op -Wno-missing-field-initializers -Wnon-virtual-dtor   \
	-Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing \
	-Wstrict-null-sentinel -Wtype-limits -Wwrite-strings #-Werror

CDEBFLAGS = -D _DEBUG -ggdb3 -std=c++17 -O0 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations \
-Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported \
-Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security \
-Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual\
-Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel \
-Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override\
-Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros\
-Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs \
-Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging\
-fno-omit-frame-pointer -Wlarger-than=8192 -Wstack-usage=8192 -pie -fPIE -Werror=vla\

LAUNCH_FLAGS =

SANITIZER_FLAGS = -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,$\
integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,$\
shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr

#EXTRA_FLAGS used in linking in debug mode
EXTRA_FLAGS =


ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST)))) #path to makefile

ifeq ($(MODE),DEBUG)
	CFLAGS = $(CDEBFLAGS)
	OUT_O_DIR = debug
	EXTRA_FLAGS = $(SANITIZER_FLAGS)
endif


override CFLAGS += $(COMMONINC) # CFLAGS - environment variable. We can change it using only override, but not +=, :=, =

#There are src folder files. We can use wildcard $(SRC_DIR)/*.cpp, but it isn't a good manner













#/---------------------------PROJECT_SRC_CONFIG--------------------\#
PROJECT_NAME = processor
CSRC = main.cpp src/proc_err.cpp src/proc_output.cpp src/processor_func.cpp src/processor_args_proc.cpp
LOGS_DIR = ./logs
OUT_O_DIR = build
COMMONINC = -I./inc
SRC = ./src
#/---------------------------PROJECT_SRC_CONFIG--------------------\#

#/---------------------------SUBMODULES_CONFIG--------------------\#
SUBMODULES = stack string_funcs
#/---------------------------SUBMODULES_CONFIG--------------------\#












#/---------------------------PROJECT_PROCESSING--------------------\#
SO_LIB_NAME = $(PROJECT_NAME)
OUTFILE_NAME = $(PROJECT_NAME).out
COBJ := $(addprefix $(OUT_O_DIR)/,$(CSRC:.cpp=.o))
DEPS = $(COBJ:.o=.d)
#/---------------------------PROJECT_PROCESSING--------------------\#

#/---------------------------SUBMODULES_PROCESSING--------------------\#
SO_LIBS_PATHES = $(foreach item,$(SUBMODULES),./$(item)/libs)
SO_LIBS_START_FILES = $(foreach item,$(SO_LIBS_PATHES), $(wildcard $(item)/*.so))
SO_LIBS_RAW_FILES = $(foreach item,$(SO_LIBS_PATHES), \
	$(foreach file,$(wildcard $(item)/*.so),$(subst $(item)/lib,,$(file))))
SO_LIBS_FILES = $(foreach item,$(SO_LIBS_RAW_FILES),$(subst .so,,$(item)))
COMMONINC += $(foreach item,$(SUBMODULES), -I./$(item)/inc)
LDFLAGS += $(foreach item,$(SUBMODULES), -L ./$(item)/libs)
LDFLAGS += $(foreach item,$(SO_LIBS_FILES), -l $(item))
LAUNCH_PREFLAGS = LD_LIBRARY_PATH=$(subst $(space),:,$(SO_LIBS_PATHES))
#/---------------------------SUBMODULES_PROCESSING--------------------\#

#":=" - forced assignment (not lazy)











.PHONY: all # We use this comand to avoid conflicts with files or folders with name "all". "all" is name of target

all: $(OUT_O_DIR)/$(OUTFILE_NAME) # Target all depends on "$(OUTFILE_NAME)" file creation. That is when we use "all" target, "$(OUTFILE_NAME)" file will be created




launch:
	@$(LAUNCH_PREFLAGS) ./$(OUT_O_DIR)/$(OUTFILE_NAME) $(LAUNCH_FLAGS)

#FIXME: научится автоматически создавать динамические библиотеки сабмодулей. cd submodule_path && make DynLibGen

DynLibGen: $(COBJ)
	@mkdir -p libs

#copying all contaiment '/libs' directory of all submodules
	@$(foreach path,$(SO_LIBS_PATHES), \
		$(foreach item,$(wildcard $(path)/*.so), \
			cp $(item) ./libs/$(subst $(path)/,,$(item)); \
		) \
	)
#creation dynamic lib of project source files
	@$(CC) -shared $(filter-out %main.o, $(COBJ)) -o libs/lib$(SO_LIB_NAME).so


$(OUT_O_DIR)/$(OUTFILE_NAME): $(COBJ) # Each "$(OUTFILE_NAME)" file depends on objects after ":". If we touch "$(OUTFILE_NAME)", all files from "$(COBJ)" will be touched
# All $(COBJ) files will be linked and converted into "$@" file i.e. "$(OUTFILE_NAME)". Note: "$(OUTFILE_NAME)" is executable project file
# echo $^ -> main.o args_proc.o conf_ctor.o error_processing.o output.o stack_funcs.o
# $@ - target
#LINKING!!!
	@$(CC) $^ -o $@ $(LDFLAGS) $(EXTRA_FLAGS)

# static pattern rule to not redefine generaic one
#@D - target directory
$(COBJ) : $(OUT_O_DIR)/%.o : %.cpp # Object files creation
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@

$(DEPS) : $(OUT_O_DIR)/%.d : %.cpp # Object files creation
	@mkdir -p $(@D)
	@$(CC) -E $(CFLAGS) $< -MM -MT $(@:.d=.o) > $@

.PHONY: clean

clean:
	@rm -rf $(COBJ) $(DEPS) $(OUT_O_DIR)/*.out $(OUT_O_DIR)/*.log

clean_logs:
	@rm -rf $(LOGS_DIR)
NODEPS = clean

ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS)))) # if we use make clean, we shouldn't include $(DEPS)
include $(DEPS) #????? if header was changed, src cpp would compile again, to remake preprocessing with changed header
endif