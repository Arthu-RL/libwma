install(
    TARGETS wma
    EXPORT wmaTargets

    FILE_SET public_headers
    DESTINATION include

    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(
    EXPORT wmaTargets
    FILE wma-targets.cmake
    NAMESPACE wma::
    DESTINATION lib/cmake/wma
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/wmaConfig.cmake.in
    ${PROJECT_BINARY_DIR}/wmaConfig.cmake
    INSTALL_DESTINATION lib/cmake/wma
)

write_basic_package_version_file(
    ${PROJECT_BINARY_DIR}/wmaConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
    ${PROJECT_BINARY_DIR}/wmaConfig.cmake
    ${PROJECT_BINARY_DIR}/wmaConfigVersion.cmake
    DESTINATION
    lib/cmake/wma
)