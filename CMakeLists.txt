# cxx_std_20 added in 3.12
cmake_minimum_required(VERSION 3.12)
project(Animera
    VERSION 0.1.0
    DESCRIPTION "A cross-platform Qt tool for making pixel-art animations for games"
    HOMEPAGE_URL "https://github.com/Kerndog73/Animera"
    LANGUAGES CXX
)

add_executable(Animera
    Animera.rc
    "src/abstract export params.cpp"
    "src/abstract export params.hpp"
    "src/animation file.cpp"
    "src/animation file.hpp"
    src/animation.cpp
    src/animation.hpp
    src/animation.moc
    src/application.cpp
    src/application.hpp
    src/application.moc
    "src/atlas generator.cpp"
    "src/atlas generator.hpp"
    "src/brush tool.cpp"
    "src/brush tool.hpp"
    "src/cel array.cpp"
    "src/cel array.hpp"
    "src/cel painter.cpp"
    "src/cel painter.hpp"
    src/cel.cpp
    src/cel.hpp
    "src/chunk io.cpp"
    "src/chunk io.hpp"
    "src/cli export.cpp"
    "src/cli export.hpp"
    "src/cli info.cpp"
    "src/cli info.hpp"
    "src/cli new.cpp"
    "src/cli new.hpp"
    src/cli.cpp
    src/cli.hpp
    "src/color convert.cpp"
    "src/color convert.hpp"
    "src/color graph widget.cpp"
    "src/color graph widget.hpp"
    "src/color graph widget.moc"
    "src/color handle.cpp"
    "src/color handle.hpp"
    "src/color picker impl.cpp"
    "src/color picker impl.hpp"
    "src/color picker impl.moc"
    "src/color picker widget.cpp"
    "src/color picker widget.hpp"
    "src/color picker widget.moc"
    "src/color slider widget.cpp"
    "src/color slider widget.hpp"
    "src/color slider widget.moc"
    "src/combo box widget.cpp"
    "src/combo box widget.hpp"
    "src/combo box widget.moc"
    src/composite.cpp
    src/composite.hpp
    "src/config colors.hpp"
    "src/config geometry.hpp"
    "src/config keys.hpp"
    src/connect.hpp
    "src/cpp atlas generator.cpp"
    "src/cpp atlas generator.hpp"
    "src/current tool.cpp"
    "src/current tool.hpp"
    src/dialog.cpp
    src/dialog.hpp
    "src/docopt helpers.cpp"
    "src/docopt helpers.hpp"
    "src/drag paint tools.cpp"
    "src/drag paint tools.hpp"
    "src/editor widget.cpp"
    "src/editor widget.hpp"
    "src/editor widget.moc"
    "src/enum operators.hpp"
    "src/error dialog.cpp"
    "src/error dialog.hpp"
    src/error.hpp
    "src/export dialog.cpp"
    "src/export dialog.hpp"
    "src/export dialog.moc"
    "src/export name.cpp"
    "src/export name.hpp"
    "src/export params.cpp"
    "src/export params.hpp"
    "src/export png.cpp"
    "src/export png.hpp"
    "src/export texture atlas.cpp"
    "src/export texture atlas.hpp"
    "src/file input widget.cpp"
    "src/file input widget.hpp"
    "src/file input widget.moc"
    "src/file io error.hpp"
    "src/file io.cpp"
    "src/file io.hpp"
    "src/flood fill tool.cpp"
    "src/flood fill tool.hpp"
    src/geometry.hpp
    "src/global font.cpp"
    "src/global font.hpp"
    "src/gradient tool.cpp"
    "src/gradient tool.hpp"
    "src/graphics convert.hpp"
    "src/group array.cpp"
    "src/group array.hpp"
    "src/hex input widget.cpp"
    "src/hex input widget.hpp"
    "src/hex input widget.moc"
    "src/icon push button widget.cpp"
    "src/icon push button widget.hpp"
    "src/icon radio button widget.cpp"
    "src/icon radio button widget.hpp"
    src/image.cpp
    src/image.hpp
    "src/init canvas dialog.cpp"
    "src/init canvas dialog.hpp"
    "src/init canvas dialog.moc"
    "src/int range.hpp"
    "src/keys dialog.cpp"
    "src/keys dialog.hpp"
    "src/label widget.cpp"
    "src/label widget.hpp"
    src/main.cpp
    src/main.moc
    src/math.hpp
    #"src/native mac.hpp"
    #"src/native mac.mm"
    "src/number input widget.cpp"
    "src/number input widget.hpp"
    "src/number input widget.moc"
    "src/paint params.hpp"
    src/painting.cpp
    src/painting.hpp
    "src/palette span.hpp"
    "src/palette widget.cpp"
    "src/palette widget.hpp"
    "src/palette widget.moc"
    src/palette.cpp
    src/palette.hpp
    src/palette.moc
    "src/picker impl gray.cpp"
    "src/picker impl gray.hpp"
    "src/picker impl rgba.cpp"
    "src/picker impl rgba.hpp"
    "src/png atlas generator.cpp"
    "src/png atlas generator.hpp"
    src/png.cpp
    src/png.hpp
    "src/quit dialog.cpp"
    "src/quit dialog.hpp"
    "src/quit dialog.moc"
    "src/radio button widget.cpp"
    "src/radio button widget.hpp"
    src/resources.cpp
    "src/sample object.cpp"
    "src/sample object.hpp"
    "src/sample object.moc"
    "src/scope time.cpp"
    "src/scope time.hpp"
    "src/scroll bar widget.cpp"
    "src/scroll bar widget.hpp"
    "src/select tools.cpp"
    "src/select tools.hpp"
    "src/separator widget.cpp"
    "src/separator widget.hpp"
    src/settings.cpp
    src/settings.hpp
    "src/sprite packer.cpp"
    "src/sprite packer.hpp"
    "src/status bar widget.cpp"
    "src/status bar widget.hpp"
    "src/status bar widget.moc"
    "src/status msg.cpp"
    "src/status msg.hpp"
    "src/status object.cpp"
    "src/status object.hpp"
    "src/status object.moc"
    src/strings.hpp
    "src/surface factory.hpp"
    "src/text input widget.cpp"
    "src/text input widget.hpp"
    "src/text input widget.moc"
    "src/text push button widget.cpp"
    "src/text push button widget.hpp"
    "src/timeline cels widget.cpp"
    "src/timeline cels widget.hpp"
    "src/timeline cels widget.moc"
    "src/timeline controls widget.cpp"
    "src/timeline controls widget.hpp"
    "src/timeline controls widget.moc"
    "src/timeline frames widget.cpp"
    "src/timeline frames widget.hpp"
    "src/timeline frames widget.moc"
    "src/timeline layers widget.cpp"
    "src/timeline layers widget.hpp"
    "src/timeline layers widget.moc"
    "src/timeline painting.cpp"
    "src/timeline painting.hpp"
    "src/timeline widget.cpp"
    "src/timeline widget.hpp"
    "src/timeline widget.moc"
    src/timeline.cpp
    src/timeline.hpp
    src/timeline.moc
    "src/tool colors widget.cpp"
    "src/tool colors widget.hpp"
    "src/tool colors widget.moc"
    "src/tool param bar widget.cpp"
    "src/tool param bar widget.hpp"
    "src/tool param widget.cpp"
    "src/tool param widget.hpp"
    "src/tool select widget.cpp"
    "src/tool select widget.hpp"
    "src/tool select widget.moc"
    "src/tool widgets.cpp"
    "src/tool widgets.hpp"
    src/tool.cpp
    src/tool.hpp
    src/tool.moc
    "src/transform tools.cpp"
    "src/transform tools.hpp"
    "src/undo object.cpp"
    "src/undo object.hpp"
    "src/undo object.moc"
    src/undo.cpp
    src/undo.hpp
    "src/widget painting.cpp"
    "src/widget painting.hpp"
    "src/widget rect.hpp"
    src/window.cpp
    src/window.hpp
    src/window.moc
    src/zlib.hpp
    third_paty/stb/stb_rect_pack.c
    third_paty/stb/stb_rect_pack.h
)

