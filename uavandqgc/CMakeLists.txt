package(uavandqgc)
SET(CMAKE_C_FLAGS "-fPIC -O0 -Wall")  #c  
SET(CMAKE_CXX_FLAGS "-fPIC -O2 -Wall")  #c++

add_definitions(-D SHARE_LIBRARY)

INCLUDE_DIRECTORIES(
	.
	./src
	./proto
	../protobuf
	../gshare/src
	../vgmysql/src
)
set(SRC_LIST
	src/GSManager.cpp
	src/GSManager.h
	src/ProtoMsg.cpp
	src/ProtoMsg.h
	src/UavManager.cpp
	src/UavManager.h
	proto/das.pb.cc
)

add_library(${PACKAGE_NAME} SHARED ${SRC_LIST})
set_target_properties(
	${PACKAGE_NAME}
	PROPERTIES
	OUTPUT_NAME
	${PACKAGE_NAME}
)
target_link_libraries(${PACKAGE_NAME} protobuf)
target_link_libraries(${PACKAGE_NAME} gshare)
target_link_libraries(${PACKAGE_NAME} vgmysql)

#install(
#	TARGETS
#	${PACKAGE_NAME}
#	#ARCHIVE DESTINATION
#	LIBRARY DESTINATION
#	${PROJECT_BINARY_DIR}
#)
