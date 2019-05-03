cmake_minimum_required (VERSION 2.6)

project (embeddingapitests)
include_directories( ${INC_PLATFORM_DIR} )
include_directories( "../../../../src/inc/embeddingapi" )

set(SOURCES
    embeddingapitests.cpp)

# add the executable
add_executable (embeddingapitests ${SOURCES})
target_link_libraries(embeddingapitests ${LINK_LIBRARIES_ADDITIONAL})

# add the install targets
install (TARGETS embeddingapitests DESTINATION bin)
