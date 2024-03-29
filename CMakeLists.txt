cmake_minimum_required(VERSION 3.12)

###########
# Options #
###########
set(BACKEND "SDL2" CACHE STRING "Which backend to use (SDL2)")
option(REV01 "Compile REV01 ROM" ON)
option(JAPANESE "Compile Japanese ROM" OFF)
option(FIX_BUGS "Fix bugs (completely screwed up code, not gameplay bugs)" OFF)
option(SPLASH "Enable the SSRG splash screen (for my own demo releases)" OFF)

option(SANITIZE "Enable sanitization" OFF)
option(LTO "Enable link-time optimization" OFF)
option(MSVC_LINK_STATIC_RUNTIME "Link the static MSVC runtime library (Visual Studio only)" OFF)

#########
# Setup #
#########

# Define project, source, and includes
project(SoniCPort LANGUAGES C)

add_executable(SoniCPort WIN32
	# Game
	"src/Main.c"
	"src/Constants.h"
	"src/Macros.h"
	"src/Kosinski.c"
	"src/Kosinski.h"
	"src/Nemesis.c"
	"src/Nemesis.h"
	"src/MathUtil.c"
	"src/MathUtil.h"
	"src/Game.c"
	"src/Game.h"
	"src/Demo.c"
	"src/Demo.h"
	"src/Video.c"
	"src/Video.h"
	"src/Palette.c"
	"src/Palette.h"
	"src/PLC.c"
	"src/PLC.h"
	"src/PaletteCycle.c"
	"src/PaletteCycle.h"
	"src/GM_Sega.c"
	"src/GM_Sega.h"
	"src/GM_Title.c"
	"src/GM_Title.h"
	"src/GM_Level.c"
	"src/GM_Level.h"
	"src/GM_Special.c"
	"src/GM_Special.h"
	"src/Level.c"
	"src/Level.h"
	"src/LevelDraw.c"
	"src/LevelDraw.h"
	"src/LevelScroll.c"
	"src/LevelScroll.h"
	"src/LevelCollision.c"
	"src/LevelCollision.h"
	"src/SpecialStage.c"
	"src/SpecialStage.h"
	"src/HUD.c"
	"src/HUD.h"
	"src/Object.c"
	"src/Object.h"
	
	"src/Object/Sonic.c"
	"src/Object/Sonic.h"
	"src/Object/SpecialSonic.c"
	"src/Object/Signpost.c"
	"src/Object/TitleSonic.c"
	"src/Object/PSB.c"
	"src/Object/GHZBridge.c"
	"src/Object/Crabmeat.c"
	"src/Object/HUD.c"
	"src/Object/BuzzBomber.c"
	"src/Object/Ring.c"
	"src/Object/Monitor.c"
	"src/Object/Explosion.c"
	"src/Object/Chopper.c"
	"src/Object/TitleCard.c"
	"src/Object/Spikes.c"
	"src/Object/ShieldInvincibility.c"
	"src/Object/GameOverCard.c"
	"src/Object/GHZRock.c"
	"src/Object/Motobug.c"
	"src/Object/Spring.c"
	"src/Object/Newtron.c"
	"src/Object/GHZEdge.c"
	"src/Object/Credits.c"
	
	# Backend
	"src/Backend/MegaDrive.c"
	"src/Backend/MegaDrive.h"
	"src/Backend/VDP.c"
	"src/Backend/VDP.h"
	"src/Backend/Joypad.c"
	"src/Backend/Joypad.h"
)

