#!/usr/bin/env python
import os
import sys

env = SConscript("godot-cpp/SConstruct")

# For reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# tweak this if you want to use different folders, or more folders, to store your source code in.
env.Append(CPPPATH=["extension/include/", 
                    "extension/include/biomes/",
                    "extension/include/generators/",
                    "extension/include/decorations/",
                    "extension/include/structures",
                    "extension/src/",
                    "extension/src/biomes/",
                    "extension/src/generators/",
                    "extension/src/decorations/",
                    "extension/src/structures"])

sources =  Glob("extension/src/*.cpp")
sources += Glob("extension/src/generators/*.cpp")
sources += Glob("extension/src/decorations/*.cpp")
sources += Glob("extension/src/biomes/*.cpp")
sources += Glob("extension/src/structures/*.cpp")

if env["platform"] == "linux":
    env.Append(CCFLAGS=["-fvisibility=default"])
    env.Append(LIBS=["stdc++"])  

if env["platform"] == "macos":
    library = env.SharedLibrary(
        "game/bin/libgdblocks.{}.{}.framework/libgdblocks.{}.{}".format(env["platform"], env["target"], env["platform"], env["target"]),
        source=sources,
    )
else:
    library = env.SharedLibrary(
        "game/bin/libgdblocks{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

Default(library)
