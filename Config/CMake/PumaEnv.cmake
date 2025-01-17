cmake_minimum_required(VERSION 3.26)

if(NOT DEFINED PUMADIR)
    set(PUMADIR "$ENV{PUMADIR}")
endif()

if(PUMADIR STREQUAL "")
    set(PUMADIR ${IMTCOREDIR}/../Puma)
endif()

message("PUMADIR ${PUMADIR}")

include_directories("${PUMADIR}/AuxInclude/${TARGETNAME}")
include_directories("${PUMADIR}/Include")
include_directories("${PUMADIR}/Impl")

link_directories("${PUMADIR}/Lib/${CMAKE_BUILD_TYPE}_${TARGETNAME}")