set(RESOURCES
	"Icon"
	
	"Art/SegaREV00"
	"Art/SegaREV01"
	"Art/JapaneseCredits"
	"Art/TitleFG"
	"Art/Sonic"
	"Art/TitleSonic"
	"Art/TitleTM"
	"Art/TitleCard"
	"Art/GameOver"
	"Art/CreditsFont"
	"Art/Lamppost"
	"Art/HUD"
	"Art/HUDLife"
	"Art/Ring"
	"Art/Points"
	"Art/Monitor"
	"Art/Shield"
	"Art/Invincibility"
	"Art/Explosion"
	"Art/GHZStalk"
	"Art/GHZRock"
	"Art/Crabmeat"
	"Art/BuzzBomber"
	"Art/Chopper"
	"Art/Newtron"
	"Art/Motobug"
	"Art/Spikes"
	"Art/SpringH"
	"Art/SpringV"
	"Art/GHZSwing"
	"Art/GHZBridge"
	"Art/GHZLog"
	"Art/GHZBall"
	"Art/GHZWall1"
	"Art/GHZWall2"
	"Art/Bumper"
	"Art/GHZ1"
	"Art/GHZ2"
	"Art/LZ"
	"Art/MZ"
	"Art/SLZ"
	"Art/SYZ"
	"Art/SBZ"
	"Art/Signpost"
	"Art/HiddenBonus"
	"Art/BigFlash"
	"Art/GHZWaterfall"
	"Art/GHZFlowerLarge"
	"Art/GHZFlowerSmall"
	"Art/HUDNum"
	"Art/LifeNum"
	"Art/Text"
	"Art/SSClouds"
	"Art/SSBack"
	"Art/SSWall"
	"Art/SSGoal"
	"Art/SSSpeed"
	"Art/SSRotate"
	"Art/SSLife"
	"Art/SSTwinkle"
	"Art/SSChecker"
	"Art/SSGhost"
	"Art/SSWarp"
	"Art/SSGlass"
	"Art/SSEmerald"
	"Art/SSZone1"
	"Art/SSZone2"
	"Art/SSZone3"
	"Art/SSZone4"
	"Art/SSZone5"
	"Art/SSZone6"
	"Art/SSResultEmerald"
	
	"Tilemap/SegaREV00"
	"Tilemap/SegaREV01"
	"Tilemap/JapaneseCredits"
	"Tilemap/TitleFG"
	
	"Palette/SegaBG"
	"Palette/Title"
	"Palette/LevelSel"
	"Palette/Sonic"
	"Palette/GHZ"
	"Palette/LZ"
	"Palette/MZ"
	"Palette/SYZ"
	"Palette/SLZ"
	"Palette/SBZ1"
	"Palette/Special"
	"Palette/LZWater"
	"Palette/SBZ3"
	"Palette/SBZ3Water"
	"Palette/SBZ2"
	"Palette/SonicLZ"
	"Palette/SonicSBZ"
	"Palette/SSResults"
	"Palette/Continue"
	"Palette/Ending"
	"Palette/Sega1"
	"Palette/Sega2"
	"Palette/TitleCycle"
	"Palette/GHZCycle"
	
	"Animation/Sonic"
	"Animation/Signpost"
	"Animation/TitleSonic"
	"Animation/PSB"
	"Animation/Crabmeat"
	"Animation/BuzzBomber"
	"Animation/BuzzMissile"
	"Animation/Ring"
	"Animation/Monitor"
	"Animation/Chopper"
	"Animation/ShieldInvincibility"
	"Animation/Motobug"
	"Animation/Spring"
	"Animation/Newtron"
	
	"Mappings/Sonic"
	"Mappings/SonicDPLC"
	"Mappings/Signpost"
	"Mappings/TitleSonic"
	"Mappings/PSB"
	"Mappings/GHZBridge"
	"Mappings/Crabmeat"
	"Mappings/HUD"
	"Mappings/BuzzBomber"
	"Mappings/BuzzMissile"
	"Mappings/BuzzExplode"
	"Mappings/RingREV00"
	"Mappings/RingREV01"
	"Mappings/Monitor"
	"Mappings/Explosion"
	"Mappings/Chopper"
	"Mappings/TitleCard"
	"Mappings/Spikes"
	"Mappings/ShieldInvincibility"
	"Mappings/GameOver"
	"Mappings/GHZRock"
	"Mappings/Motobug"
	"Mappings/Spring"
	"Mappings/Newtron"
	"Mappings/GHZEdge"
	"Mappings/Bumper"
	"Mappings/Credits"
	"Mappings/SSRotate"
	"Mappings/SSGlass"
	"Mappings/SSUp"
	"Mappings/SSDown"
	"Mappings/SSEmerald"
	"Mappings/SSWall"
	"Mappings/SSResultEmerald"
	
	"Layout/GHZ1"
	"Layout/GHZ2"
	"Layout/GHZ3"
	"Layout/GHZBG"
	"Layout/LZ1"
	"Layout/LZ2"
	"Layout/LZ3"
	"Layout/LZBG"
	"Layout/MZ1"
	"Layout/MZ1BG"
	"Layout/MZ2"
	"Layout/MZ2BG"
	"Layout/MZ3"
	"Layout/MZ3BG"
	"Layout/SLZ1"
	"Layout/SLZ2"
	"Layout/SLZ3"
	"Layout/SLZBG"
	"Layout/SYZ1"
	"Layout/SYZ2"
	"Layout/SYZ3"
	"Layout/SYZBGREV00"
	"Layout/SYZBGREV01"
	"Layout/SBZ1"
	"Layout/SBZ1BG"
	"Layout/SBZ2"
	"Layout/SBZ2BG"
	"Layout/SBZ3"
	"Layout/Ending"
	
	"ObjectLayout/GHZ1"
	"ObjectLayout/GHZ2"
	"ObjectLayout/GHZ3REV00"
	"ObjectLayout/GHZ3REV01"
	"ObjectLayout/LZ1PF1"
	"ObjectLayout/LZ1PF2"
	"ObjectLayout/LZ1REV00"
	"ObjectLayout/LZ1REV01"
	"ObjectLayout/LZ2"
	"ObjectLayout/LZ2PF1"
	"ObjectLayout/LZ2PF2"
	"ObjectLayout/LZ3PF1"
	"ObjectLayout/LZ3PF2"
	"ObjectLayout/LZ3REV00"
	"ObjectLayout/LZ3REV01"
	"ObjectLayout/MZ1REV00"
	"ObjectLayout/MZ1REV01"
	"ObjectLayout/MZ2"
	"ObjectLayout/MZ3"
	"ObjectLayout/SLZ1"
	"ObjectLayout/SLZ2"
	"ObjectLayout/SLZ3"
	"ObjectLayout/SYZ1"
	"ObjectLayout/SYZ2"
	"ObjectLayout/SYZ3REV00"
	"ObjectLayout/SYZ3REV01"
	"ObjectLayout/SBZ1PF1"
	"ObjectLayout/SBZ1PF2"
	"ObjectLayout/SBZ1PF3"
	"ObjectLayout/SBZ1PF4"
	"ObjectLayout/SBZ1PF5"
	"ObjectLayout/SBZ1PF6"
	"ObjectLayout/SBZ1REV00"
	"ObjectLayout/SBZ1REV01"
	"ObjectLayout/SBZ2"
	"ObjectLayout/SBZ3"
	"ObjectLayout/FZ"
	"ObjectLayout/Ending"
	
	"Map16/GHZ"
	"Map16/LZ"
	"Map16/MZ"
	"Map16/SBZ"
	"Map16/SLZ"
	"Map16/SYZ"
	
	"Map256/GHZ"
	"Map256/LZ"
	"Map256/MZREV00"
	"Map256/MZREV01"
	"Map256/SBZREV00"
	"Map256/SBZREV01"
	"Map256/SLZ"
	"Map256/SYZ"
	
	"CollisionIndex/GHZ"
	"CollisionIndex/LZ"
	"CollisionIndex/MZ"
	"CollisionIndex/SLZ"
	"CollisionIndex/SYZ"
	"CollisionIndex/SBZ"
	
	"Collision/AngleMap"
	"Collision/HeightMap"
	"Collision/WidthMap"
	
	"Demo/IntroGHZ"
	"Demo/IntroMZ"
	"Demo/IntroSYZ"
	"Demo/IntroSS"
	"Demo/EndingGHZ1"
	"Demo/EndingMZ"
	"Demo/EndingSYZ"
	"Demo/EndingLZ"
	"Demo/EndingSLZ"
	"Demo/EndingSBZ1"
	"Demo/EndingSBZ2"
	"Demo/EndingGHZ2"
	
	"SSLayout/1"
	"SSLayout/2"
	"SSLayout/3"
	"SSLayout/4"
	"SSLayout/5REV00"
	"SSLayout/5REV01"
	"SSLayout/6REV00"
	"SSLayout/6REV01"
)

