rd ".\jsonchat_windows" /S /Q
md ".\jsonchat_windows"
xcopy "..\..\build-chatting-Desktop_Qt_5_12_0_MinGW_64_bit-Release\release\*.exe" ".\jsonchat_windows" /F /R /Y /I /S
"C:\Qt\5.12.0\mingw73_64\bin\windeployqt.exe" ".\jsonchat_windows"
xcopy "C:\Qt\5.12.0\mingw73_64\bin\lib*.dll" ".\jsonchat_windows" /F /R /Y /I /S
