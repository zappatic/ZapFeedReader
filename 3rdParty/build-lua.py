#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import urllib.request
import tarfile
import subprocess


root = os.path.dirname(os.path.realpath(__file__))
lua_dir = os.path.join(root, "lua")
lua_install_dir = os.path.join(root, "lua-install")

lua_version = "5.4.2"
archive_subdir = os.path.join(lua_dir, "lua54")

if not os.path.isdir(lua_dir):
    os.mkdir(lua_dir)

lua_tarball = os.path.join(lua_dir, f"lua-{lua_version}_Sources.tar.gz")
if not os.path.exists(lua_tarball):
    print("Downloading lua from Sourceforge")
    download_url = f"https://sourceforge.net/projects/luabinaries/files/{lua_version}/Docs%20and%20Sources/lua-{lua_version}_Sources.tar.gz/download"
    urllib.request.urlretrieve(download_url, lua_tarball)

if not os.path.isdir(archive_subdir):
    print("Extracting lua tarball")
    archive = tarfile.open(lua_tarball)
    archive.extractall(lua_dir)
    archive.close()

    makefile = os.path.join(archive_subdir, "Makefile")
    makefile_contents = ""
    with open(makefile) as f:
        makefile_contents = f.read()
        makefile_contents = makefile_contents.replace("INSTALL_TOP= /usr/local", f"INSTALL_TOP= {lua_install_dir}")
    with open(makefile, 'w') as f:
        f.write(makefile_contents)

if not os.path.isdir(lua_install_dir):
    os.mkdir(lua_install_dir)
    subprocess.run(["make", "-j"], cwd=archive_subdir)
    subprocess.run(["make", "install"], cwd=archive_subdir)

    pkgconfig_dir = os.path.join(lua_install_dir, "lib", "pkgconfig")
    os.mkdir(pkgconfig_dir)
    with open(os.path.join(pkgconfig_dir, "lua.pc"), 'w') as f:
        f.write(f"prefix={lua_install_dir}\n")
        f.write("libdir=${prefix}/lib\n")
        f.write("includedir=${prefix}/include\n")
        f.write("\n")
        f.write("Name: lua\n")
        f.write("Description: Lua\n")
        f.write(f"Version: {lua_version}\n")
        f.write("Cflags: -I${includedir}\n")
        f.write("Libs: -L${libdir} -llua")

