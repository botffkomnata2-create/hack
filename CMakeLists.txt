cmake_minimum_required(VERSION 3.10)
project(FreeFireMod)

set(CMAKE_CXX_STANDARD 17)

# Создаем динамическую библиотеку .dylib из main.cpp
add_library(FreeFireMod SHARED main.cpp)

# Убираем лишние префиксы для macOS/iOS
set_target_properties(FreeFireMod PROPERTIES PREFIX "")
