#
# Build with CentOS 7
# ===================
#
# Use qt.io's online installer for Qt 5.15.x to build linuxdeployqt.
# Use build instructions at the end to compile Qt 6.5.0 which we use to
# compile Frida.
#
# We need proper C++ support:
#
# yum install centos-release-scl
# yum install devtoolset-11-gcc devtoolset-11-gcc-c++
# scl enable devtoolset-11 -- bash
#
# Do not forget yum -y install mesa-libGL-devel.x86_64 to satisfy -lGL.
#
# And we need yum -y install patchelf, too.
#
# Build linuxdeployqt from source as their AppImage does not run on CentOS 7.
# Simply build the .pro file with qmake and make. Use patchelf to set its
# rpath as we are not making an AppImage out of it, and copy the binary
# to somewhere in your $PATH. Note that linuxdeployqt only builds against
# Qt 5.x as it uses deprecated APIs, but it can be used perfectly fine  to
# package Qt 6.x applications.
#
# There is no appstreamcli for CentOS 7, so ignore the warning. We don't use
# it anyway.
#
# For appimagetool, we need:
#
# export ARCH=x86_64
#
# Rename the appimagetool AppImage to appimagetool somewhere in your $PATH
# and be sure it is executable and it's the x86_64 version (not i686).
#
# And finally, run:

linuxdeployqt                   \
    frida                       \
    -appimage                   \
    -no-translations            \
    -no-copy-copyright-files    \
    -extra-plugins=platforms,wayland-decoration-client,wayland-graphics-integration-client,wayland-shell-integration,xcbglintegrations,egldeviceintegrations

# Wait, and succes!

#
# Building Qt 6.5.0 for CentOS 7
# ==============================
#
# The Qt Company's online installer installs a version of Qt6 that does not
# run on CentOS 7. Here's how to build it from source.
#
# Install the full devtoolset-11 
#
# yum -y install devtoolset-11
# scl enable devtoolset-11 -- bash
#
# Install the latest CMake and Ninja from source (or install their provided
# Linux binaries in your $PATH)
#
# Install libxcb, mesa-Gl, mesa-EGL, wayland, libX development packages
# Especially xcb-proto, xcb-util-*-devel, and compat-libxcb
#
# Download qt-everywhere-src-6.5.0.tar.xz and unpack.
#
# mkdir qt6-build && cd qt6-build
#
# ../qt-everywhere-src-6.5.0/configure -prefix $HOME/Qt/6.5.0 -opensource -confirm-license -release -nomake tests -nomake examples -strip -pch -no-warnings-are-errors -skip qtdoc,qtwebengine,qttranslations -qt-libpng -qt-libjpeg -qt-pcre -qt-zlib -qt-freetype -xcb -xcb-xlib -disable-deprecated-up-to 0x050F00
#
# NPROC=`cat /proc/cpuinfo | grep ^processor | wc -l`
#
# cmake --build . --parallel $NPROC
# cmake --install .
#

