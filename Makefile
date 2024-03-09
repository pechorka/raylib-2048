RAYLIB_5_AMD64_LIB = raylib/5.0/lib/libraylib_amd64.a
RAYLIB_5_WEB_LIB = raylib/5.0/lib/libraylib_web.a
RAYLIB_H=raylib/5.0/src

$(RAYLIB_5_AMD64_LIB): 
	$(MAKE) clean -C raylib/5.0/src
	$(MAKE) -C raylib/5.0/src PLATFORM=PLATFORM_DESKTOP
	mkdir -p raylib/5.0/lib
	mv raylib/5.0/src/libraylib.a raylib/5.0/lib/libraylib_amd64.a

$(RAYLIB_5_WEB_LIB):
	$(MAKE) clean -C raylib/5.0/src
	$(MAKE) -C raylib/5.0/src PLATFORM=PLATFORM_WEB
	mkdir -p raylib/5.0/lib
	mv raylib/5.0/src/libraylib.a raylib/5.0/lib/libraylib_web.a

INCLUDES = -I$(RAYLIB_H)
CFLAGS = -Wall -Wextra -ggdb $(INCLUDES)
LDFLAGS = -Lraylib/5.0/lib -lglfw -lGL -lm -lpthread -ldl -lrt
GAME_EXE = game

.PHONY: $(GAME_EXE)
$(GAME_EXE): $(RAYLIB_5_AMD64_LIB)
	gcc game.c $(INCLUDES) $(CFLAGS) -o $(GAME_EXE) $(LDFLAGS) -lraylib_amd64

run: $(GAME_EXE)
	./$(GAME_EXE)

GAME_HTML = web/game.js

.PHONY: $(GAME_HTML)
$(GAME_HTML): $(RAYLIB_5_WEB_LIB)
	emcc -o $(GAME_HTML) game.c -Os -Wall $(INCLUDES) -Lraylib/5.0/lib -lraylib_web -s USE_GLFW=3 -DPLATFORM_WEB

web: $(GAME_HTML)
	python3 -m http.server --directory web/
