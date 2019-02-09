rd ".\bin_jsonserver_windows" /S /Q
md ".\bin_jsonserver_windows"
xcopy "..\..\build-jsonserver-Desktop_Qt_5_12_0_MSVC2017_64bit-Release\release\*.exe" ".\bin_jsonserver_windows" /F /R /Y /I /S
"C:\Qt\5.12.0\msvc2017_64\bin\windeployqt.exe" ".\bin_jsonserver_windows"
xcopy "C:\Qt\5.12.0\msvc2017_64\bin\lib*.dll" ".\bin_jsonserver_windows" /F /R /Y /I /S