target_compile_features(Animera
    PRIVATE
    cxx_std_20
)

target_link_options(Animera
    PRIVATE
    /SUBSYSTEM:WINDOWS # stop the command prompt window from showing
)

target_include_directories(Animera
    PRIVATE
    src
    third_party/Graphics/include
    third_party/span/include
)

set(USE_BOOST_REGEX ON CACHE BOOL "Use boost-regex")
set(WITH_STATIC ON)
add_subdirectory(third_party/docopt EXCLUDE_FROM_ALL)

find_package(libpng CONFIG REQUIRED)
find_package(Qt5Core CONFIG REQUIRED)
find_package(Qt5Gui CONFIG REQUIRED)
find_package(Qt5Widgets CONFIG REQUIRED)

target_link_libraries(Animera
    PRIVATE
    docopt_s
    png
    Qt5::Core
    Qt5::CorePrivate
    Qt5::WinMain
    Qt5::Gui
    Qt5::GuiPrivate
    Qt5::Widgets
    Qt5::WidgetsPrivate
)

install(
    DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/"
    DESTINATION "bin"
    FILES_MATCHING
    PATTERN "*.dll"
    PATTERN "qt.conf"
)

install(
    TARGETS Animera
    DESTINATION "bin"
)

install(
    FILES COPYING
    DESTINATION "."
)

