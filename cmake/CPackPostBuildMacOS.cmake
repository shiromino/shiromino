foreach(PACKAGE_FILE ${CPACK_PACKAGE_FILES})
	execute_process(COMMAND xcrun codesign -f -o runtime --timestamp -s "${CPACK_APPLE_CERT_NAME}" --entitlements "${CPACK_ENTITLEMENTS_FILE}" "${PACKAGE_FILE}")
endforeach()