target_include_directories(SoniCPort PRIVATE "src")

##################
# Compiler flags #
##################

# Revision
if(REV01)
	target_compile_definitions(SoniCPort PRIVATE SCP_REV01)
else()
	target_compile_definitions(SoniCPort PRIVATE SCP_REV00)
endif()

# Region
if(JAPANESE)
	target_compile_definitions(SoniCPort PRIVATE SCP_JP)
endif()

# Bug fixes
if(FIX_BUGS)
	target_compile_definitions(SoniCPort PRIVATE SCP_FIX_BUGS)
endif()

# Sanitization
if(SANITIZE)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Og -ggdb3 -fsanitize=address")
endif()

# Splash
if(SPLASH)
	target_compile_definitions(SoniCPort PRIVATE SCP_SPLASH)
	target_sources(SoniCPort PRIVATE
		"src/GM_SSRG.c"
		"src/GM_SSRG.h"
	)
	list(APPEND RESOURCES
		"SSRG/ArtLink"
		"SSRG/ArtMain"
		"SSRG/ArtSonic"
		"SSRG/ArtSquare"
		"SSRG/MapLink"
		"SSRG/MapMain"
		"SSRG/MapSquare"
	)
endif()

# Strip release builds
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")

# Force warnings
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(SoniCPort PRIVATE /W4)
else()
	target_compile_options(SoniCPort PRIVATE -Wall -Wextra -pedantic)
endif()

