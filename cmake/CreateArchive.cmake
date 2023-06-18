cmake_minimum_required(VERSION 3.19)

if(NOT DEFINED ARCHIVE)
	message(FATAL_ERROR "No archive filename specified; provide the archive filename as the CMake variable ARCHIVE (-D ARCHIVE=\"<filenam>\").")
elseif(NOT DEFINED PATHS)
	message(FATAL_ERROR "No paths specified to archive; provide the list of semicolon-separated paths as the CMake variable PATHS (-D PATHS=\"<path1>;<path2>;...\").")
endif()

if(NOT DEFINED FORMAT)
	set(FORMAT zip)
elseif(
	NOT (
		FORMAT STREQUAL 7zip OR
		FORMAT STREQUAL gnutar OR
		FORMAT STREQUAL pax OR
		FORMAT STREQUAL paxr OR
		FORMAT STREQUAL raw OR
		FORMAT STREQUAL zip
	)
)
	message(FATAL_ERROR "The specified archive format is invalid; only 7zip, gnutar, pax, paxr, raw, and zip are supported.")
endif()

if(
	(
		FORMAT STREQUAL gnutar OR
		FORMAT STREQUAL pax OR
		FORMAT STREQUAL paxr OR
		FORMAT STREQUAL raw
	) AND DEFINED COMPRESSION
)
	if(
		NOT (
			COMPRESSION STREQUAL None OR
			COMPRESSION STREQUAL BZip2 OR
			COMPRESSION STREQUAL GZip OR
			COMPRESSION STREQUAL XZ OR
			COMPRESSION STREQUAL Zstd
		)
	)
		message(FATAL_ERROR "The specified type of compression is invalid; only None, BZip2, GZip, XZ, or Zstd are supported.")
	endif()

	file(ARCHIVE_CREATE OUTPUT "${ARCHIVE}"
		PATHS ${PATHS}
		FORMAT "${FORMAT}"
		COMPRESSION "${COMPRESSION}"
	)
else()
	file(ARCHIVE_CREATE OUTPUT "${ARCHIVE}"
		PATHS ${PATHS}
		FORMAT "${FORMAT}"
	)
endif()
