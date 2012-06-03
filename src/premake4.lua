solution "AmigaDX"
configurations {"DebugLib","ReleaseLib"}
platforms {"x64","x32"}

project "AmigaDX"
    language "C"
    kind "SharedLib"
    files {"*.c", "*.rc"}
    links {"zlib","adflib","xdmslib"}
    includedirs { "../lib/adflib", "../lib/xdmslib", "../lib/zlib" }
    targetdir "../install"
    defines {"WCX_PLUGIN_EXPORTS"}

configuration "x64"
    targetextension ".wcx64"

configuration "x32"
    targetextension ".wcx"

configuration "DebugLib"
    libdirs { "../lib/adflib/lib/debug", "../lib/xdmslib/lib/debug", "../lib/zlib/lib/debug" }
    flags {"ExtraWarnings","Symbols"}
    targetdir "lib/debug"
    postbuildcommands { "cp lib/debug/AmigaDX.wcx64 ../install" }
    defines {"DEBUG"}

configuration "ReleaseLib"
    libdirs { "../lib/adflib/lib/release", "../lib/xdmslib/lib/release", "../lib/zlib/lib/release" }
    flags {"ExtraWarnings","OptimizeSpeed"}
    targetdir "lib/release"
    postbuildcommands { "cp lib/release/AmigaDX.wcx64 ../install" }
    defines {"NDEBUG"}

