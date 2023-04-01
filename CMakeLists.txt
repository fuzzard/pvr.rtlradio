cmake_minimum_required(VERSION 3.5)
project(pvr.rtlradio)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

find_package(Kodi REQUIRED)
find_package(FFTW REQUIRED)
find_package(FFTW3f REQUIRED)
find_package(MPG123 REQUIRED)
find_package(RapidJSON 1.1.0 REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(rtlsdr REQUIRED)
find_package(libusb-1.0 REQUIRED)
find_package(glm REQUIRED)
find_package(FDK-AAC REQUIRED)

if(NOT WIN32 AND (APP_RENDER_SYSTEM STREQUAL "gl" OR NOT APP_RENDER_SYSTEM))
  find_package(OpenGl REQUIRED)
  set(DEPLIBS ${OPENGL_LIBRARIES})
  set(includes ${OPENGL_INCLUDE_DIR})
  add_definitions(${OPENGL_DEFINITIONS})
else()
  find_package(OpenGLES REQUIRED)
  set(DEPLIBS ${OPENGLES_LIBRARIES})
  set(includes ${OPENGLES_INCLUDE_DIR})
  add_definitions(${OPENGLES_DEFINITIONS})
endif()

add_definitions(-DDABLIN_AAC_FDKAAC -DFFTW_NO_Complex -DFINGER)

include_directories(${KODI_INCLUDE_DIR}/.. # Hack way with "/..", need bigger Kodi cmake rework to match right include ways
                    ${FFTW_INCLUDE_DIRS}
                    ${MPG123_INCLUDE_DIR}
                    ${GLM_INCLUDE_DIR}
                    ${RAPIDJSON_INCLUDE_DIRS}
                    ${SQLITE3_INCLUDE_DIR}
                    ${rtlsdr_INCLUDE_DIRS}
                    ${LIBUSB_1_INCLUDE_DIRS}
                    ${FDK_AAC_INCLUDE_DIRS})

list(APPEND DEPLIBS ${FFTW_DOUBLE_LIB}
                    ${MPG123_LIBRARIES}
                    ${SQLITE3_LIBRARY}
                    ${RTLSDR_LIBRARIES}
                    ${LIBUSB_1_LIBRARIES}
                    ${FDK_AAC_LIBRARIES})
#             ${RTLSDR_LIBRARIES}
#             ${FFTW_LIBRARIES})

# message(FATAL_ERROR "-->${FFTW_DOUBLE_LIB}")

set(PVRRTLRADIO_SOURCES src/addon.cpp
                        src/dabdsp/fft.cpp
                        src/dabdsp/Xtan2.cpp
                        src/dabdsp/channels.cpp
                        src/dabdsp/charsets.cpp
                        src/dabdsp/dab-audio.cpp
                        src/dabdsp/dab-constants.cpp
                        src/dabdsp/dab_decoder.cpp
                        src/dabdsp/dabplus_decoder.cpp
                        src/dabdsp/decode_rs_char.c
                        src/dabdsp/decoder_adapter.cpp
                        src/dabdsp/eep-protection.cpp
                        src/dabdsp/fib-processor.cpp
                        src/dabdsp/fic-handler.cpp
                        src/dabdsp/freq-interleaver.cpp
                        src/dabdsp/init_rs_char.c
                        src/dabdsp/mot_manager.cpp
                        src/dabdsp/msc-handler.cpp
                        src/dabdsp/ofdm-decoder.cpp
                        src/dabdsp/ofdm-processor.cpp
                        src/dabdsp/pad_decoder.cpp
                        src/dabdsp/phasereference.cpp
                        src/dabdsp/phasetable.cpp
                        src/dabdsp/profiling.cpp
                        src/dabdsp/protTables.cpp
                        src/dabdsp/radio-receiver.cpp
                        src/dabdsp/tii-decoder.cpp
                        src/dabdsp/tools.cpp
                        src/dabdsp/uep-protection.cpp
                        src/dabdsp/viterbi.cpp
                        src/fmdsp/demodulator.cpp
                        src/fmdsp/downconvert.cpp
                        src/fmdsp/fastfir.cpp
                        src/fmdsp/fft.cpp
                        src/fmdsp/fir.cpp
                        src/fmdsp/fmdemod.cpp
                        src/fmdsp/fractresampler.cpp
                        src/fmdsp/iir.cpp
                        src/fmdsp/wfmdemod.cpp
                        src/hddsp/acquire.c
                        src/hddsp/conv_dec.c
                        src/hddsp/decode.c
                        src/hddsp/firdecim_q15.c
                        src/hddsp/frame.c
                        src/hddsp/input.c
                        src/hddsp/nrsc5.c
                        src/hddsp/output.c
                        src/hddsp/pids.c
                        src/hddsp/rs_decode.c
                        src/hddsp/rs_init.c
                        src/hddsp/strndup.c
                        src/hddsp/sync.c
                        src/hddsp/unicode.c
                        src/anglepch.cpp
                        src/channeladd.cpp
                        src/channelsettings.cpp
                        src/dabmuxscanner.cpp
                        src/dabstream.cpp
                        src/database.cpp
                        src/filedevice.cpp
                        src/fmstream.cpp
                        src/hdmuxscanner.cpp
                        src/hdstream.cpp
                        src/id3v1tag.cpp
                        src/id3v2tag.cpp
                        src/libusb_exception.cpp
                        src/rdsdecoder.cpp
                        src/signalmeter.cpp
                        src/stdafx.cpp
                        src/sqlite_exception.cpp
                        src/tcpdevice.cpp
                        src/uecp.cpp
                        src/usbdevice.cpp
                        src/wxstream.cpp)

set(PVRRTLRADIO_HEADERS src/addon.h
                        src/align.h
                        src/anglepch.h
                        src/dabmuxscanner.h
                        src/dabstream.h
                        src/channeladd.h
                        src/channelsettings.h
                        src/database.h
                        src/filedevice.h
                        src/fmstream.h
                        src/hdmuxscanner.h
                        src/hdstream.h
                        src/id3v1tag.h
                        src/id3v2tag.h
                        src/libusb_exception.h
                        src/dbtypes.h
                        src/muxscanner.h
                        src/props.h
                        src/pvrstream.h
                        src/pvrtypes.h
                        src/rdsdecoder.h
                        src/renderingcontrol.h
                        src/rtldevice.h
                        src/scalar_condition.h
                        src/signalmeter.h
                        src/stdafx.h
                        src/socket_exception.h
                        src/sqlite_exception.h
                        src/string_exception.h
                        src/tcpdevice.h
                        src/uecp.h
                        src/usbdevice.h
                        src/wxstream.h
                        src/compat/bionic/complex.cpp
                        src/dabdsp/MathHelper.h
                        src/dabdsp/Xtan2.h
                        src/dabdsp/channels.h
                        src/dabdsp/char.h
                        src/dabdsp/charsets.h
                        src/dabdsp/dab-audio.h
                        src/dabdsp/dab-constants.h
                        src/dabdsp/dab-processor.h
                        src/dabdsp/dab-virtual.h
                        src/dabdsp/dab_decoder.h
                        src/dabdsp/dabplus_decoder.h
                        src/dabdsp/decode_rs.h
                        src/dabdsp/decoder_adapter.h
                        src/dabdsp/eep-protection.h
                        src/dabdsp/encode_rs.h
                        src/dabdsp/energy_dispersal.h
                        src/dabdsp/fec.h
                        src/dabdsp/fft.h
                        src/dabdsp/fib-processor.h
                        src/dabdsp/fic-handler.h
                        src/dabdsp/freq-interleaver.h
                        src/dabdsp/init_rs.h
                        src/dabdsp/mot_manager.h
                        src/dabdsp/msc-handler.h
                        src/dabdsp/ofdm-decoder.h
                        src/dabdsp/ofdm-processor.h
                        src/dabdsp/pad_decoder.h
                        src/dabdsp/phasereference.h
                        src/dabdsp/phasetable.h
                        src/dabdsp/profiling.h
                        src/dabdsp/protTables.h
                        src/dabdsp/protection.h
                        src/dabdsp/radio-controller.h
                        src/dabdsp/radio-receiver-options.h
                        src/dabdsp/radio-receiver.h
                        src/dabdsp/ringbuffer.h
                        src/dabdsp/rs-common.h
                        src/dabdsp/subchannel_sink.h
                        src/dabdsp/tii-decoder.h
                        src/dabdsp/tools.h
                        src/dabdsp/uep-protection.h
                        src/dabdsp/viterbi.h
                        src/fmdsp/datatypes.h
                        src/fmdsp/demodulator.h
                        src/fmdsp/downconvert.h
                        src/fmdsp/fastfir.h
                        src/fmdsp/fft.h
                        src/fmdsp/filtercoef.h
                        src/fmdsp/fir.h
                        src/fmdsp/fmdemod.h
                        src/fmdsp/fractresampler.h
                        src/fmdsp/iir.h
                        src/fmdsp/rbdsconstants.h
                        src/fmdsp/wfmdemod.h
                        src/hddsp/acquire.h
                        src/hddsp/bitwriter.h
                        src/hddsp/config.h
                        src/hddsp/conv.h
                        src/hddsp/conv_gen.h
                        src/hddsp/conv_neon.h
                        src/hddsp/conv_sse.h
                        src/hddsp/decode.h
                        src/hddsp/defines.h
                        src/hddsp/firdecim_q15.h
                        src/hddsp/frame.h
                        src/hddsp/input.h
                        src/hddsp/nrsc5.h
                        src/hddsp/output.h
                        src/hddsp/pids.h
                        src/hddsp/private.h
                        src/hddsp/rs_char.h
                        src/hddsp/sync.h
                        src/hddsp/unicode.h)

if(WIN32)
  list(APPEND PVRRTLRADIO_SOURCES src/win32_exception.cpp)
  list(APPEND PVRRTLRADIO_HEADERS src/win32_exception.h)
  list(APPEND DEPLIBS ws2_32 Shlwapi)
endif()

build_addon(${CMAKE_PROJECT_NAME} PVRRTLRADIO DEPLIBS)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE "VERSION_PRODUCTNAME_ANSI=\"${CMAKE_PROJECT_NAME}\"")
# target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE VERSION_PRODUCTNAME_ANSI="Michael G. Brehm")
# target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE VERSION_PRODUCTNAME_ANSI="Michael G. Brehm")
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE "VERSION_VERSION3_ANSI=\"${PVRRTLRADIO_VERSION}\"")
# target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE PVRRTLRADIO_VERSION)
# Copyright=^M
# Product=zuki.pvr.rtlradio^M
# Version=20.2.0


include(CPack)
