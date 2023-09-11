#!/bin/bash

rm -Rf deploy-appimage
mkdir deploy-appimage

echo "**************** Building Linux AppImage ****************"

export QMAKE=$QTDIR/../../../bin/qmake

linuxdeploy-x86_64.AppImage \
    --appdir ./deploy-appimage/ZapFeedReader.AppDir \
    -e ./build/client-desktop/release/zapfeedreader-client \
    -d ./deploy/client-desktop/AppImage/ZapFeedReader.desktop \
    -i ./deploy/client-desktop/AppImage/ZapFeedReader.svg \
    --plugin qt 

appimagetool-x86_64.AppImage ./deploy-appimage/ZapFeedReader.AppDir ./deploy-appimage/ZapFeedReader.AppImage
