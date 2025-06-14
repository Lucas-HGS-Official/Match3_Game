TARGET = bin/game
SRC = $(wildcard code/src/*.c)
OBJ = $(patsubst code/src/%.c, code/obj/%.o, $(SRC))
LIB_SRC = $(wildcard code/libs/src/*.c)
LIB_OBJ = $(patsubst code/libs/src/%.c, code/libs/obj/%.o, $(LIB_SRC))

###################################
# Build Everything
###################################
default: from_scratch

from_scratch: lib_build clean build
###################################

###################################
# Build Game
###################################
build: lib_copy $(TARGET)

$(TARGET): $(OBJ)
	gcc -o $@ $? -Wall -std=c99 -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

code/obj/%.o : code/src/%.c
	gcc -c $< -o $@ -Icode/include -Icode/libs/include

clean:
	rm -f code/obj/*.o
	rm -f $(TARGET)
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