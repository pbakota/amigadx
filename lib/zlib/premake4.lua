solution "zlib"
configurations {"DebugLib","ReleaseLib"}
platforms {"x64","x32"}

project "zlib"
    language "C"
    kind "StaticLib"
    files {"*.c"}
configuration "DebugLib"
    flags {"ExtraWarnings","Symbols"}
    targetdir "lib/debug"
configuration "ReleaseLib"
    flags {"ExtraWarnings","OptimizeSpeed"}
    targetdir "lib/release"
    
