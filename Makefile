TARGET = cadContestProbD
SRC_DIR = src
OBJ_DIR = .obj
DEP_DIR = .dep
INC_DIR = include

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(addprefix $(OBJ_DIR)/, $(patsubst %.cpp, %.o, $(notdir $(SRC))))
DEP = $(addprefix $(DEP_DIR)/, $(patsubst %.cpp, %.d, $(notdir $(SRC))))

CC = g++
CFLAG = -std=c++17 -Wall -g -O3 -I$(INC_DIR) #-static

$(TARGET) : $(OBJ)
	${CC} ${CFLAG} $^ -o $@

-include $(DEP)

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	@mkdir -p $(DEP_DIR)
	@mkdir -p $(OBJ_DIR)
	${CC} ${CFLAG} $< -MMD -MF $(DEP_DIR)/$(basename $(notdir $<)).d -c -o $@

.PHONY: clean
clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(DEP_DIR)




