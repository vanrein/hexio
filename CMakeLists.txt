cmake_minimum_required (VERSION 3.7)

project ("hexio" C)

include (FindPkgConfig)

pkg_search_module (PCSCLITE libpcsclite libpcsclite-dev)

if (PCSCLITE_FOUND)
	# message ("Found pcsclite, ${PCSCLITE_FOUND}")
	message ("Found pcsclite includes at ${PCSCLITE_INCLUDE_DIRS}")
	# message ("Found pcsclite libraries at ${PCSCLITE_LIBRARIES}")
	# message ("Found pcsclite library dirs at ${PCSCLITE_LIBRARY_DIRS}")
	include_directories (${PCSCLITE_INCLUDE_DIRS})
	set (PCSC_OPT_TARGET pcscio)
else()
	set (PCSC_OPT_TARGET)
endif ()

add_executable (hexin  hexin.c)
add_executable (hexout hexout.c)
add_executable (devio  devio.c)
if (PCSCLITE_FOUND)
	add_executable (pcscio pcscio.c)
	target_link_libraries (pcscio ${PCSCLITE_LIBRARIES})
endif ()

install (TARGETS hexin hexout devio ${PCSC_OPT_TARGET}
	RUNTIME DESTINATION /usr/local/bin
)
install (PROGRAMS derdump
	DESTINATION /usr/local/bin
)
