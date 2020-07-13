cmake_minimum_required(VERSION 3.10)
project(shiromino VERSION 0.1 LANGUAGES CXX)
set(GAME_EXECUTABLE "${PROJECT_NAME}")
set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
configure_file(include/configuration.h.in configuration.h @ONLY)
set(SOURCES
	src/Audio.cc
	src/core.cc
	src/game_menu.cc
	src/game_qs.cc
	src/gfx_helpers.cc
	src/gfx_menu.cc
	src/gfx_qs.cc
	src/gfx.cc
	src/Grid.cc
	src/GuiGridCanvas.cc
	src/GuiScreenManager.cc
	src/main.cc
	src/Menu/Option.cc
	src/Mode/BaseMode.cc
	src/MultiEditor.cc
	src/PieceDefinition.cc
	src/QRS.cc
	src/random.cc
	src/RecordList.cc
	src/replay.cc
	src/RotationTables.cc
	src/Settings.cc
	src/ShiroPhysoMino.cc
	src/SPM_Randomizer.cc
	src/SPM_Spec.cc
	src/Timer.cc
	src/SGUIL/SGUIL_GuiButton.cc
	src/SGUIL/SGUIL_GuiTextField.cc
	src/SGUIL/SGUIL_GuiWindow.cc
	src/SGUIL/SGUIL.cc
	${OPENGL_SOURCES}
)
set(HEADERS
	src/AssetStore.h
	src/Audio.h
	src/CoreState.h
	src/Debug.h
	src/DisplayMode.h
	src/fonts.h
	src/game_menu.h
	src/game_qs.h
	src/gfx.h
	src/gfx_helpers.h
	src/gfx_menu.h
	src/gfx_qs.h
	src/gfx_structures.h
	src/Grid.h
	src/GuiGridCanvas.h
	src/GuiScreenManager.h
	src/images.h
	src/Input.h
	src/Menu/ActionOption.h
	src/Menu/ElementType.h
	src/Menu/GameArguments.h
	src/Menu/GameMultiOption.h
	src/Menu/GameOption.h
	src/Menu/MetaGameOption.h
	src/Menu/MultiOption.h
	src/Menu/Option.h
	src/Menu/TextOption.h
	src/Menu/ToggleOption.h
	src/Mode/BaseMode.h
	src/MultiEditor.h
	src/music.h
	src/PieceDefinition.h
	src/Player.h
	src/Player/BasePlayer.h
	src/QRS0.h
	src/QRS1.h
	src/RecordList.h
	src/random.h
	src/replay.h
	src/RefreshRates.h
	src/RotationTables.h
	src/Settings.h
	src/sfx.h
	src/ShiroPhysoMino.h
	src/SPM_Randomizer.h
	src/SPM_Spec.h
	src/stringtools.h
	src/TGM.h
	src/Timer.h
	src/SGUIL/SGUIL.h
	${OPENGL_HEADERS}
)
find_package(OpenGL)
if (OPENGL_FOUND)
	set(ENABLE_OPENGL_INTERPOLATION ON)
endif()
if(ENABLE_OPENGL_INTERPOLATION)
	set(OpenGL_GL_PREFERENCE GLVND)
	find_package(OpenGL REQUIRED)
	set(SOURCES
		${SOURCES}
		third-party/glad.cc
	)
	set(HEADERS
		${HEADERS}
		third-party/glad.h
		third-party/khrplatform.h
	)
endif()
add_executable("${GAME_EXECUTABLE}" ${SOURCES} ${HEADERS})
source_group("Header Files" FILES ${HEADERS})
target_include_directories("${GAME_EXECUTABLE}" SYSTEM PRIVATE third-party)
target_include_directories("${GAME_EXECUTABLE}" PUBLIC src)
# TODO: This is pretty dirty. Replace this with a generator.
target_include_directories("${GAME_EXECUTABLE}" PUBLIC ${PROJECT_BINARY_DIR})
set_property(
	TARGET "${GAME_EXECUTABLE}"
	PROPERTY VS_DEBUGGER_WORKING_DIRECTORY
	${CMAKE_SOURCE_DIR}
)
if(ENABLE_OPENGL_INTERPOLATION)
	target_link_libraries("${GAME_EXECUTABLE}" PRIVATE OpenGL::GL)
endif()
find_package(PkgConfig)
if(PKG_CONFIG_FOUND AND NOT VCPKG_TOOLCHAIN)
	pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
	pkg_check_modules(SDL2_image REQUIRED IMPORTED_TARGET SDL2_image)
	pkg_check_modules(SDL2_mixer REQUIRED IMPORTED_TARGET SDL2_mixer)
	pkg_check_modules(sqlite3 REQUIRED IMPORTED_TARGET sqlite3)
	target_link_libraries("${GAME_EXECUTABLE}" PRIVATE
		m
		${CMAKE_DL_LIBS}
		${SDL2_LIBRARIES}
		${SDL2_image_LIBRARIES}
		${SDL2_mixer_LIBRARIES}
		${sqlite3_LIBRARIES}
	)
	if(APPLE)
		# Homebrew doesn't appear to append entries to `LD_LIBRARY_PATH`, so we have to do that ourselves.
		target_link_directories("${GAME_EXECUTABLE}" PUBLIC
			${SDL2_LIBRARY_DIRS}
			${SDL2_image_LIBRARY_DIRS}
			${SDL2_mixer_LIBRARY_DIRS}
			${sqlite3_LIBRARY_DIRS}
		)
	endif()
	target_include_directories("${GAME_EXECUTABLE}" SYSTEM PRIVATE ${SDL2_INCLUDE_DIRS})
else()
	find_package(SDL2 REQUIRED)
	find_package(sdl2-image REQUIRED)
	find_package(sdl2-mixer REQUIRED)
	find_package(SQLite3 REQUIRED)
	target_link_libraries("${GAME_EXECUTABLE}" PRIVATE
		SDL2::SDL2
		SDL2::SDL2main
		SDL2::SDL2_image
		SDL2::SDL2_mixer
		SQLite::SQLite3
	)
	if(VCPKG_TOOLCHAIN)
		# When using dynamically linked libraries, Vcpkg doesn't automatically copy over the vorbis library needed to play ogg files with SDL_mixer,
		# so they have to be explicitly added as dependencies to shiromino to work around that bug.
		find_library(VORBISFILE_LIBRARY vorbisfile REQUIRED)
		target_link_libraries("${GAME_EXECUTABLE}" PRIVATE ${VORBISFILE_LIBRARY})
		add_compile_definitions(VCPKG_TOOLCHAIN)
		set(VCPKG_CRT_LINKAGE dynamic)
		set(VCPKG_LIBRARY_LINKAGE dynamic)
	endif()
endif()
# target_link_libraries("${GAME_EXECUTABLE}" PRIVATE -fsanitize=address,leak,pointer-compare,pointer-subtract,undefined -fsanitize-address-use-after-scope -fsanitize-undefined-trap-on-error)
install(DIRECTORY assets DESTINATION "usr/share/${PROJECT_NAME}")
install(TARGETS "${GAME_EXECUTABLE}" RUNTIME)