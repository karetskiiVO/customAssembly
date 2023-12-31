cmake_minimum_required(VERSION 2.8)			# Проверка версии CMake.
											# Если версия установленой программы
											# старее указаной, произайдёт аварийный выход.

project(assembly)							# Название проекта

set(SOURCE_EXE source/compiler.cpp)			# Установка переменной со списком исходников для исполняемого файла
set(SOURCE_LIB source/token_src.cpp
			   source/assembly_translator.cpp
			   source/text_src.cpp)			# Тоже самое, но для библиотеки
			   
set(CMAKE_CXX_FLAGS "-fsanitize=address -lm -g -O0 -Wall -Wextra")

include_directories(includes)
add_library(textlib STATIC ${SOURCE_LIB})	# Создание статической библиотеки

add_executable(compiler ${SOURCE_EXE})		# Создает исполняемый файл

target_link_libraries(compiler textlib)		# Линковка программы с библиотекой



project(virtualMachine)

set(SOURCE_EXE source/virtualmachine.cpp)
set(SOURCE_LIB source/virtualmachine_tools.cpp)
			   
set(CMAKE_CXX_FLAGS "-fsanitize=address -lm -g -O0 -Wall -Wextra")

include_directories(includes)
add_library(virtualmachine_tools STATIC ${SOURCE_LIB})	

add_executable(virtual_machine ${SOURCE_EXE})

target_link_libraries(virtual_machine virtualmachine_tools)


