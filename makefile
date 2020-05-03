SRC_DIR = src
OBJ_DIR = bin
INC_DIR = include

TARGET = asteroids
CC = g++ -std=c++11
CFLAGS = -g -Wall -Wextra -Werror
INC = -I$(INC_DIR) -I./libs/build/include
LIBS = -L./libs/build/lib -lSDL2 -lSDL2_image -Wl,-rpath=./libs/build/lib

# installation sdl2 et sdl2_image
INSTALLDIR 		:= $(shell pwd)/libs/build
SDL_INSTALLED  	:= $(shell test -e ./libs/.installed && echo -n 1 || echo -n 0)

.PHONY: default all clean

default: setup-sdl setup-asteroids
all: default

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(addprefix $(OBJ_DIR)/, $(patsubst %.cpp, %.o, $(notdir $(SOURCES))))
HEADERS = $(wildcard $(INC_DIR)/*.hpp)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@ $(INC) $(LIBS)

.PRECIOUS: $(TARGET) $(OBJECTS)

# compiler le jeu (sans compiler sdl)
setup-asteroids: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@ $(INC)
	@echo === COMPILATION FINISHED EXECUTABLE CREATED asteroids

# effacer seulement fichiers projet
clean:
	-rm -f $(OBJ_DIR)/*.o
	-rm -f $(TARGET)

# installer sdl
setup-sdl:
	@echo === SETTING UP SDL2
ifeq ($(SDL_INSTALLED), 1)
	@echo === SDL2 ALREADY INSTALLED
else
	@echo === STARTING SETUP
# @echo === Unzipping SDL2 and SDL2_image archives
# cd libs && unzip SDL2-2.0.12.zip
# cd libs && unzip SDL2_image-2.0.5.zip
	@echo === Installing SDL2 into libs/build folder
	cd libs/SDL2-2.0.12 && ./configure --prefix=$(INSTALLDIR); make; make install
	@echo === Installing SDL2_image into libs/build folder
	cd libs/SDL2_image-2.0.5 && ./configure --prefix=$(INSTALLDIR); make all; make install
# fichier qui nous dit si sdl est installé ou pas
	@touch libs/.installed
	@echo === SETUP COMPLETE
endif

# commande à utiliser pour effacer tout tout
uninstall-all: clean uninstall-sdl mr-clean

# effacer fichiers sdl dans dossier final
uninstall-sdl:
ifeq ($(SDL_INSTALLED), 0)
	@echo === SDL NOT INSTALLED
else
	@echo === REMOVING BUILT FILES
	rm -rf libs/build/bin
	rm -rf libs/build/include
	rm -rf libs/build/lib
	rm -rf libs/build/share
# @echo === REMOVING EXTRACTED FILES
# rm -rf libs/SDL2-2.0.12
# rm -rf libs/SDL2_image-2.0.5
	rm libs/.installed
	@echo === SDL2 SUCCESSFULLY UNINSTALLED
endif

# effacer fichiers créés pendant l'installation qui peuvent accélerer une réinstallation
mr-clean:
	rm -rf libs/SDL2-2.0.12/build
	rm -rf libs/SDL2_image-2.0.5/.deps
	rm -rf libs/SDL2_image-2.0.5/.libs
	rm libs/SDL2_image-2.0.5/*.o
	rm libs/SDL2_image-2.0.5/*.lo
	rm libs/SDL2_image-2.0.5/libSDL2_image.la
	rm libs/SDL2_image-2.0.5/libtool
