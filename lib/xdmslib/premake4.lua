solution "xdmslib"
configurations {"DebugLib","ReleaseLib"}
platforms {"x64","x32"}

project "xdmslib"
    language "C"
    kind "StaticLib"
    files {"*.c"}
configuration "DebugLib"
    flags {"ExtraWarnings","Symbols"}
    targetdir "lib/debug"
configuration "ReleaseLib"
    flags {"ExtraWarnings","OptimizeSpeed"}
    targetdir "lib/release"

    

