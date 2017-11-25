cmake_minimum_required(VERSION 3.1.0)
PROJECT(allinone CXX C)
set (CMAKE_CXX_STANDARD 11)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")
set(BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin")
# set(RULES_INSTALL_DIR "/etc/udev/rules.d")


find_package(INDI REQUIRED)
find_package(Threads REQUIRED)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

include_directories( ${CMAKE_CURRENT_BINARY_DIR})
include_directories( ${CMAKE_SOURCE_DIR})
include_directories( ${CMAKE_SOURCE_DIR}/../ucontroler)

include_directories( ${INDI_INCLUDE_DIR})

set(indiallinone_SRCS
   ${CMAKE_SOURCE_DIR}/allinone.cpp
   ${CMAKE_SOURCE_DIR}/Lock.cpp
   ${CMAKE_SOURCE_DIR}/../ucontroler/ReadBuffer.cpp
   ${CMAKE_SOURCE_DIR}/../ucontroler/WriteBuffer.cpp
   ${CMAKE_SOURCE_DIR}/../ucontroler/BinSerialReadBuffer.cpp
   ${CMAKE_SOURCE_DIR}/../ucontroler/BinSerialWriteBuffer.cpp
   ${CMAKE_SOURCE_DIR}/../ucontroler/IndiDevice.cpp
   ${CMAKE_SOURCE_DIR}/../ucontroler/IndiProtocol.cpp
   ${CMAKE_SOURCE_DIR}/../ucontroler/IndiVector.cpp
   ${CMAKE_SOURCE_DIR}/../ucontroler/IndiVectorMember.cpp
   ${CMAKE_SOURCE_DIR}/../ucontroler/IndiTextVector.cpp
   ${CMAKE_SOURCE_DIR}/../ucontroler/IndiTextVectorMember.cpp
   ${CMAKE_SOURCE_DIR}/../ucontroler/IndiNumberVector.cpp
   ${CMAKE_SOURCE_DIR}/../ucontroler/IndiNumberVectorMember.cpp
   ${CMAKE_SOURCE_DIR}/../ucontroler/IndiIntVectorMember.cpp
   ${CMAKE_SOURCE_DIR}/../ucontroler/IndiFloatVectorMember.cpp
   ${CMAKE_SOURCE_DIR}/../ucontroler/IndiVectorGroup.cpp
   )

add_executable(indi_allinone ${indiallinone_SRCS})

target_link_libraries(indi_allinone ${INDI_DRIVER_LIBRARIES})
target_link_libraries(indi_allinone Threads::Threads)

install(TARGETS indi_allinone RUNTIME DESTINATION bin )

install(FILES indi_allinone.xml DESTINATION ${INDI_DATA_DIR})
# install(FILES 99-qsi.rules DESTINATION ${RULES_INSTALL_DIR})