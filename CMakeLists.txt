# Ampersand Project
# Copyright (C) 2025, Bruce MacKinnon KC1FSZ
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# ( at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
cmake_minimum_required(VERSION 3.13)
project(ampersand C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)
add_compile_options(-fstack-protector-all -Wall -Wpedantic -g)

# ----- Custom work needed to use  libpiper ----------------------------------

# A pre-build version of the libpiper run-time has been created to avoid the 
# complexity of building it on the fly.
# TODO: Figure out how to remove this.
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/libpiper-${CMAKE_HOST_SYSTEM_PROCESSOR}/include
  COMMAND wget https://ampersand-asl.s3.us-west-1.amazonaws.com/releases/libpiper-${CMAKE_HOST_SYSTEM_PROCESSOR}.tar.gz -O ${CMAKE_CURRENT_BINARY_DIR}/libpiper-${CMAKE_HOST_SYSTEM_PROCESSOR}.tar.gz
  COMMAND ${CMAKE_COMMAND} -E tar xf ${CMAKE_CURRENT_BINARY_DIR}/libpiper-${CMAKE_HOST_SYSTEM_PROCESSOR}.tar.gz
  COMMENT "Unpacking libpiper"
  VERBATIM
)
add_custom_target(libpiper_untar DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/libpiper-${CMAKE_HOST_SYSTEM_PROCESSOR}/include)

# ----- ASL Parrot (Network) --------------------------------------------------

add_executable(asl-parrot
  src/main.cpp
  src/service-thread.cpp
  src/api-thread.cpp
  amp-core/src/EventLoop.cpp
  amp-core/src/Message.cpp
  amp-core/src/Line.cpp
  amp-core/src/LineIAX2.cpp
  amp-core/src/IAX2FrameFull.cpp
  amp-core/src/IAX2Util.cpp
  amp-core/src/RegisterTask.cpp
  amp-core/src/StatsTask.cpp
  amp-core/src/ManagerTask.cpp
  amp-core/src/RetransmissionBufferStd.cpp
  amp-core/src/Transcoder_G711_ULAW.cpp
  amp-core/src/Transcoder_SLIN_48K.cpp
  amp-core/src/Transcoder_SLIN_16K.cpp
  amp-core/src/Transcoder_SLIN_8K.cpp
  amp-core/src/Transcoder_G726.cpp
  amp-core/src/Bridge.cpp
  amp-core/src/BridgeCall.cpp
  amp-core/src/BridgeIn.cpp
  amp-core/src/BridgeOut.cpp
  amp-core/src/Resampler.cpp
  amp-core/src/ThreadUtil.cpp
  amp-core/src/MultiRouter.cpp
  amp-core/src/KerchunkFilter.cpp
  amp-core/src/Poker.cpp
  amp-core/src/TTSService.cpp
  amp-core/src/ProgramUtils.cpp
  amp-core/src/LineParrot.cpp
  kc1fsz-tools-cpp/src/Common.cpp
  kc1fsz-tools-cpp/src/NetUtils.cpp
  kc1fsz-tools-cpp/src/DTMFDetector2.cpp
  kc1fsz-tools-cpp/src/MicroDNS.cpp
  kc1fsz-tools-cpp/src/StdPollTimer.cpp
  kc1fsz-tools-cpp/src/fixed_math.cpp
  kc1fsz-tools-cpp/src/linux/StdClock.cpp
  kc1fsz-tools-cpp/src/linux/MTLog.cpp
  kc1fsz-tools-cpp/src/md5/md5c.c
  itu-g711-codec/src/codec.cpp
  itu-g711-codec/src/Plc.cpp
  cmsis-dsp-mock/src/main.cpp
  ed25519/src/add_scalar.c
  ed25519/src/ge.c
  ed25519/src/keypair.c
  ed25519/src/seed.c
  ed25519/src/sign.c
  ed25519/src/fe.c
  ed25519/src/key_exchange.c
  ed25519/src/sc.c
  ed25519/src/sha512.c
  ed25519/src/verify.c
  g726-codec/src/g711.c
  g726-codec/src/g72x.c
  g726-codec/src/g726_32.c
)
add_dependencies(asl-parrot libpiper_untar)

target_include_directories(asl-parrot PRIVATE src)
target_include_directories(asl-parrot PRIVATE include)
target_include_directories(asl-parrot PRIVATE kc1fsz-tools-cpp/include)
target_include_directories(asl-parrot PRIVATE itu-g711-codec/src)
target_include_directories(asl-parrot PRIVATE cmsis-dsp-mock/include)
target_include_directories(asl-parrot PRIVATE ed25519/src)
target_include_directories(asl-parrot PRIVATE amp-core/include)
target_include_directories(asl-parrot PRIVATE cpp-httplib)
target_include_directories(asl-parrot PRIVATE json/include)
target_include_directories(asl-parrot PRIVATE g726-codec/src)

# The untared libpiper
target_include_directories(asl-parrot PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/libpiper/include)
# TODO: REMOVE
target_include_directories(asl-parrot PRIVATE amp-core/src)
target_link_libraries(asl-parrot -lcurl)
target_link_directories(asl-parrot PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/libpiper)
target_link_directories(asl-parrot PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/libpiper/lib)
target_link_libraries(asl-parrot -lpiper -lonnxruntime)

# Installation 
install(TARGETS asl-parrot RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin COMPONENT asl-parrot)

# Files needed for Piper TTS
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libpiper/libpiper.so DESTINATION ${CMAKE_INSTALL_PREFIX}/lib COMPONENT asl-parrot)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libpiper/lib/libonnxruntime.so.1 DESTINATION ${CMAKE_INSTALL_PREFIX}/lib COMPONENT asl-parrot)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libpiper/lib/libonnxruntime.so.1.22.0 DESTINATION ${CMAKE_INSTALL_PREFIX}/lib COMPONENT asl-parrot)

# ----- test-voice -----------------------------------------------------------

add_executable(test-voice EXCLUDE_FROM_ALL
  src/demos/test-voice.cpp
)
add_dependencies(test-voice libpiper_untar)
add_dependencies(test-voice piper_voice)

target_include_directories(test-voice PRIVATE src)
target_include_directories(test-voice PRIVATE include)
# The untared libpiper
target_include_directories(test-voice PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/libpiper-${CMAKE_HOST_SYSTEM_PROCESSOR}/include)
target_link_directories(test-voice PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/libpiper-${CMAKE_HOST_SYSTEM_PROCESSOR})
target_link_directories(test-voice PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/libpiper-${CMAKE_HOST_SYSTEM_PROCESSOR}/lib)
target_link_libraries(test-voice -lpiper -lonnxruntime)
