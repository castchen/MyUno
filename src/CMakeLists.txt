cmake_minimum_required(VERSION 2.8)
project(uno)
aux_source_directory(. DIR_SRCS)
aux_source_directory(./log LOG_SRCS)
aux_source_directory(./proto PROTO_SRCS)
set(ALL_SRCS
		${DIR_SRCS}
		${LOG_SRCS}
		${PROTO_SRCS}
	)

add_executable(uno ${ALL_SRCS})

add_definitions(-std=c++11 -ggdb)

execute_process(COMMAND mysql_config --cflags
    OUTPUT_VARIABLE MYSQL_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND mysql_config --libs
    OUTPUT_VARIABLE MYSQL_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)

target_compile_options(uno PUBLIC ${MYSQL_CFLAGS})
target_link_libraries(uno ${MYSQL_LIBS} protobuf pthread)


#`mysql_config --cflags --libs`
# protoc -I=$SRC_DIR --cpp_out=$DST_DIR $SRC_DIR/addressbook.proto
# -lprotobuf -lpthread