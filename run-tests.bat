@echo off
start cmd.exe @cmd /k "pushd bin && tests.exe && popd"