# Use LTO if requested
if(LTO)
	include(CheckIPOSupported)
	check_ipo_supported(RESULT result)
	if(result)
		set_target_properties(SoniCPort PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
	endif()
endif()

##############################
# Other compile-time defines #
##############################

# Determine endianness
INCLUDE(TestBigEndian)
TEST_BIG_ENDIAN(ENDIAN)
if(ENDIAN)
	target_compile_definitions(SoniCPort PRIVATE SCP_BIG_ENDIAN)
else()
	target_compile_definitions(SoniCPort PRIVATE SCP_LIL_ENDIAN)
endif()

########
# MSVC #
########

# This is messy as hell, and has been replaced by CMAKE_MSVC_RUNTIME_LIBRARY,
# but that's a very recent CMake addition, so we're still doing it this way for now
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND MSVC_LINK_STATIC_RUNTIME)
	# Statically-link the CRT (vcpkg static libs do this)
	foreach(flag_var CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
		if(${flag_var} MATCHES "/MD")
			string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
		endif()
	endforeach()
endif()

# Do some other MSVC fixes
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	# Disable warnings that normally fire up on MSVC when using "unsafe" functions instead of using MSVC's "safe" _s functions
	target_compile_definitions(SoniCPort PRIVATE _CRT_SECURE_NO_WARNINGS)
	
	# Make it so source files are recognized as UTF-8 by MSVC
	target_compile_options(SoniCPort PRIVATE "/utf-8")
	
	# Use `main` instead of `WinMain`
	set_target_properties(SoniCPort PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
endif()

##########################
# Further compiler setup #
##########################

# Force strict C99
set_target_properties(SoniCPort PROPERTIES
	C_STANDARD 99
	C_STANDARD_REQUIRED ON
	C_EXTENSIONS OFF
)

# Set debug executable name
set_target_properties(SoniCPort PROPERTIES DEBUG_OUTPUT_NAME "SoniCPort_debug")

# Set output directory
set(BUILD_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

set_target_properties(SoniCPort PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIRECTORY}
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BUILD_DIRECTORY}
	RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${BUILD_DIRECTORY}
	RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${BUILD_DIRECTORY}
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BUILD_DIRECTORY}
)

############
# Backends #
############

# Find PkgConfig for dependency linking
find_package(PkgConfig QUIET)

if(BACKEND MATCHES "SDL2")
	target_compile_definitions(SoniCPort PRIVATE SCP_BACKEND_SDL2)
	target_sources(SoniCPort PRIVATE
		"src/Backend/SDL2/System.c"
		"src/Backend/SDL2/Render.c"
		"src/Backend/SDL2/Input.c"
	)
	
	# Compile and link SDL2
	set(SDL_SHARED_ENABLED_BY_DEFAULT OFF)
	if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
			set(LIBC ON CACHE INTERNAL "" FORCE)	# Needed to prevent possible 'symbol already defined' errors
	endif()
	add_subdirectory("lib/SDL" EXCLUDE_FROM_ALL)
	target_link_libraries(SoniCPort PRIVATE SDL2-static)
endif()

#######################
# Resource conversion #
#######################

# Build bin2h externally, so it isn't cross-compiled when SoniCPort is (Emscripten, cross-GCC, MinGW on Linux, etc.)
include(ExternalProject)

ExternalProject_Add(bin2h
	SOURCE_DIR "${CMAKE_SOURCE_DIR}/bin2h"
	DOWNLOAD_COMMAND ""
	UPDATE_COMMAND ""
	BUILD_BYPRODUCTS "<INSTALL_DIR>/bin/bin2h"
	CMAKE_ARGS
		-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
		-DCMAKE_BUILD_TYPE=Release
	INSTALL_COMMAND
		${CMAKE_COMMAND} --build . --config Release --target install
)

ExternalProject_Get_Property(bin2h INSTALL_DIR)

add_executable(bin2h_tool IMPORTED)
add_dependencies(bin2h_tool bin2h)
set_target_properties(bin2h_tool PROPERTIES IMPORTED_LOCATION "${INSTALL_DIR}/bin/bin2h")

# Convert resources to header files
foreach(FILENAME IN LISTS RESOURCES)
	set(IN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res")
	set(OUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/Resource")
	get_filename_component(DIRECTORY "${FILENAME}" DIRECTORY)
	add_custom_command(
		OUTPUT "${OUT_DIR}/${FILENAME}.h"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${OUT_DIR}/${DIRECTORY}"
		COMMAND bin2h_tool "${IN_DIR}/${FILENAME}" "${OUT_DIR}/${FILENAME}.h"
		DEPENDS bin2h_tool "${IN_DIR}/${FILENAME}"
		)
	target_sources(SoniCPort PRIVATE "${OUT_DIR}/${FILENAME}.h")
endforeach()

