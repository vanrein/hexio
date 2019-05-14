cmake_minimum_required (VERSION 3.7)

project ("hexio" C)

include (FindPkgConfig)

pkg_search_module (PCSCLITE REQUIRED libpcsclite libpcsclite-dev)

# message ("Found pcsclite, ${PCSCLITE_FOUND}")
message ("Found pcsclite includes at ${PCSCLITE_INCLUDE_DIRS}")
# message ("Found pcsclite libraries at ${PCSCLITE_LIBRARIES}")
# message ("Found pcsclite library dirs at ${PCSCLITE_LIBRARY_DIRS}")

include_directories (${PCSCLITE_INCLUDE_DIRS})

add_executable (hexin  hexin.c)
add_executable (hexout hexout.c)
add_executable (devio  devio.c)
add_executable (pcscio pcscio.c)

target_link_libraries (pcscio ${PCSCLITE_LIBRARIES})

install (TARGETS hexin hexout devio pcscio
	RUNTIME DESTINATION /usr/local/bin
)
install (PROGRAMS derdump
	DESTINATION /usr/local/bin
)
