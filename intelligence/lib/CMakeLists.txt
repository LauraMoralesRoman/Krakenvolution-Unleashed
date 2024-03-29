file(GLOB FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
	"${CMAKE_CURRENT_SOURCE_DIR}/*"
)

set(MODULE_DIRS "")
set(TMP_ALL_LIBS "")

foreach(DIR ${FILES})
	set(FULL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${DIR}")
	if(IS_DIRECTORY ${FULL_PATH})
		list(APPEND MODULE_DIRS ${DIR})
	endif()
endforeach()


# Build each module
message("\n${Green}Found the following lib modules: ${Reset}")
foreach(MOD ${MODULE_DIRS})
    get_filename_component(MOD_NAME ${MOD} NAME)

    # Path to the module's CMakeLists.txt
    set(MOD_CMAKE_LISTS "${CMAKE_CURRENT_SOURCE_DIR}/${MOD}/CMakeLists.txt")

	file(GLOB_RECURSE MOD_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${MOD}/*.cpp")

	if (EXISTS "${MOD_CMAKE_LISTS}")
		message("- ${Yellow}${MOD_NAME}${Reset}${BoldYellow} (included cmake)${Reset}")
	else()
		message("- ${Yellow}${MOD_NAME}${Reset}")
	endif()

	if(MOD_SOURCES)
		foreach(SOURCE ${MOD_SOURCES})
			get_filename_component(REL_SOURCE ${SOURCE} NAME)
			message("    - ${REL_SOURCE}")
			set_source_files_properties(${SOURCE} PROPERTIES LANGUAGE CXX)
		endforeach()

		# Add the library
		add_library(${MOD_NAME} STATIC ${MOD_SOURCES})

		if(EXISTS "${MOD_CMAKE_LISTS}")
			include("${MOD_CMAKE_LISTS}")
		endif()

		list(APPEND TMP_ALL_LIBS "${MOD_NAME}")
	else()
		message("    ${Blue}No sources were found")
	endif()
endforeach()
message("")

set(ALL_LIBS "${TMP_ALL_LIBS}" PARENT_SCOPE)
