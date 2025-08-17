@echo off
cd /d "C:\Users\poyu0\STM32CubeIDE\workspace_1.19.0"

:: 確認有沒有 .gitignore，沒有就建立一份
if not exist ".gitignore" (
    echo Creating default .gitignore...
    (
        echo /.metadata/
        echo /.settings/
        echo **/Debug*/
        echo **/Release*/
        echo **/build*/
        echo **/*.o
        echo **/*.d
        echo **/*.elf
        echo **/*.bin
        echo **/*.hex
        echo **/*.map
        echo **/*.list
    ) > .gitignore
    git add .gitignore
    git commit -m "Add default .gitignore"
)

:: 顯示目前所在位置
echo Current directory:
cd

:: 讓你輸入 commit 訊息，若沒輸入就用預設
set /p msg=Enter commit message (default: auto update): 
if "%msg%"=="" set msg=auto update

:: 提交 & 推送
git add .
git commit -m "%msg%"
git push origin master

pause
