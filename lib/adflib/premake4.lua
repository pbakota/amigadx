solution "adflib"
configurations {"DebugLib","ReleaseLib"}
platforms {"x64","x32"}

project "adflib"
    language "C"
    kind "StaticLib"
    files {"*.c", "generic/*.c"}
    includedirs {"generic"}
configuration "DebugLib"
    flags {"ExtraWarnings","Symbols"}
    targetdir "lib/debug"
configuration "ReleaseLib"
    flags {"ExtraWarnings","OptimizeSpeed"}
    targetdir "lib/release"

    

