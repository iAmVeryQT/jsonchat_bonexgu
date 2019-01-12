rd ".\bin_jsonchat_windows" /S /Q
md ".\bin_jsonchat_windows"
xcopy "..\..\build-chatting-Desktop_Qt_5_12_0_MinGW_64_bit-Release\release\*.exe" ".\bin_jsonchat_windows" /F /R /Y /I /S
"C:\Qt\5.12.0\mingw73_64\bin\windeployqt.exe" ".\bin_jsonchat_windows"
xcopy "C:\Qt\5.12.0\mingw73_64\bin\lib*.dll" ".\bin_jsonchat_windows" /F /R /Y /I /S
