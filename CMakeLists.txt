cmake_minimum_required(VERSION 3.16)
project(web_server C)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.c")

add_executable(${PROJECT_NAME} ${SOURCES})

include_directories(
  ${CMAKE_SOURCE_DIR}/include
)

find_package(OpenSSL REQUIRED)
target_link_libraries(${PROJECT_NAME} OpenSSL::SSL OpenSSL::Crypto)

set(CERT_FILE ${CMAKE_BINARY_DIR}/cert.pem)
set(KEY_FILE  ${CMAKE_BINARY_DIR}/key.pem)

add_custom_command(
    OUTPUT ${CERT_FILE} ${KEY_FILE}
    COMMAND openssl req -x509 -newkey rsa:2048 -nodes
            -keyout ${KEY_FILE}
            -out ${CERT_FILE}
            -days 365
            -subj "/CN=localhost"
    COMMENT "Generating self-signed TLS certificate"
)

add_custom_target(generate_cert ALL
    DEPENDS ${CERT_FILE} ${KEY_FILE}
)

add_dependencies(${PROJECT_NAME} generate_cert)
