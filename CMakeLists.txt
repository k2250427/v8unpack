cmake_minimum_required (VERSION 2.8)
project (v8unpack)

set (v8unpack_VERSION_MAJOR 3)
set (v8unpack_VERSION_MINOR 0)

if(WIN32)
	set(V8UNPACK_RC_FILE "src/v8unpack/VersionInfo.rc")
	set(V8VERSIONS_RC_FILE "src/v8versions/VersionInfo.rc")
else()
	set(V8UNPACK_RC_FILE "")
	set(V8VERSIONS_RC_FILE "")
endif()

if(MSVC)

	foreach(flag_var
		CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
		CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)

		if(${flag_var} MATCHES "/MD")
			string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
		endif(${flag_var} MATCHES "/MD")

	endforeach(flag_var)

	SET( CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} "/NODEFAULTLIBS:MSVCRT" )

endif()

set(CMAKE_CXX_STANDARD 11)
add_executable (v8unpack src/v8unpack/main.cpp src/v8unpack/V8File.cpp ${V8UNPACK_RC_FILE})
add_executable (v8versions src/v8versions/main.cxx src/v8versions/versions.cxx ${V8VERSIONS_RC_FILE})

add_definitions (-DBOOST_ALL_NO_LIB)
set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_MULTITHREADED OFF)
set (Boost_USE_STATIC_RUNTIME ON)

find_package (Boost 1.53 REQUIRED COMPONENTS filesystem system)

include_directories (${Boost_INCLUDE_DIRS})
target_link_libraries (v8unpack ${Boost_LIBRARIES})
target_link_libraries (v8versions ${Boost_LIBRARIES})


find_package (ZLIB REQUIRED)

include_directories (${ZLIB_INCLUDE_DIRS})
target_link_libraries (v8unpack ${ZLIB_LIBRARIES})