set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "Animera")
set(CPACK_PACKAGE_VENDOR "Indiana Kernick")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")
set(CPACK_STRIP_FILES ON)
set(CPACK_GENERATOR "NSIS64")
set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/Resources/Logo/appicon.ico")
set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/Resources/Logo/appicon.ico")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
# Control panel
set(CPACK_NSIS_DISPLAY_NAME "Animera")
set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\Animera.exe")
set(CPACK_NSIS_URL_INFO_ABOUT "${CMAKE_PROJECT_HOMEPAGE_URL}")
# Start menu
set(CPACK_PACKAGE_EXECUTABLES "Animera;Animera")

set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
    WriteRegStr HKCR '.animera' '' 'Animera.Sprite'
    WriteRegStr HKCR 'Animera.Sprite' '' 'Animera Sprite'
    WriteRegStr HKCR 'Animera.Sprite\\\\shell' '' 'open'
    WriteRegStr HKCR 'Animera.Sprite\\\\shell\\\\open\\\\command' '' '\\\"\$INSTDIR\\\\bin\\\\Animera.exe\\\" open \\\"%1\\\"'
    WriteRegStr HKCR 'Animera.Sprite\\\\shell\\\\edit\\\\command' '' '\\\"\$INSTDIR\\\\bin\\\\Animera.exe\\\" open \\\"%1\\\"'
    WriteRegStr HKCR 'Animera.Sprite\\\\DefaultIcon' '' '\$INSTDIR\\\\bin\\\\Animera.exe,1'
    
    WriteRegStr HKCR '.png\\\\OpenWithProgids' 'Animera.PNG' ''
    WriteRegStr HKCR 'Animera.PNG\\\\shell' '' 'open'
    WriteRegStr HKCR 'Animera.PNG\\\\shell\\\\open\\\\command' '' '\\\"\$INSTDIR\\\\bin\\\\Animera.exe\\\" open \\\"%1\\\"'
    ; These are required for some reason
    WriteRegStr HKCR 'Animera.PNG\\\\Application' 'ApplicationName' 'Animera'
    WriteRegStr HKCR 'Animera.PNG\\\\Application' 'ApplicationCompany' 'Indiana Kernick'
    WriteRegStr HKCR 'Animera.PNG\\\\Application' 'ApplicationIcon' '\$INSTDIR\\\\bin\\\\Animera.exe'
")

set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
    DeleteRegKey HKCR '.animera' ''
    DeleteRegKey HKCR 'Animera.Sprite' ''
    DeleteRegValue HKCR '.png\\\\OpenWithProgids' 'Animera.PNG'
    DeleteRegKey HKCR 'Animera.PNG' ''
")

include(CPack)
