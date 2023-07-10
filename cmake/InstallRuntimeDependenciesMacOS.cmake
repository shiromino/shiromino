set_target_properties(${GAME_EXECUTABLE} PROPERTIES
        INSTALL_RPATH @executable_path
)
include(CMakeFindBinUtils)
install(CODE "

set(POST_EXCLUDES
\"^/usr/lib\"
\"^/System/Library/Frameworks\"
)

file(GET_RUNTIME_DEPENDENCIES
        EXECUTABLES \$<TARGET_FILE:${GAME_EXECUTABLE}>
        POST_EXCLUDE_REGEXES
                \${POST_EXCLUDES}
        RESOLVED_DEPENDENCIES_VAR RESOLVED_DEPENDENCIES
        UNRESOLVED_DEPENDENCIES_VAR UNRESOLVED_DEPENDENCIES
)

if(NOT \"\${RESOLVED_DEPENDENCIES}\" STREQUAL \"\")
	file(INSTALL
		DESTINATION \"\${CMAKE_INSTALL_PREFIX}/${GAME_EXECUTABLE}/${GAME_EXECUTABLE}.app/Contents/libs\"
		TYPE SHARED_LIBRARY
		FOLLOW_SYMLINK_CHAIN
		FILES \${RESOLVED_DEPENDENCIES}
	)

	set(CHANGES \"\")
	set(LIBS \"\")
	foreach(RESOLVED_DEPENDENCY \${RESOLVED_DEPENDENCIES})
		string(REGEX REPLACE \"^.*/([^/]+)\" \"\\\\1\" LIB \"\${RESOLVED_DEPENDENCY}\")
		list(APPEND CHANGES -change \"\${RESOLVED_DEPENDENCY}\" \"@rpath/../libs/\${LIB}\")
		list(APPEND LIBS \"\${LIB}\")
	endforeach()

	foreach(LIB \${LIBS})
		execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} \${CHANGES} -id \"@rpath/../libs/\${LIB}\" \"\${CMAKE_INSTALL_PREFIX}/${GAME_EXECUTABLE}/${GAME_EXECUTABLE}.app/Contents/libs/\${LIB}\")
		execute_process(COMMAND xcrun codesign --remove-signature \"\${CMAKE_INSTALL_PREFIX}/${GAME_EXECUTABLE}/${GAME_EXECUTABLE}.app/Contents/libs/\${LIB}\")
		execute_process(COMMAND xcrun codesign -f -o runtime --timestamp -s \"${CPACK_APPLE_CERT_NAME}\" --entitlements \"${CPACK_ENTITLEMENTS_FILE}\" \"\${CMAKE_INSTALL_PREFIX}/${GAME_EXECUTABLE}/${GAME_EXECUTABLE}.app/Contents/libs/\${LIB}\")
	endforeach()
	execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} \${CHANGES} \"\${CMAKE_INSTALL_PREFIX}/${GAME_EXECUTABLE}/${GAME_EXECUTABLE}.app/Contents/MacOS/${GAME_EXECUTABLE}\")
endif()

execute_process(COMMAND xcrun codesign --remove-signature \"\${CMAKE_INSTALL_PREFIX}/${GAME_EXECUTABLE}/${GAME_EXECUTABLE}.app\")
execute_process(COMMAND xcrun codesign -f -o runtime --timestamp -s \"${CPACK_APPLE_CERT_NAME}\" --entitlements \"${CPACK_ENTITLEMENTS_FILE}\" \"\${CMAKE_INSTALL_PREFIX}/${GAME_EXECUTABLE}/${GAME_EXECUTABLE}.app\")

" COMPONENT Game)
