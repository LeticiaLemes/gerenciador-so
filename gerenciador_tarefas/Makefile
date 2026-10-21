CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -pthread
INCLUDES := -I src
SRC      := $(shell find src -name '*.cpp')
OBJ      := $(SRC:.cpp=.o)
BIN      := bin/gerenciador

all: $(BIN)

$(BIN): $(OBJ)
	@mkdir -p bin
	$(CXX) $(OBJ) -o $@ -pthread

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJ) $(BIN)

.PHONY: all clean