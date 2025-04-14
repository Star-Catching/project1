cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(MyImGuiApp)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 添加 ImGui 和 GLFW 库的路径
# 假设你已经将 imgui 和 glfw 放在项目的 libs 目录下
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/libs/imgui)
set(GLFW_DIR ${CMAKE_SOURCE_DIR}/libs/glfw)
set(OPENCV_DIR "/path/to/your/opencv")  # 请替换为实际的 OpenCV 安装路径

# 包含头文件目录
include_directories(${IMGUI_DIR})
include_directories(${GLFW_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/libs)
include_directories(${OPENCV_DIR}/include)  # 加入 OpenCV 的 include 目录

# 查找 OpenGL 和 OpenCV 库
find_package(OpenGL REQUIRED)
find_package(OpenCV REQUIRED)  # 查找 OpenCV 库

# 收集 ImGui 和 GLFW 的源文件
file(GLOB IMGUI_SOURCES "${IMGUI_DIR}/*.cpp")
file(GLOB GLFW_SOURCES "${GLFW_DIR}/src/*.c")

# 添加 ImGui 的实现文件（backends 目录下）
file(GLOB IMGUI_IMPL_SOURCES "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp" "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp")

# 创建可执行文件
add_executable(MyImGuiApp src/main.cpp ${IMGUI_SOURCES} ${GLFW_SOURCES} ${IMGUI_IMPL_SOURCES})

# 链接 OpenGL 库、GLFW、OpenCV 和其他依赖库
target_link_libraries(MyImGuiApp OpenGL::GL glfw dl ${OpenCV_LIBS})  # 添加 OpenCV 库
