
#### danmaku2ass ####
add_subdirectory(danmaku2ass_native)

#### hlsdl ####
if (UNIX)
    if(APPLE AND NOT OPENSSL_ROOT_DIR)
        EXECUTE_PROCESS(COMMAND brew --prefix openssl OUTPUT_VARIABLE OPENSSL_ROOT_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()
    find_package(OpenSSL REQUIRED)
    find_package(CURL REQUIRED)
    find_package(Threads REQUIRED)

    add_executable(moonplayer-hlsdl
        hlsdl/src/main.c
        hlsdl/src/aes_openssl.c
        hlsdl/src/curl.c
        hlsdl/src/hls.c
        hlsdl/src/misc.c
        hlsdl/src/msg.c
        hlsdl/src/mpegts.c)

    target_include_directories(moonplayer-hlsdl PRIVATE ${OPENSSL_INCLUDE_DIR} ${CURL_INCLUDE_DIRS})
    target_link_libraries(moonplayer-hlsdl PRIVATE ${OPENSSL_CRYPTO_LIBRARY} ${CURL_LIBRARIES} Threads::Threads)

    if (NOT APPLE)
        install(TARGETS moonplayer-hlsdl RUNTIME DESTINATION bin)
    endif()
endif()