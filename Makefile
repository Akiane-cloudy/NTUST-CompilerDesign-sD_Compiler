# Directories
SRC       := src
INCLUDE   := include
BUILD     := build
BIN       := parser

# Compiler & flags
CXX       := g++
CXXFLAGS  := -std=c++17 -Wall -I$(INCLUDE) -I$(SRC) \
			 -Wno-unused-function -Wno-unused-variable

# Flex
FLEX      := flex
LEX_OUT   := $(SRC)/yy.lex.cpp

# Bison
BISON     := bison
BISON_FLAGS := -d --defines=$(INCLUDE)/y.tab.hpp -o $(SRC)/y.tab.cpp

# Sources
BISON_SRC      := $(SRC)/parser.y
FLEX_SRC       := $(SRC)/scanner.l
SRCS           := $(wildcard $(SRC)/*.cpp)
GENERATED_SRCS := $(SRC)/y.tab.cpp $(SRC)/yy.lex.cpp

# Objects
ALL_SRCS := $(SRCS) $(GENERATED_SRCS)
OBJS     := $(patsubst $(SRC)/%.cpp,$(BUILD)/%.o,$(ALL_SRCS))

.PHONY: all clean

all: $(BIN)

# generate parser sources
$(SRC)/y.tab.cpp $(INCLUDE)/y.tab.hpp: $(BISON_SRC) $(LEX_OUT)
	@echo "Generating parser..."
	@$(BISON) $(BISON_FLAGS) $<

# generate lexer sources
$(SRC)/yy.lex.cpp: $(FLEX_SRC)
	@echo "Generating lexer..."
	@$(FLEX) --outfile=$@ $<

# build object directory
$(BUILD):
	@mkdir -p $@

# compile .cpp → .o
$(BUILD)/%.o: $(SRC)/%.cpp | $(BUILD)
	@echo "Compiling $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# link
$(BIN): $(OBJS)
	@echo "Linking $@"
	@$(CXX) $(CXXFLAGS) $^ -o $@

debug: CXXFLAGS  += -g -O0 -DDEBUG
debug: BISON_FLAGS += -Wcounterexamples
debug: all

clean:
	@echo "Cleaning..."
	@rm -rf $(BUILD) $(BIN) \
           $(SRC)/y.tab.cpp $(INCLUDE)/y.tab.hpp $(SRC)/yy.lex.cpp\
		   token.txt\
		   *.j\
		   *.class\
		   *.log