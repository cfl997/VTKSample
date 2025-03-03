cmake_minimum_required(VERSION 3.18)

project(vtk3D)

find_package(VTK CONFIG QUIET)
if (NOT VTK_FOUND)
  message(FATAL_ERROR "${_vtk_not_found_msg}")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)




set(src
"main.cpp"

)


include_directories(
	${CGRender_ROOT_DIR}/include/CGRenderView
)


link_directories(
	${CGRender_ROOT_DIR}/lib/win64/
)


add_executable(${PROJECT_NAME}  
${src}

)

SET_PROPERTY(TARGET ${PROJECT_NAME} PROPERTY FOLDER "sample")





if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
	set_target_properties(${PROJECT_NAME}
		PROPERTIES
		OUTPUT_NAME ${PROJECT_NAME}D
		)      
endif()


add_dependencies(${PROJECT_NAME} CGRenderView)
# 使用现代 CMake 方式来链接和包含 VTK
target_link_libraries(${PROJECT_NAME} PRIVATE ${VTK_LIBRARIES})

# 设置 include 目录
target_include_directories(${PROJECT_NAME} PRIVATE ${VTK_INCLUDE_DIRS})