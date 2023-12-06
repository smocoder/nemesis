echo Cleaning sdk folders...
attrib -r -h -s /s sdk\sln\*.suo
del /s /q ..\sln\*.ncb
del /s /q ..\sln\*.suo
del /s /q ..\sln\*.user
del /s /q ..\sln\*.sdf
del /s /q ..\sln\*.VC.db
del /s /q ..\tmp\*.*
rd /s /q ..\sln\.vs
rd /s /q ..\tmp
rd /s /q ..\lib
rd /s /q ..\dll
rd /s /q ..\bin
