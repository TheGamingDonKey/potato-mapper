cmake_minimum_required(VERSION 3.24)
project(PotatoSurfaceMaterialProof VERSION 0.8.8 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 6.10 REQUIRED COMPONENTS Widgets OpenGLWidgets Multimedia Network)
get_filename_component(REPO "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
find_program(PWSH_EXECUTABLE pwsh REQUIRED)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/generate-proof.ps1")
execute_process(COMMAND "${PWSH_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass -File "${CMAKE_CURRENT_SOURCE_DIR}/generate-proof.ps1" -Destination "${CMAKE_CURRENT_BINARY_DIR}"
    RESULT_VARIABLE GENERATED OUTPUT_VARIABLE GENERATION_LOG ERROR_VARIABLE GENERATION_ERROR)
if(NOT GENERATED EQUAL 0)
    message(FATAL_ERROR "Proof generation failed: ${GENERATION_LOG} ${GENERATION_ERROR}")
endif()
qt_add_executable(PotatoSurfaceMaterialProof WIN32
    "${CMAKE_CURRENT_BINARY_DIR}/legacy-main.cpp"
    "${REPO}/src/model.h" "${REPO}/src/model.cpp" "${REPO}/src/canvas.h" "${REPO}/src/canvas.cpp"
    "${REPO}/src/updates.h" "${REPO}/src/updates.cpp" "${REPO}/src/dynamic.cpp"
    "${REPO}/src/projects.cpp" "${REPO}/src/connections.cpp")
target_include_directories(PotatoSurfaceMaterialProof PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${REPO}/src")
target_compile_definitions(PotatoSurfaceMaterialProof PRIVATE POTATO_VERSION="0.8.8-material-proof" NOMINMAX)
target_link_libraries(PotatoSurfaceMaterialProof PRIVATE Qt6::Widgets Qt6::OpenGLWidgets Qt6::Multimedia Qt6::Network)
qt_add_resources(PotatoSurfaceMaterialProof proof_assets PREFIX "/" BASE "${REPO}"
    FILES "${REPO}/assets/potato-mapper.png" "${REPO}/assets/patterns.frag" "${REPO}/assets/entrance.frag" "${REPO}/packaging/extract-update.ps1")
if(MSVC)
    target_compile_options(PotatoSurfaceMaterialProof PRIVATE /W4 /permissive- /utf-8)
endif()
