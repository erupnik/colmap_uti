cmake_minimum_required(VERSION 2.8.11)

project(ColmapPrj)

set(THISDIR "/home/er/Documents/d_development/structureless_BA/colmap")

find_package(COLMAP REQUIRED)
# or to require a specific version: find_package(COLMAP 3.4 REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp  -std=c++11")

include_directories(${COLMAP_INCLUDE_DIRS} ${THISDIR})
link_directories(${COLMAP_LINK_DIRS})

add_executable(colmap_sfm colmap_sfm.cpp)
add_executable(colmap_export_features colmap_export_features.cpp) 
add_executable(colmap_import_features colmap_import_features.cpp) 
add_executable(colmap_import_features_batch colmap_import_features_batch.cpp)  
add_executable(colmap_import_Rt colmap_import_Rt.cpp)
add_executable(colmap_transfer_features colmap_transfer_features.cpp) 
add_executable(colmap_modify_intrinsics colmap_modify_intrinsics.cpp)
add_executable(colmap_add_camera colmap_add_camera.cpp)   
add_executable(colmap_export_Rt colmap_export_Rt.cpp)


target_link_libraries(colmap_sfm ${COLMAP_LIBRARIES})
target_link_libraries(colmap_export_features ${COLMAP_LIBRARIES})
target_link_libraries(colmap_import_features ${COLMAP_LIBRARIES})
target_link_libraries(colmap_import_features_batch ${COLMAP_LIBRARIES})
target_link_libraries(colmap_import_Rt ${COLMAP_LIBRARIES})
target_link_libraries(colmap_transfer_features ${COLMAP_LIBRARIES})
target_link_libraries(colmap_modify_intrinsics ${COLMAP_LIBRARIES})
target_link_libraries(colmap_add_camera ${COLMAP_LIBRARIES})
target_link_libraries(colmap_export_Rt ${COLMAP_LIBRARIES})

install(TARGETS colmap_sfm colmap_export_features colmap_import_features colmap_import_features_batch colmap_import_Rt colmap_transfer_features colmap_modify_intrinsics colmap_add_camera colmap_export_Rt DESTINATION "bin")
