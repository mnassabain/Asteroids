SRC_DIR = src
OBJ_DIR = bin
INC_DIR = include

TARGET = main
CC = g++
CFLAGS = -g -Wall -Wextra -Werror
INC = -I$(INC_DIR) -I./libs/SDL2/include
LIBS = -L./libs/SDL2/lib -lSDL2 -lSDL2_image -Wl,-rpath=./libs/SDL2/lib

.PHONY: default all clean

default: $(TARGET)
all: default

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(addprefix $(OBJ_DIR)/, $(patsubst %.cpp, %.o, $(notdir $(SOURCES))))
HEADERS = $(wildcard $(INC_DIR)/*.hpp)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@ $(INC) $(LIBS)

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@ $(INC)
	export LD_LIBRAIRY_PATH=./libs/libpng12

clean:
	-rm -f $(OBJ_DIR)/*.o
	-rm -f $(TARGET)