#!/bin/sh
cd /Users/jsonchat/build
rm -rf jsonchat_macx
mkdir jsonchat_macx
cp -R ../../build-chatting-Desktop_Qt_5_12_0_clang_64bit-Release/chatting.app jsonchat_macx/chatting.app
/Users/cto/Qt/5.12.0/clang_64/bin/macdeployqt jsonchat_macx/chatting.app
