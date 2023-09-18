#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import shutil
import subprocess

shutil.rmtree("deploy-appimage", True)
os.mkdir("deploy-appimage")

e = os.environ
e["QMAKE"] = os.path.realpath(e["QTDIR"] + "/../../../bin/qmake")

subprocess.run(["linuxdeploy-x86_64.AppImage", 
                "--appdir=./deploy-appimage/ZapFeedReader.AppDir",
                "-e", "./build/client-desktop/release/zapfeedreader-client", 
                "-d", "./deploy/client-desktop/AppImage/zapfeedreader-client.desktop",
                "-i", "./deploy/client-desktop/AppImage/zapfeedreader-client.svg", 
                "--plugin", "qt" ], shell=False, env=e)

os.rename("./deploy-appimage/ZapFeedReader.AppDir/AppRun.wrapped", "./deploy-appimage/ZapFeedReader.AppDir/zapfeedreader-client")

with open("./deploy-appimage/ZapFeedReader.AppDir/AppRun", "r") as fp:
    apprun = fp.read()

apprun = apprun.replace("AppRun.wrapped", "zapfeedreader-client")
with open("./deploy-appimage/ZapFeedReader.AppDir/AppRun", "w") as fp:
    fp.write(apprun)

subprocess.run(["appimagetool-x86_64.AppImage", "./deploy-appimage/ZapFeedReader.AppDir", "./deploy-appimage/ZapFeedReader.AppImage"])