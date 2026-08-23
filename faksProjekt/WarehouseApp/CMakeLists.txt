cmake_minimum_required(VERSION 3.16)

project(WarehouseApp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Widgets Xml Sql PrintSupport Network)

# Manually define OpenSSL targets for MinGW (bypasses FindOpenSSL discovery issues)
set(OPENSSL_INCLUDE_DIR "C:/Program Files/OpenSSL-Win64/include")
set(OPENSSL_FOUND TRUE)
add_library(OpenSSL::Crypto SHARED IMPORTED GLOBAL)
set_target_properties(OpenSSL::Crypto PROPERTIES
    IMPORTED_LOCATION  "C:/Program Files/OpenSSL-Win64/libcrypto-4-x64.dll"
    IMPORTED_IMPLIB    "C:/Users/tindo/openssl-mingw/libcrypto.dll.a"
    INTERFACE_INCLUDE_DIRECTORIES "C:/Program Files/OpenSSL-Win64/include"
)
add_library(OpenSSL::SSL SHARED IMPORTED GLOBAL)
set_target_properties(OpenSSL::SSL PROPERTIES
    IMPORTED_LOCATION  "C:/Program Files/OpenSSL-Win64/libssl-4-x64.dll"
    IMPORTED_IMPLIB    "C:/Users/tindo/openssl-mingw/libssl.dll.a"
    INTERFACE_INCLUDE_DIRECTORIES "C:/Program Files/OpenSSL-Win64/include"
)

# Statička biblioteka
add_library(WarehouseUtils STATIC
    warehouseutils.h
    warehouseutils.cpp
)
target_link_libraries(WarehouseUtils PRIVATE Qt6::Core)

