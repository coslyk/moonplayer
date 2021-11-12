
#### danmaku2ass ####
add_subdirectory(danmaku2ass_cpp)

#### hlsdl ####
if (UNIX)
    if(APPLE)
        # Fix CMake's search for OpenSSL, see https://stackoverflow.com/a/62063357
        set(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
        set(OPENSSL_CRYPTO_LIBRARY ${OPENSSL_ROOT_DIR}/lib/libcrypto.dylib CACHE FILEPATH "" FORCE)
        set(OPENSSL_SSL_LIBRARY ${OPENSSL_ROOT_DIR}/lib/libssl.dylib CACHE FILEPATH "" FORCE)
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
