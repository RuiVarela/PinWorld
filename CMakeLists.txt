cmake_minimum_required(VERSION 3.27)
project(PinWorld)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_BUILD 1)

add_compile_definitions(VERSION_MAJOR=${VERSION_MAJOR})
add_compile_definitions(VERSION_MINOR=${VERSION_MINOR})
add_compile_definitions(VERSION_BUILD=${VERSION_BUILD})

#set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(vendor)

set(APP_FILES
	src/MetaShader.hpp
	src/MetaShaderCpp.cpp
	src/MetaShaderWater.cpp
	src/MetaShaderGif.cpp
	src/MetaShaderPy.cpp
	src/PinMaterial.hpp
	src/PinMaterial.cpp
	src/PinWorld.hpp
	src/PinWorld.cpp
	src/Lang.hpp
	src/Lang.cpp
	src/Text.hpp
	src/Text.cpp
	src/Menu.hpp
	src/Menu.cpp
	src/main.cpp
	vendor/pocketpy.h
	vendor/vendor.cpp
)

if(CMAKE_SYSTEM_NAME STREQUAL Windows)
	#set(APP_FILES ${APP_FILES} app/Platform_win.cpp)

	add_definitions(-D_CRT_SECURE_NO_WARNINGS)

	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/app.rc.in ${CMAKE_CURRENT_BINARY_DIR}/app.rc @ONLY)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/app.ico ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

	# add_executable(${PROJECT_NAME} WIN32 ${APP_FILES} ${CMAKE_CURRENT_BINARY_DIR}/app.rc)
	add_executable(${PROJECT_NAME} ${APP_FILES} ${CMAKE_CURRENT_BINARY_DIR}/app.rc)

	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/EHsc")

	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
	# set(APP_FILES ${APP_FILES} app/Platform_osx.mm)

	set(APP_BUNDLE_IDENTIFIER "pt.demanda.pinworld")
 
	set(MACOSX_BUNDLE_ICON_FILE app.icns)
	set(MACOSX_BUNDLE_COPYRIGHT "Rui Varela")
	set(MACOSX_BUNDLE_GUI_IDENTIFIER ${APP_BUNDLE_IDENTIFIER})
	set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})
	
	set(MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION})
	set(MACOSX_BUNDLE_LONG_VERSION_STRING ${VERSION})
	set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${VERSION})

	add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${APP_FILES})

	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_SOURCE_DIR}/py" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../Resources/py")

		add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_SOURCE_DIR}/gif" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../Resources/gif")

	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
		"${CMAKE_CURRENT_SOURCE_DIR}/assets/app.icns" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../Resources/")

	set_property(TARGET ${PROJECT_NAME} PROPERTY MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/assets/Info.plist.in")

	target_compile_options(${PROJECT_NAME} PRIVATE -x objective-c++)

	target_link_libraries(${PROJECT_NAME} "-framework IOKit")
	target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
	target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()

if (EMSCRIPTEN)
	add_executable(${PROJECT_NAME} ${APP_FILES})

	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -sALLOW_MEMORY_GROWTH")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file py")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file gif")

	# set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line is used to set your executable to build with the emscripten html template so taht you can directly open it.
	
	file(INSTALL py DESTINATION .)
	file(INSTALL gif DESTINATION .)
	file(INSTALL web/index.html DESTINATION .)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC 
	vendor
	${RAYGUI_DIR})
target_link_libraries(${PROJECT_NAME} raylib)

# organize in folders for VS
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${APP_FILES})