# ── Icon generator (build-time executable) ───────────────────────────────────
# A small Qt tool that draws 3 coloured-square PNGs with QPainter/QImage and
# saves them into icons/ in the source tree.  It runs exactly once (CMake
# re-runs it only when its OUTPUT files are missing or stale).
add_executable(IconGenerator generate_icons.cpp)
set_target_properties(IconGenerator PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_link_libraries(IconGenerator PRIVATE Qt6::Core Qt6::Gui)

set(WAREHOUSE_ICON_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/icons/product.png
    ${CMAKE_CURRENT_SOURCE_DIR}/icons/supplier.png
    ${CMAKE_CURRENT_SOURCE_DIR}/icons/warning.png
)

add_custom_command(
    OUTPUT  ${WAREHOUSE_ICON_FILES}
    COMMAND ${CMAKE_COMMAND} -E env "PATH=C:/Qt/6.11.0/mingw_64/bin$<SEMICOLON>$ENV{PATH}" $<TARGET_FILE:IconGenerator> ${CMAKE_CURRENT_SOURCE_DIR}/icons
    DEPENDS IconGenerator
    COMMENT "Generating warehouse PNG icons via IconGenerator"
    VERBATIM
)

add_custom_target(WarehouseIconsTarget ALL
    DEPENDS ${WAREHOUSE_ICON_FILES}
)

# ── WarehouseResources shared library (DLL) ───────────────────────────────────
# Embeds the 3 PNGs in a Qt resource file; exports loadWarehouseIcon().
# OBJECT_DEPENDS tells AUTORCC that the .qrc compilation depends on the PNGs.
set_source_files_properties(warehouse_icons.qrc PROPERTIES
    OBJECT_DEPENDS "${WAREHOUSE_ICON_FILES}"
)
add_library(WarehouseResources SHARED
    warehouseresources.h
    warehouseresources.cpp
    warehouse_icons.qrc
)
add_dependencies(WarehouseResources WarehouseIconsTarget)
target_compile_definitions(WarehouseResources PRIVATE WAREHOUSERESOURCES_EXPORTS)
target_link_libraries(WarehouseResources PRIVATE Qt6::Core Qt6::Gui)

# ── WarehouseDlgs shared library (DLL) ───────────────────────────────────────
# Exports ShowAboutDialog / ShowQuickCalcDialog via extern "C" + dllexport.
# Requires Qt6::Widgets because it creates QDialogs.
add_library(WarehouseDlgs SHARED
    warehousedlgs.h
    warehousedlgs.cpp
)
target_compile_definitions(WarehouseDlgs PRIVATE WAREHOUSEDLGS_EXPORTS)
target_link_libraries(WarehouseDlgs PRIVATE Qt6::Core Qt6::Widgets)

# ── WarehouseExport shared library (DLL) ─────────────────────────────────────
# WAREHOUSEEXPORT_EXPORTS is defined only for the DLL translation units so the
# header emits __declspec(dllexport); consumers get __declspec(dllimport).
add_library(WarehouseExport SHARED
    warehouseexport.h
    warehouseexport.cpp
)
target_compile_definitions(WarehouseExport PRIVATE WAREHOUSEEXPORT_EXPORTS)
target_link_libraries(WarehouseExport PRIVATE Qt6::Core)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(WarehouseApp
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ActivityLog.cpp ActivityLog.h Category.cpp Category.h CMakeLists.txt main.cpp mainwindow.cpp mainwindow.h mainwindow.ui Product.cpp Product.h   projekt.vcxproj.filters projekt.vcxproj.user Supplier.cpp Supplier.h User.cpp User.h
        logindialog.h logindialog.cpp
        productdialog.h productdialog.cpp
        settingsdialog.h settingsdialog.cpp
        translationmanager.h translationmanager.cpp
        settingsmanager.h settingsmanager.cpp
        supplierdialog.h supplierdialog.cpp
        jsonmanager.h jsonmanager.cpp
        binarymanager.h binarymanager.cpp
        databasemanager.h databasemanager.cpp
        reportmanager.h reportmanager.cpp
        inventoryanalyzer.h inventoryanalyzer.cpp
        udpprotocol.h
        cryptomanager.h cryptomanager.cpp
        Order.h Order.cpp
        orderxmlmanager.h orderxmlmanager.cpp
        orderdialog.h orderdialog.cpp
        userdialog.h userdialog.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET WarehouseApp APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(WarehouseApp SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(WarehouseApp
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(WarehouseApp PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(WarehouseApp PRIVATE Qt6::Widgets Qt6::Xml Qt6::Sql Qt6::PrintSupport Qt6::Network OpenSSL::Crypto WarehouseUtils WarehouseExport WarehouseDlgs WarehouseResources)


# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.WarehouseApp)
endif()
set_target_properties(WarehouseApp PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS WarehouseApp
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(WarehouseApp)
endif()

# ── WarehouseServer ───────────────────────────────────────────────────────────
# Qt console app: QTcpServer on port 23117.
# Shares the model + DB source files with WarehouseApp (compiled separately).
qt_add_executable(WarehouseServer
    server_main.cpp
    warehouseserver.h  warehouseserver.cpp
    clienthandler.h    clienthandler.cpp
    udpprotocol.h
    databasemanager.h  databasemanager.cpp
    cryptomanager.h    cryptomanager.cpp
    Product.h          Product.cpp
    Category.h         Category.cpp
    Supplier.h         Supplier.cpp
    User.h             User.cpp
)
set_target_properties(WarehouseServer PROPERTIES
    CXX_STANDARD          17
    CXX_STANDARD_REQUIRED ON
    # No WIN32_EXECUTABLE — console app so qInfo() output is visible
)
target_link_libraries(WarehouseServer PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Sql
    OpenSSL::Crypto
)

# ── WarehouseValidator ────────────────────────────────────────────────────────
# Standalone console tool — no Qt dependency, just standard C++.
# Reads the binary snapshot header and returns 0 (valid) or 1 (invalid).
add_executable(WarehouseValidator
    validator_main.cpp
)
set_target_properties(WarehouseValidator PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
