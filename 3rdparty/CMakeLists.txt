
#### hlsdl ####
if (UNIX)
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
    target_link_libraries(moonplayer-hlsdl PRIVATE OpenSSL::Crypto CURL::libcurl Threads::Threads)

    if (NOT APPLE)
        install(TARGETS moonplayer-hlsdl RUNTIME DESTINATION bin)
    endif()
endif()