SRC_DIR = src
OBJ_DIR = bin
INC_DIR = include

TARGET = main
LIBS = -lm
CC = g++
CFLAGS = -g -Wall -Wextra -Werror -I$(INC_DIR)

.PHONY: default all clean

default: $(TARGET)
all: default

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(addprefix $(OBJ_DIR)/, $(patsubst %.cpp, %.o, $(notdir $(SOURCES))))
HEADERS = $(wildcard $(INC_DIR)/*.hpp)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f $(OBJ_DIR)/*.o
	-rm -f $(TARGET)