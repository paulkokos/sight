cmake_minimum_required (VERSION 3.13)

# Use new policy for 'install_name' and RPATH on macOS (use `cmake --help-policy CMP0068` for details)
cmake_policy(SET CMP0068 NEW)
# Use new policy for `FindOpenGL` to prefer GLVND by default when available on linux (use `cmake --help-policy CMP0072` for details).
cmake_policy(SET CMP0072 NEW)
# Use new policy for 'Honor visibility properties for all target types.' (use `cmake --help-policy CMP0063` for details)
cmake_policy(SET CMP0063 NEW)
# Adds support for the new IN_LIST operator.
cmake_policy(SET CMP0057 NEW)
# Use new policy to use CMAKE_CXX_STANDARD in try_compile() macro
cmake_policy(SET CMP0067 NEW)

# On Windows, if the user doesn't specify a value,
# 'CMAKE_BUILD_TYPE' is automatically initialized to 'Debug' after 'project()'.
# So we need to check this variable at this point.
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of build, options are: Debug, Release")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug;Release")

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    message(FATAL_ERROR "Invalid value for CMAKE_BUILD_TYPE: '${CMAKE_BUILD_TYPE}' (required Debug, Release)")
endif()

project (sight)

if(APPLE)
    message(WARNING "macOS is no more officially supported and complete support will be removed in sight 21.0")
endif()

enable_testing()

include(CheckVariableExists)
include(CMakeParseArguments)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/build/macros.cmake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/build/linux/modules)

########################################################################################################################
# User options
########################################################################################################################

option(BUILD_SDK "Build Sight as a SDK" OFF)

# Tests build / run options
set(BUILD_TESTS ON CACHE BOOL "Configures projects associated tests (<project>Test projects)")
set(TESTS_XML_OUTPUT OFF CACHE BOOL "Tests will generate an xml output, suitable for CI integration")
mark_as_advanced(TESTS_XML_OUTPUT)
set(TESTS_FILTER "" CACHE STRING "Allows to only build/run tests whose path contains the filter string.")
mark_as_advanced(TESTS_FILTER)

set(SET_API_VERSION ON)

option(ENABLE_PCH "Use pre-compiled headers to speedup the compilation" ON)
option(VERBOSE_PCH "Display debug messages to help debugging PCH" OFF)
mark_as_advanced(ENABLE_PCH)
mark_as_advanced(VERBOSE_PCH)

# QML_IMPORT_PATH allows qtCreator to find the qml modules created in our modules
set(QML_IMPORT_PATH "" CACHE STRING "Path of the Qml modules." FORCE)
mark_as_advanced(QML_IMPORT_PATH)
set( ENABLE_QML_APPLICATION OFF CACHE BOOL "Temporary Variable to set Qml or not in applications to kept old services working" )

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/build/PrecompiledHeader.cmake)

if(MSVC)
    if(NOT DEFINED CMAKE_PCH_COMPILER_TARGETS)
        # this will be executed in just before makefile generation
        variable_watch(CMAKE_BACKWARDS_COMPATIBILITY pch_msvc_hook)
    endif()
endif()

if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE} CACHE STRING "List of supported configurations." FORCE)
endif()

set(CREATE_SUBPROJECTS OFF CACHE BOOL "Create a project for each subproject")

set(PROJECTS_TO_BUILD CACHE STRING
        "List of projects that will be configured for build.
        Leave empty to configure all projects"
    )

# Use solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY AUTOGEN_TARGETS_FOLDER automoc)

set(SPYLOG_LEVEL "error" CACHE STRING "Log level")
set(SPYLOG_LEVEL_VALUES "fatal;error;warning;info;debug;trace" CACHE INTERNAL
    "List of possible values for log level")
