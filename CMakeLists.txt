cmake_minimum_required(VERSION 2.8)			# Проверка версии CMake.
											# Если версия установленой программы
											# старее указаной, произайдёт аварийный выход.

project(assembly)							# Название проекта

set(SOURCE_EXE main.cpp)					# Установка переменной со списком исходников для исполняемого файла
set(SOURCE_LIB source/token_src.cpp
			   source/text_src.cpp)			# Тоже самое, но для библиотеки
set(CMAKE_CXX_FLAGS "-fsanitize=address -lm -g -O0 -Wall -Wextra")

include_directories(includes)
add_library(textlib STATIC ${SOURCE_LIB})	# Создание статической библиотеки с именем foo

add_executable(solution ${SOURCE_EXE})			# Создает исполняемый файл с именем main

target_link_libraries(solution textlib)			# Линковка программы с библиотекой
