CXX       = g++
CXXFLAGS  = -Ofast -std=c++17 -march=native -flto -fopenmp
TARGET    = main
SRC       = main.cpp parser.cpp fm_partitioner.cpp
BIN_DIR   = ../bin
BIN       = $(BIN_DIR)/$(TARGET)

INPUT_DIR   = ../testcase
OUTPUT_DIR  = ../output
VERIFIER    = ../verifier/verify

TIME_CMD  = /usr/bin/time -f "real %e"

RUN1_IN   = $(INPUT_DIR)/public1.txt
RUN1_OUT  = $(OUTPUT_DIR)/public1.2way.out
RUN2_IN   = $(INPUT_DIR)/public1.txt
RUN2_OUT  = $(OUTPUT_DIR)/public1.4way.out
RUN3_IN   = $(INPUT_DIR)/public2.txt
RUN3_OUT  = $(OUTPUT_DIR)/public2.2way.out
RUN4_IN   = $(INPUT_DIR)/public2.txt
RUN4_OUT  = $(OUTPUT_DIR)/public2.4way.out

all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(BIN)
	@echo "Build complete. Executable is in $(BIN)"

run1: all
	@mkdir -p $(OUTPUT_DIR)
	$(TIME_CMD) $(BIN) $(RUN1_IN) $(RUN1_OUT) 2
	@$(VERIFIER) $(RUN1_IN) $(RUN1_OUT) 2

run2: all
	@mkdir -p $(OUTPUT_DIR)
	$(TIME_CMD) $(BIN) $(RUN2_IN) $(RUN2_OUT) 4
	@$(VERIFIER) $(RUN2_IN) $(RUN2_OUT) 4

run3: all
	@mkdir -p $(OUTPUT_DIR)
	$(TIME_CMD) $(BIN) $(RUN3_IN) $(RUN3_OUT) 2
	@$(VERIFIER) $(RUN3_IN) $(RUN3_OUT) 2

run4: all
	@mkdir -p $(OUTPUT_DIR)
	$(TIME_CMD) $(BIN) $(RUN4_IN) $(RUN4_OUT) 4
	@$(VERIFIER) $(RUN4_IN) $(RUN4_OUT) 4

clean:
	rm -f $(BIN_DIR)/$(TARGET)
	@echo "Clean complete."

.PHONY: all clean run1 run2 run3 run4
