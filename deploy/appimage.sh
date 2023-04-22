#
# Build with CentOS 7 and Qt 5.15.2 from qt.io (latest online installer)
#
# We need proper C++ support:
#
# yum install centos-release-scl
# yum install devtoolset-11-gcc devtoolset-11-gcc-c++
# scl enable devtoolset-11 -- bash
#
# Do not forget yum -y install mesa-libGL-devel.x86_64 to satisfy -lGL.
#
# Build linuxdeployqt from source as their AppImage does not run on CentOS7.
# Simply build the .pro file with qmake and make.
#
# We need yum -y install patchelf, too.
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
# For linuxdeployqt, we need:
#
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HOME/Qt/5.15.2/gcc_64/lib 
#
# And finally, run:

linuxdeployqt                   \
    frida                       \
    -appimage                   \
    -no-translations            \
    -no-copy-copyright-files    \
    -extra-plugins=platforms,wayland-decoration-client,wayland-graphics-integration-client,wayland-shell-integration,xcbglintegrations,egldeviceintegrations

# Wait, and succes!