set(SPYLOG_LEVEL_MAP_fatal 1)
set(SPYLOG_LEVEL_MAP_error 2)
set(SPYLOG_LEVEL_MAP_warning 3)
set(SPYLOG_LEVEL_MAP_info 4)
set(SPYLOG_LEVEL_MAP_debug 5)
set(SPYLOG_LEVEL_MAP_trace 6)
set_property(CACHE SPYLOG_LEVEL PROPERTY STRINGS ${SPYLOG_LEVEL_VALUES} )

########################################################################################################################
# Warn user of install dir isn't empty
########################################################################################################################

file(GLOB_RECURSE INSTALL_DIR_CONTENT ${CMAKE_INSTALL_PREFIX}/*)
list(LENGTH INSTALL_DIR_CONTENT CONTENT)
if(NOT CONTENT EQUAL 0)
    # DIR isn't empty, warn user.
    message(WARNING "CMAKE_INSTALL_PREFIX (${CMAKE_INSTALL_PREFIX}) isn't empty, please select another folder or clean it before running install command.")
endif()

########################################################################################################################
# External libraries management
########################################################################################################################

if(UNIX AND NOT APPLE)
    option(USE_SYSTEM_LIB "Use system libraries to compile sight" OFF)
endif()

set(FWCMAKE_RESOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)

if(USE_SYSTEM_LIB)
    # Append our 'FindPackages.cmake' to CMAKE_MODULE_PATH and include BinPkgs config file
    list(APPEND CMAKE_PREFIX_PATH ${FWCMAKE_RESOURCE_PATH}/modules)
    list(APPEND CMAKE_MODULE_PATH ${FWCMAKE_RESOURCE_PATH}/modules)
    list(APPEND CMAKE_FIND_ROOT_PATH ${FWCMAKE_RESOURCE_PATH}/modules)

    # Parsing of 'fw-*.cmake' scripts. This should go away at some point if we use CMake properly
    file(GLOB LIB_CONFIGS ${FWCMAKE_RESOURCE_PATH}/modules/fw-*.cmake )
    foreach(LIB_CONFIG ${LIB_CONFIGS})
        message(STATUS "include: ${LIB_CONFIG}")
        include("${LIB_CONFIG}")
    endforeach()
else()
    option(CONAN_BUILD_MISSING "Build missing conan package automatically (experimental)" OFF)
    mark_as_advanced(CONAN_BUILD_MISSING)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/conan/conan-deps.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/conan/conan.cmake)
endif()

########################################################################################################################
# Automatic detection of additional repositories using .sight files
########################################################################################################################
file(GLOB ADDITIONAL_REPOSITORIES_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../*/.sight ${CMAKE_CURRENT_SOURCE_DIR}/../*/.fw4spl)

foreach(REPOSITORY ${ADDITIONAL_REPOSITORIES_FILES})
    file(READ ${REPOSITORY} REPO_RAW_NAME)
    string(STRIP ${REPO_RAW_NAME} REPO_RAW_NAME)
    string(TOUPPER ${REPO_RAW_NAME} REPO_NAME)

    if(NOT ${REPO_NAME} STREQUAL "SIGHT")

        get_filename_component(REPO_DIR ${REPOSITORY} DIRECTORY)
        get_filename_component(REPO_DIR ${REPO_DIR} ABSOLUTE)

        option(BUILD_${REPO_NAME} "Enable ${REPO_NAME} repository" ON)
        list(APPEND ADDITIONAL_REPOSITORIES_OPTION BUILD_${REPO_NAME})
        list(APPEND ADDITIONAL_REPOSITORIES_NAME ${REPO_RAW_NAME})
        list(APPEND ADDITIONAL_REPOSITORIES ${REPO_DIR})
    endif()
endforeach()

# parse additional CMakeLists
if(ADDITIONAL_REPOSITORIES)
    list(LENGTH ADDITIONAL_REPOSITORIES ADDITIONAL_REPOSITORIES_LEN)

    math(EXPR ADDITIONAL_REPOSITORIES_LEN ${ADDITIONAL_REPOSITORIES_LEN}-1)
    foreach(REPO_INDEX RANGE ${ADDITIONAL_REPOSITORIES_LEN})
        list(GET ADDITIONAL_REPOSITORIES ${REPO_INDEX} ADDITIONAL_REPO)
        list(GET ADDITIONAL_REPOSITORIES_OPTION ${REPO_INDEX} ADDITIONAL_REPO_OPTION)

        if(EXISTS ${ADDITIONAL_REPO}/CMakeLists.txt AND ${ADDITIONAL_REPO_OPTION})
            get_filename_component(DIR_NAME ${ADDITIONAL_REPO} NAME)
            add_subdirectory(${ADDITIONAL_REPO} ${DIR_NAME})
        endif()
    endforeach()
endif()

########################################################################################################################
# Default paths settings for libraries, modules and resources
########################################################################################################################

if(WIN32)
    set(FW_INSTALL_PATH_SUFFIX )
    set(SIGHT_MODULE_LIB_PREFIX "${CMAKE_INSTALL_BINDIR}")
    set(SIGHT_MODULE_RC_PREFIX "${CMAKE_INSTALL_DATADIR}")
else()
    set(FW_INSTALL_PATH_SUFFIX "sight")
    set(SIGHT_MODULE_LIB_PREFIX "${CMAKE_INSTALL_LIBDIR}/${FW_INSTALL_PATH_SUFFIX}")
    set(SIGHT_MODULE_RC_PREFIX "${CMAKE_INSTALL_DATADIR}/${FW_INSTALL_PATH_SUFFIX}")
endif()

# For backward compatibility with Sight < 20.0
set(FWBUNDLE_LIB_PREFIX "${SIGHT_MODULE_LIB_PREFIX}")
set(FWBUNDLE_RC_PREFIX "${SIGHT_MODULE_RC_PREFIX}")

set(LIBRARY_OUTPUT_DIR lib/${FW_INSTALL_PATH_SUFFIX})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${LIBRARY_OUTPUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${LIBRARY_OUTPUT_DIR})
# Fixed path for multi-config builds (e.g. msvc)
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/${LIBRARY_OUTPUT_DIR} )
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/${LIBRARY_OUTPUT_DIR} )
endforeach()

########################################################################################################################
# The Properties.cmake files are the entry point to scan for targets
# We really start the configure process here
########################################################################################################################
file(GLOB_RECURSE PROJECTS_PROPERTIES */Properties.cmake)

