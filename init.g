#
# meataxe64: meataxe64
#
# Reading the declaration part of the package.
#
_PATH_SO:=Filename(DirectoriesPackagePrograms("meataxe64"), "meataxe64.so");
if _PATH_SO <> fail then
    LoadDynamicModule(_PATH_SO);
fi;
Unbind(_PATH_SO);

ReadPackage( "meataxe64", "gap/meataxe64.gd");
ReadPackage( "meataxe64", "gap/echelize.gd");
