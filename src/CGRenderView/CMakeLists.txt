cmake_minimum_required(VERSION 3.18)

project(CGRenderView)

# Display VTK version
find_package(VTK CONFIG QUIET)
if (NOT VTK_FOUND)
  message(FATAL_ERROR "${_vtk_not_found_msg}")
endif()
#include(${VTK_USE_FILE})


#set(KIT_LIBS ${VTK_LIBRARIES})



SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CGRender_ROOT_DIR}/bin64/")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CGRender_ROOT_DIR}/bin64/")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CGRender_ROOT_DIR}/bin64/")


add_definitions(-DCGRENDERVIEWEXPORT)


include_directories(
	${CGRender_ROOT_DIR}/include
	${CGRender_ROOT_DIR}/include/CGRenderView
)



set(COMMON_HEAD_PUBLIC
	${CGRender_ROOT_DIR}/include/CGRenderView/CGRenderView.h
	${CGRender_ROOT_DIR}/include/CGRenderView/CGWindowsWindow.h
	${CGRender_ROOT_DIR}/include/CGRenderView/CGRenderView_Export.h
)
set(COMMON_INCLUDE
	VTKInclude.h
	VTKReader3D.h
)

set(commom_src
	CGRenderView.cpp
	CGWindowsWindow.cpp
	VTKReader3D.cpp
)



add_library(${PROJECT_NAME}  SHARED
	${COMMON_HEAD_PUBLIC}
	${commom_src}
	${COMMON_INCLUDE}
)




# 使用现代 CMake 方式来链接和包含 VTK
target_link_libraries(${PROJECT_NAME} PRIVATE ${VTK_LIBRARIES})

# 设置 include 目录
target_include_directories(${PROJECT_NAME} PRIVATE ${VTK_INCLUDE_DIRS})


source_group("Public" FILES ${COMMON_HEAD_PUBLIC})
