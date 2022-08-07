function(InstallRuntimeDependenciesMacOS
        EXE
        EXE_INSTALLDIR
        LIBS_INSTALLDIR
        LIBDIR
        APPLE_CERT_NAME
        ENTITLEMENTS_ADHOC
        ENTITLEMENTS_IDENTITY_REQUIRED
)
        set_target_properties(${EXE} PROPERTIES
                INSTALL_RPATH @executable_path
        )
        include(CMakeFindBinUtils)
        install(CODE
                "
                set(POST_EXCLUDES
                \"^/usr/lib\"
                \"^/System/Library/Frameworks\"
                )

                file(GET_RUNTIME_DEPENDENCIES
                        EXECUTABLES \$<TARGET_FILE:${EXE}>
                        POST_EXCLUDE_REGEXES
                                \${POST_EXCLUDES}
                        RESOLVED_DEPENDENCIES_VAR RESOLVED_DEPENDENCIES
                        UNRESOLVED_DEPENDENCIES_VAR UNRESOLVED_DEPENDENCIES
                )

                file(INSTALL
                        DESTINATION \"${LIBS_INSTALLDIR}/${LIBDIR}\"
                        TYPE SHARED_LIBRARY
                        FOLLOW_SYMLINK_CHAIN
                        FILES \${RESOLVED_DEPENDENCIES}
                )

                set(CHANGES \"\")
                set(LIBS \"\")
                foreach(RESOLVED_DEPENDENCY \${RESOLVED_DEPENDENCIES})
                        string(REGEX REPLACE \"^.*/([^/]+)\" \"\\\\1\" LIB \"\${RESOLVED_DEPENDENCY}\")
                        list(APPEND CHANGES -change \"\${RESOLVED_DEPENDENCY}\" \"@rpath/${LIBDIR}/\${LIB}\")
                        list(APPEND LIBS \"\${LIB}\")
                endforeach()

                if(\"${APPLE_CERT_NAME}\" STREQUAL -)
                        set(ENTITLEMENTS \"${ENTITLEMENTS_ADHOC}\")
                else()
                        set(ENTITLEMENTS \"${ENTITLEMENTS_IDENTITY_REQUIRED}\")
                endif()

                foreach(LIB \${LIBS})
                        execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} \${CHANGES} -id \"@rpath/${LIBDIR}/\${LIB}\" \"${LIBS_INSTALLDIR}/${LIBDIR}/\${LIB}\")
                        execute_process(COMMAND xcrun codesign --remove-signature \"${LIBS_INSTALLDIR}/${LIBDIR}/\${LIB}\")
                        execute_process(COMMAND xcrun codesign -f -o runtime --timestamp -s \"${APPLE_CERT_NAME}\" --entitlements \"\${ENTITLEMENTS}\" \"${LIBS_INSTALLDIR}/${LIBDIR}/\${LIB}\")
                endforeach()
                execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} \${CHANGES} \"${EXE_INSTALLDIR}/${EXE}\")
                execute_process(COMMAND xcrun codesign --remove-signature \"${EXE_INSTALLDIR}/${EXE}\")
                execute_process(COMMAND xcrun codesign -f -o runtime --timestamp -s \"${APPLE_CERT_NAME}\" --entitlements \"\${ENTITLEMENTS}\" \"${EXE_INSTALLDIR}/${EXE}\")
                "
        )
endfunction()