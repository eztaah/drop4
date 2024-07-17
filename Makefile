# Variables
SOURCE_FILES = src/*.c src/3dparty/cJSON/cJSON.c
RAYLIB_DESKTOP_LIB = src/3dparty/raylib/libraylib-desktop.a
RAYLIB_WEB_LIB = src/3dparty/raylib/libraylib-web.a

CFLAGS = -Wall -Wextra -std=c18
DESKTOP_FLAGS = -g -lGL -lm -lpthread -ldl -lrt -lX11
PRELOAD_LANG_FILES_FR = --preload-file ./src/assets/languages/fr.json --preload-file ./src/assets/rules-fr.png
PRELOAD_LANG_FILES_EN = --preload-file ./src/assets/languages/en.json --preload-file ./src/assets/rules-en.png
PRELOAD_FILES_WEB = --preload-file ./src/assets/home.png \
					--preload-file ./src/assets/restart.png \
					--preload-file ./src/assets/game-overview.png \
					--preload-file ./src/assets/play_image.png \
					--preload-file ./src/assets/one_player_image.png \
					--preload-file ./src/assets/two_players_image.png \
					--preload-file ./src/assets/game_icon.png
WEB_FLAGS = -Os -s USE_GLFW=3 \
			-s EXPORTED_FUNCTIONS="['_main', '_update_canvas_size', '_set_device_type']" \
    		-s EXPORTED_RUNTIME_METHODS="['ccall', 'cwrap']" --shell-file src/my_shell.html -DPLATFORM_WEB \
			-Wformat-security

# Targets
.PHONY: debug release desktop clean

debug:
	mkdir -p out/web/en
	emcc $(SOURCE_FILES) -Isrc/ -DDEV_FEATURES -DLANG_EN $(RAYLIB_WEB_LIB) $(CFLAGS) $(PRELOAD_FILES_WEB) $(PRELOAD_LANG_FILES_EN) $(WEB_FLAGS) -o out/web/en/game.html
	mkdir -p out/web/fr
	emcc $(SOURCE_FILES) -Isrc/ -DDEV_FEATURES -DLANG_FR $(RAYLIB_WEB_LIB) $(CFLAGS) $(PRELOAD_FILES_WEB) $(PRELOAD_LANG_FILES_FR) $(WEB_FLAGS) -o out/web/fr/game.html

release:
	mkdir -p out/web/en
	emcc $(SOURCE_FILES) -Isrc/ -DLANG_EN $(RAYLIB_WEB_LIB) $(CFLAGS) $(PRELOAD_FILES_WEB) $(PRELOAD_LANG_FILES_EN) $(WEB_FLAGS) -o out/web/en/game.html
	mkdir -p out/web/fr
	emcc $(SOURCE_FILES) -Isrc/ -DLANG_FR $(RAYLIB_WEB_LIB) $(CFLAGS) $(PRELOAD_FILES_WEB) $(PRELOAD_LANG_FILES_FR) $(WEB_FLAGS) -o out/web/fr/game.html

desktop:
	mkdir -p out/desktop
	gcc $(SOURCE_FILES) -Isrc/ -DDEV_FEATURES -DLANG_EN $(RAYLIB_DESKTOP_LIB) $(CFLAGS) $(DESKTOP_FLAGS) -o out/desktop/debug_en
	gcc $(SOURCE_FILES) -Isrc/ -DDEV_FEATURES -DLANG_FR $(RAYLIB_DESKTOP_LIB) $(CFLAGS) $(DESKTOP_FLAGS) -o out/desktop/debug_fr

clean:
	rm -rf out