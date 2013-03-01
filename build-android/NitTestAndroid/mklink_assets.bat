rmdir /S /Q assets

mkdir assets

mklink /D assets\packs-nit ..\..\..\packs-nit
mklink /D assets\packs-tests ..\..\..\packs-tests

mklink assets\nit-test.app.cfg ..\..\..\nit-test.app.cfg

pause