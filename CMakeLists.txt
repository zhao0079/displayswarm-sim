cmake_minimum_required(VERSION 2.6)

project(displayswarm-interface)

# Dashel
# TODO: make Dashel.cmake to perform this

find_path(DASHEL_INCLUDE_DIR dashel.h
	/usr/include/dashel
	/usr/local/include/dashel
)

find_library(DASHEL_LIBRARY dashel
	/usr/lib
	/usr/local/lib
)

include_directories(${DASHEL_INCLUDE_DIR})
set(EXTRA_LIBS "")

# libudev
find_path(UDEV_INCLUDE_DIR libudev.h)
find_library(UDEV_LIBS udev)

if (UDEV_INCLUDE_DIR AND UDEV_LIBS)
	message(STATUS "libudev found")
	set(UDEV_FOUND 1)
endif (UDEV_INCLUDE_DIR AND UDEV_LIBS)

if (WIN32)
	set(EXTRA_LIBS ${EXTRA_LIBS} winspool ws2_32 setupapi)
else (WIN32)
	if (APPLE)
		find_library(COREFOUNDATION_LIBRARY CoreFoundation)
		find_library(IOKIT_LIBRARY IOKit)
		mark_as_advanced(COREFOUNDATION_LIBRARY)
		mark_as_advanced(IOKIT_LIBRARY)
		set(EXTRA_LIBS ${EXTRA_LIBS} ${COREFOUNDATION_LIBRARY} ${IOKIT_LIBRARY} ${EXTRA_LIBS})
	else (APPLE)
                if (UDEV_FOUND)
                        include_directories(${UDEV_INCLUDE_DIR})
			set(EXTRA_LIBS ${EXTRA_LIBS} ${UDEV_LIBS} ${EXTRA_LIBS})
                else (UDEV_FOUND)
			find_package(PkgConfig)
                        pkg_check_modules(HAL hal)
                        if (${HAL_FOUND})
                                include_directories(${HAL_INCLUDE_DIRS})
				set(EXTRA_LIBS ${EXTRA_LIBS} ${HAL_LIBRARIES} ${EXTRA_LIBS})
                        endif (${HAL_FOUND})
                endif (UDEV_FOUND)
	endif (APPLE)
endif (WIN32)

# Enki
find_path(ENKI_INCLUDE_DIR enki/PhysicalEngine.h
	/usr/include/enki
	/usr/local/include/enki
)

find_library(ENKI_LIBRARY enki
	/usr/lib
	/usr/local/lib
)

find_library(ENKI_VIEWER_LIBRARY enkiviewer
	/usr/lib
	/usr/local/lib
)
include_directories(${ENKI_INCLUDE_DIR})

# Qt
find_package(Qt4 COMPONENTS QtOpenGL QtGui QtCore REQUIRED)
include(${QT_USE_FILE})
find_package(OpenGL REQUIRED)

# build and link
add_executable(dss WIN32 displayswarm-sim.cpp)
if (WIN32)
	target_link_libraries(dss ${ENKI_VIEWER_LIBRARY} ${ENKI_LIBRARY} ${QT_LIBRARIES} ${QT_QTOPENGL_LIBRARIES} ${QT_QTMAIN_LIBRARY} ${EXTRA_LIBS} ${OPENGL_LIBRARIES} ${DASHEL_LIBRARY})
else (WIN32)
	target_link_libraries(dss ${ENKI_VIEWER_LIBRARY} ${ENKI_LIBRARY} ${QT_LIBRARIES} ${QT_QTOPENGL_LIBRARIES} ${EXTRA_LIBS} ${OPENGL_LIBRARIES} ${DASHEL_LIBRARY})
endif (WIN32)