# Do the same in additional repositories
if(ADDITIONAL_REPOSITORIES)

    list(LENGTH ADDITIONAL_REPOSITORIES ADDITIONAL_REPOSITORIES_LEN)
    math(EXPR ADDITIONAL_REPOSITORIES_LEN ${ADDITIONAL_REPOSITORIES_LEN}-1)

    foreach(REPO_INDEX RANGE ${ADDITIONAL_REPOSITORIES_LEN})
        list(GET ADDITIONAL_REPOSITORIES ${REPO_INDEX} ADDITIONAL_REPO)
        list(GET ADDITIONAL_REPOSITORIES_OPTION ${REPO_INDEX} ADDITIONAL_REPO_OPTION)

        if(${ADDITIONAL_REPO_OPTION})
            list(GET ADDITIONAL_REPOSITORIES_NAME ${REPO_INDEX} ADDITIONAL_REPO_NAME)
            message(STATUS "Configuring additional repository " ${ADDITIONAL_REPO_NAME} " found in: " ${ADDITIONAL_REPO})
            file(GLOB_RECURSE ADDITIONAL_REPOSITORIES_PROPERTIES ${ADDITIONAL_REPO}/*/Properties.cmake)
            list(APPEND PROJECTS_PROPERTIES ${ADDITIONAL_REPOSITORIES_PROPERTIES})
            set(ADDITIONAL_REPOSITORIES_SOURCE_DIR "${ADDITIONAL_REPOSITORIES_SOURCE_DIR} ${ADDITIONAL_REPO}")
        endif()
    endforeach()
endif()

set(FWCONFIG_PACKAGE_LOCATION lib/cmake/sight)

unset(PROJECT_LIST)

# Parse all Properties.cmake files and populate a list of CMake variables for each projet
# to know its dependencies, requirements, version, pch usage, etc...
foreach(PROPERTIES_FILE ${PROJECTS_PROPERTIES})
    get_filename_component(PROJECT_DIR ${PROPERTIES_FILE} PATH)

    loadProperties("${PROPERTIES_FILE}")
    if(NOT NAME)
        message(FATAL_ERROR "${PROPERTIES_FILE}: Project NAME can not be empty.")
    endif()

    list(FIND DEPENDENCIES ${NAME} DEP_LOOP)
    if(DEP_LOOP GREATER -1)
        message(FATAL_ERROR "${PROPERTIES_FILE}: A project can not be its own dependency.")
    endif()

    list(FIND REQUIREMENTS ${NAME} REQ_LOOP)
    if(REQ_LOOP GREATER -1)
        message(FATAL_ERROR "${PROPERTIES_FILE}: A project can not be its own requirement.")
    endif()

    if(${NAME}_DIR)
        message(FATAL_ERROR "${PROPERTIES_FILE}: A project NAME must be unique in the workspace. ${NAME} already defined there : ${${NAME}_DIR}")
    endif()

    string( TOUPPER "${TYPE}" TYPE )

    set(${NAME}_DEPENDENCIES      "${DEPENDENCIES}")
    set(${NAME}_REQUIREMENTS      "${REQUIREMENTS}")
    set(${NAME}_DISABLE_PCH       "${DISABLE_PCH}")
    set(${NAME}_CONAN_DEPS        "${CONAN_DEPS}")
    set(${NAME}_WARNINGS_AS_ERRORS "${WARNINGS_AS_ERRORS}")
    set(${NAME}_GENERATE_OBJECT_LIB "${GENERATE_OBJECT_LIB}")

    # Get the pch target, test the existence of type variable to exclude unbuilt projects
    if(ENABLE_PCH AND TYPE AND NOT ${NAME}_DISABLE_PCH)
        set(${NAME}_PROJECT_DIR "${PROJECT_DIR}")

        getPchTarget(${NAME} ${PROJECT_DIR} ${TYPE} ${USE_PCH_FROM_TARGET})

        # Append the pch target as a dependency (duplicate will be stripped later if it was already a dependency)
        if(NOT ${${NAME}_PCH_TARGET} STREQUAL ${NAME})
            list(APPEND ${NAME}_REQUIREMENTS ${${NAME}_PCH_TARGET})
        endif()
    endif()

    set(${NAME}_VERSION "${VERSION}")
    set(${NAME}_DIR "${PROJECT_DIR}")

    if(TYPE STREQUAL "APP")
        set(${NAME}_UNIQUE "${UNIQUE}")
    endif()

    if( TYPE STREQUAL "BUNDLE" OR TYPE STREQUAL "MODULE" OR TYPE STREQUAL "APP" AND START)
        set(${NAME}_START "${START}")
    endif()

    set(${NAME}_START_BEFORE "${START_BEFORE}")

    list(APPEND PROJECT_LIST ${NAME})

endforeach()

# Now find all dependencies if we restrict the build to some projects
if(PROJECTS_TO_BUILD)
    set(PROJECT_LIST ${PROJECTS_TO_BUILD})
    findAllDependencies("${PROJECT_LIST}" PROJECT_LIST)

    set(REPOSITORIES_DIRECTORIES ${ADDITIONAL_REPOSITORIES})
    list(APPEND REPOSITORIES_DIRECTORIES ${CMAKE_SOURCE_DIR})

    foreach(PROJECT ${PROJECT_LIST})
        set(${PROJECT}_INSTALL 1)

        # Configure cpack to package the project sources.
        findProjectSubdirectory("${${PROJECT}_DIR}" "${REPOSITORIES_DIRECTORIES}" SOURCE_INSTALL_DIR)
        # Add the project to CPack's list of source directories.
        list(APPEND CPACK_SOURCE_INSTALLED_DIRECTORIES ${${PROJECT}_DIR} ${SOURCE_INSTALL_DIR})
    endforeach()

    # Copy important sight files in a temporary directory to be packaged when generating a source archive.
    set(PACKAGE_SRC_DIR "${CMAKE_BINARY_DIR}/package_source")
    file(MAKE_DIRECTORY ${PACKAGE_SRC_DIR})
    file(COPY CMakeLists.txt COPYING COPYING.LESSER README.md DESTINATION ${PACKAGE_SRC_DIR})
    list(APPEND CPACK_SOURCE_INSTALLED_DIRECTORIES ${PACKAGE_SRC_DIR} "." "${CMAKE_SOURCE_DIR}/cmake" "./cmake")

    message(STATUS "Project configuration is restricted to : ${PROJECTS_TO_BUILD}")
elseif(BUILD_SDK)
    foreach(PROJECT ${PROJECT_LIST})
        set(${PROJECT}_INSTALL 1)
    endforeach()
endif()

# Clear all previous tests from project list
findTests("${PROJECT_LIST}" "" PROJECT_TESTS)
if(PROJECT_TESTS)
    list(REMOVE_ITEM PROJECT_LIST ${PROJECT_TESTS})
endif()

findTests("${PROJECT_LIST}" "${TESTS_FILTER}" PROJECT_TESTS)
findAllDependencies("${PROJECT_TESTS}" PROJECT_TESTS_DEPS)
list(REMOVE_DUPLICATES PROJECT_TESTS_DEPS)

# Add all test sources to the source package. Needed to make the package buildable without setting BUILD_TESTS=OFF.
foreach(PROJECT ${PROJECT_TESTS_DEPS})
    findProjectSubdirectory("${${PROJECT}_DIR}" "${REPOSITORIES_DIRECTORIES}" SOURCE_INSTALL_DIR)
    list(APPEND CPACK_SOURCE_INSTALLED_DIRECTORIES ${${PROJECT}_DIR} ${SOURCE_INSTALL_DIR})
endforeach()

# Append tests to the project list
if(BUILD_TESTS)
    list(APPEND PROJECT_LIST ${PROJECT_TESTS_DEPS})
    list(REMOVE_DUPLICATES PROJECT_LIST)
endif()

########################################################################################################################
# Conan packages and definition
########################################################################################################################

if(NOT USE_SYSTEM_LIB)
    findConanDeps("${PROJECT_LIST}" CONAN_DEPS_LIST)
    installConanDeps("${CONAN_DEPS_LIST}")
    add_definitions(${CONAN_DEFINES})
endif()

########################################################################################################################
# Compiler flags (We assume that we use the same flags fo C and C++ and we do only C++ here)
########################################################################################################################

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/build/flags.cmake)

########################################################################################################################
# Second projects scan where we actually perform the add_subdirectory() for each project and its dependencies
########################################################################################################################

# Define the path 'FW_EXTERNAL_LIBRARIES_DIR' used to find external libraries required by our applications
setExternalLibrariesDir()

foreach(PROJECT ${PROJECT_LIST})
    addProject(${PROJECT})
endforeach()

########################################################################################################################
# Export and install targets
########################################################################################################################

if(BUILD_SDK)
    # Create the SightConfig.cmake file
    set(FWCONFIG_FILE ${CMAKE_SOURCE_DIR}/cmake/build/SightConfig.cmake.in)

    configure_file(${FWCONFIG_FILE} "${CMAKE_BINARY_DIR}/cmake/SightConfig.cmake" @ONLY)

    # Create the SightConfigVersion file
    set(SIGHT_VERSION ${GIT_TAG})

    write_basic_package_version_file(
      "${CMAKE_BINARY_DIR}/cmake/SightConfigVersion.cmake"
      VERSION ${SIGHT_VERSION}
      COMPATIBILITY AnyNewerVersion
    )

    if(NOT USE_SYSTEM_LIB)
        file(APPEND "${CMAKE_BINARY_DIR}/cmake/SightRequirements.cmake"
             "set(CONAN_DEFINES ${CONAN_DEFINES})\n")
    endif()

    # Install the SightConfig.cmake and SightConfigVersion.cmake
    install(
      FILES
        "${CMAKE_BINARY_DIR}/cmake/SightConfig.cmake"
        "${CMAKE_BINARY_DIR}/cmake/SightConfigVersion.cmake"
        "${CMAKE_BINARY_DIR}/cmake/SightRequirements.cmake"
        "${CMAKE_SOURCE_DIR}/cmake/build/macros.cmake"
        "${CMAKE_SOURCE_DIR}/cmake/conan/conan.cmake"
        "${CMAKE_SOURCE_DIR}/cmake/conan/conan-deps.cmake"
        "${CMAKE_SOURCE_DIR}/cmake/build/linux/modules/FindFilesystem.cmake"
      DESTINATION
        ${FWCONFIG_PACKAGE_LOCATION}
      COMPONENT
        dev
    )

    # Install some files needed for the build
    install(
      FILES
        "${CMAKE_SOURCE_DIR}/cmake/build/configure_file.cmake"
        "${FWCMAKE_RESOURCE_PATH}/cppunit/cppunit_main.cpp"
        "${CMAKE_SOURCE_DIR}/cmake/build/config.hpp.in"
        "${CMAKE_SOURCE_DIR}/cmake/build/plugin_config.cmake"
        "${CMAKE_SOURCE_DIR}/cmake/build/plugin_config_command.cmake"
        "${CMAKE_SOURCE_DIR}/cmake/build/plugin.xml.in"
        "${CMAKE_SOURCE_DIR}/cmake/build/profile_config.cmake"
        "${CMAKE_SOURCE_DIR}/cmake/build/profile.xml.in"
        "${CMAKE_SOURCE_DIR}/cmake/build/registerServices.cpp.in"
        "${CMAKE_SOURCE_DIR}/cmake/build/spyLogLevel.hpp.in"
      DESTINATION
        ${FWCONFIG_PACKAGE_LOCATION}/build
      COMPONENT
        dev
    )

    # Install some files needed for the install
    install(
      FILES
        "${CMAKE_SOURCE_DIR}/cmake/install/generic_install.cmake"
        "${CMAKE_SOURCE_DIR}/cmake/install/helper.cmake"
        "${CMAKE_SOURCE_DIR}/cmake/install/get_git_rev.cmake"
      DESTINATION
        ${FWCONFIG_PACKAGE_LOCATION}/install
      COMPONENT
        dev
    )

    if(APPLE)
        install(
          FILES
            "${CMAKE_SOURCE_DIR}/cmake/install/osx_install.cmake"
          DESTINATION
            ${FWCONFIG_PACKAGE_LOCATION}/install
          COMPONENT
            dev
        )
        install(
          FILES
            "${CMAKE_SOURCE_DIR}/cmake/install/macos/rc/app.icns"
          DESTINATION
            ${FWCONFIG_PACKAGE_LOCATION}/install/macos/rc
          COMPONENT
            dev
        )
        install(
          FILES
          "${CMAKE_SOURCE_DIR}/cmake/install/macos/osx_install_name_tool.py"
          DESTINATION
            ${FWCONFIG_PACKAGE_LOCATION}/install/macos/
          COMPONENT
            dev
        )
        install(
          FILES
          "${CMAKE_SOURCE_DIR}/cmake/build/linux/template.sh.in"
          DESTINATION
            ${FWCONFIG_PACKAGE_LOCATION}/build/linux
          COMPONENT
            dev
        )
    elseif(WIN32)
        install(
          FILES
            "${CMAKE_SOURCE_DIR}/cmake/install/win_install.cmake"
          DESTINATION
            ${FWCONFIG_PACKAGE_LOCATION}/install
          COMPONENT
            dev
        )
        install(
          FILES
            "${CMAKE_SOURCE_DIR}/cmake/install/windows/template.bat.in"
            "${CMAKE_SOURCE_DIR}/cmake/install/windows/setpath.bat.in"
            "${CMAKE_SOURCE_DIR}/cmake/install/windows/windows_fixup.cmake.in"
          DESTINATION
            ${FWCONFIG_PACKAGE_LOCATION}/install/windows
          COMPONENT
            dev
        )
        install(
          FILES
            "${CMAKE_SOURCE_DIR}/cmake/install/windows/NSIS/NSIS.InstallOptions.ini.in"
            "${CMAKE_SOURCE_DIR}/cmake/install/windows/NSIS/NSIS.template.in"
          DESTINATION
            ${FWCONFIG_PACKAGE_LOCATION}/install/windows/NSIS/
          COMPONENT
            dev
        )
        install(
          FILES
            "${CMAKE_SOURCE_DIR}/cmake/install/windows/NSIS/rc/banner_nsis.bmp"
            "${CMAKE_SOURCE_DIR}/cmake/install/windows/NSIS/rc/dialog_nsis.bmp"
            "${CMAKE_SOURCE_DIR}/cmake/install/windows/NSIS/rc/app.ico"
            "${CMAKE_SOURCE_DIR}/cmake/install/windows/NSIS/rc/license.rtf"
          DESTINATION
            ${FWCONFIG_PACKAGE_LOCATION}/install/windows/NSIS/rc/
          COMPONENT
            dev
        )
    elseif(UNIX)
        install(
          FILES
          "${CMAKE_SOURCE_DIR}/cmake/build/linux/template.sh.in"
          DESTINATION
            ${FWCONFIG_PACKAGE_LOCATION}/build/linux
          COMPONENT
            dev
        )
        install(
          FILES
            "${CMAKE_SOURCE_DIR}/cmake/install/linux_install.cmake"
          DESTINATION
            ${FWCONFIG_PACKAGE_LOCATION}/install
          COMPONENT
            dev
        )

        install(
          FILES
            "${CMAKE_SOURCE_DIR}/cmake/install/linux/template.sh.in"
            "${CMAKE_SOURCE_DIR}/cmake/install/linux/linux_fixup.cmake.in"
          DESTINATION
            ${FWCONFIG_PACKAGE_LOCATION}/install/linux
          COMPONENT
            dev
        )
    endif()
endif()


########################################################################################################################
# Misc generators
########################################################################################################################

# Doxygen documentation
option(BUILD_DOCUMENTATION "Build the doxygen documentation" OFF)
if(BUILD_DOCUMENTATION)
    option(BUILD_DOCSET "Build a Dash/Zeal/XCode docset" OFF)
    include(${FWCMAKE_RESOURCE_PATH}doxygen/doxygen_generator.cmake)
    doxygenGenerator(${PROJECT_LIST})
    if(BUILD_DOCSET)
        docsetGenerator(${PROJECT_LIST})
    endif()
else()
    unset(BUILD_DOCSET CACHE)
endif()

# VS Code project
option(GENERATE_VSCODE_WS "Generate VS Code workspace" OFF)
if(GENERATE_VSCODE_WS)
    include(${FWCMAKE_RESOURCE_PATH}vscode/vscode_generator.cmake)
    vscodeGenerator(${PROJECT_LIST})
endif()

########################################################################################################################
# Packaging for the SDK
########################################################################################################################
if(BUILD_SDK)
    set(FWPROJECT_NAME sight)
    generic_install()
endif()
