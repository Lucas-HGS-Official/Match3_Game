TARGET = bin/game
WEB_TARGET = bin/game.html
WEB_RAYLIB = code/libs/static_libs
SRC = $(wildcard code/src/*.c)
OBJ = $(patsubst code/src/%.c, code/obj/%.o, $(SRC))
LIB_SRC = $(wildcard code/libs/src/*.c)
LIB_OBJ = $(patsubst code/libs/src/%.c, code/libs/obj/%.o, $(LIB_SRC))

###################################
# Build Everything
###################################
default: from_scratch

from_scratch: clean build
###################################

###################################
# Build Game
###################################
build: $(TARGET)

$(TARGET): $(OBJ)
	gcc -o $@ $? -Wall -std=c99 -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

code/obj/%.o : code/src/%.c
	gcc -c $< -o $@ -Icode/include -Icode/libs/include

clean:
	rm -f code/obj/*.o
	rm -f $(TARGET)*
###################################

###################################
# Build Libs
###################################
lib_build: lib_clean $(LIB_OBJ)

code/libs/obj/%.o : code/libs/src/%.c
	gcc -c $< -o $@ -Icode/libs/include

lib_copy:
	cp code/libs/obj/*.o code/obj/

lib_clean:
	rm -f code/libs/obj/*.o
###################################

###################################
# Run Game
###################################
run:
	./$(TARGET)

###################################
# web
###################################
update_emcc:
	~/emsdk/emsdk install latest

activate_emcc:
	~/emsdk/emsdk activate latest
	source ~/emsdk/emsdk_env.sh

web_build: $(WEB_TARGET)

$(WEB_TARGET):
	emcc -o $@ code/src/main.c -Os -Wall ./$(WEB_RAYLIB)/libraylib.a -Icode/include -Icode/libs/include -I/home/lhgs/raylib/src -L$(WEB_RAYLIB) -s USE_GLFW=3 -s ASYNCIFY -s TOTAL_MEMORY=67108864 -s STACK_SIZE=20MB -s ASSERTIONS=1 -s EXPORTED_RUNTIME_METHODS=ccall --preload-file resources --profiling -DPLATFORM_WEB
#emcc -o bin/game.html code/src/main.c -Os -Wall ./code/libs/static_libs/libraylib.a -Icode/include -Icode/libs/include -I/home/lhgs/raylib/src -Lcode/libs/static_libs -s USE_GLFW=3 -s ASYNCIFY -s ASSERTIONS=1 --profiling -DPLATFORM_WEB

web_run:
	emrun bin/game.html