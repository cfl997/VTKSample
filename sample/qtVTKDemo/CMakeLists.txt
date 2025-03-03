cmake_minimum_required(VERSION 3.18)

project(qtVTKDemo)


# 添加编译器选项
if (MSVC)
    add_compile_options(/permissive-)
endif()
if(WIN32)
	if (QTDIR OR DEFINED ENV{QTDIR} OR DEFINED ENV{QTDIR32} OR DEFINED ENV{QTDIR64})
		# Qt path set by user or env var
	else()
		set(QTDIR "C:/Qt/6.6.3/msvc2019_64" CACHE PATH "Path to Qt (e.g. C:/Qt/5.7/msvc2015_64)")
		#message(WARNING "QTDIR variable is missing.  Please set this variable to specify path to Qt (e.g. C:/Qt/5.7/msvc2015_64)")
	endif()
endif()

if(DEFINED QTDIR${_lib_suffix})
	list(APPEND CMAKE_PREFIX_PATH "${QTDIR${_lib_suffix}}")
	#message("aaa1" ${_lib_suffix})
elseif(DEFINED QTDIR)
	list(APPEND CMAKE_PREFIX_PATH "${QTDIR}")
elseif(DEFINED ENV{QTDIR${_lib_suffix}})
	list(APPEND CMAKE_PREFIX_PATH "$ENV{QTDIR${_lib_suffix}}")
elseif(DEFINED ENV{QTDIR})
	list(APPEND CMAKE_PREFIX_PATH "$ENV{QTDIR}")
endif()


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)



find_package(VTK CONFIG QUIET)
if (NOT VTK_FOUND)
  message(FATAL_ERROR "${_vtk_not_found_msg}")
endif()


# 指定UI文件所在目录
set(ui_files
    "QtDisplayWidget.ui"
    # 添加其他UI文件，如果有多个的话
)

set(includes 
"QtDisplayWidget.h"
)

set(src
"main.cpp"
"QtDisplayWidget.cpp"
)
find_package(Qt6Core REQUIRED)
find_package(Qt6Widgets REQUIRED)
find_package(Qt6UiTools REQUIRED)


include_directories(
	${CGRender_ROOT_DIR}/include/CGRenderView
)


link_directories(
	${CGRender_ROOT_DIR}/lib/win64/
)


add_executable(${PROJECT_NAME}  
${src}
${includes}
${ui_files}
)

# 设置 include 目录
target_include_directories(${PROJECT_NAME} PRIVATE ${VTK_INCLUDE_DIRS})

# 使用现代 CMake 方式来链接和包含 VTK
target_link_libraries(${PROJECT_NAME} PRIVATE ${VTK_LIBRARIES})


target_link_libraries(${PROJECT_NAME} PRIVATE  Qt6::Widgets Qt6::Core Qt6::UiTools )


SET_PROPERTY(TARGET ${PROJECT_NAME} PROPERTY FOLDER "sample")

# 使用Qt的UI文件生成器生成对应的C++代码
qt6_wrap_ui(ui_generated_files ${ui_files} OPTIONS -o ${CMAKE_CURRENT_BINARY_DIR}/Generated_uis)

# 将UI文件目录添加到包含目录中，确保可以找到生成的头文件
include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated_uis)

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
	set_target_properties(${PROJECT_NAME}
		PROPERTIES
		OUTPUT_NAME ${PROJECT_NAME}D
		)      
endif()




add_dependencies(${PROJECT_NAME} CGRenderView)
