RAYLIB_5_LIB = raylib/5.0/lib/libraylib.a
GAME_EXE = game

RAYLIB_H=raylib/5.0/src

$(RAYLIB_5_LIB): 
	$(MAKE) -C raylib/5.0/src PLATFORM=PLATFORM_DESKTOP
	mkdir -p raylib/5.0/lib
	mv raylib/5.0/src/libraylib.a raylib/5.0/lib

INCLUDES = -I$(RAYLIB_H)
CFLAGS = -Wall -Wextra -ggdb $(INCLUDES)
LDFLAGS = -Lraylib/5.0/lib -lraylib -lglfw -lGL -lm -lpthread -ldl -lrt

$(GAME_EXE): $(RAYLIB_5_LIB)
	gcc game.c $(INCLUDES) $(CFLAGS) -o $(GAME_EXE) $(LDFLAGS)

run: $(GAME_EXE)
	./$(GAME_EXE)