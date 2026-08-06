@echo off
setlocal enabledelayedexpansion
chcp 65001 >nul
cd /d "%~dp0"

set REMOTE_URL=https://github.com/suqiongcxk/CXKGIT
set BRANCH=main

echo ========================================
echo   zhukong 工程一键上传脚本
echo   仓库: %REMOTE_URL%
echo ========================================
echo.

:: 1. 初始化 Git 仓库
if not exist ".git" (
    echo [1/6] 初始化 Git 仓库...
    git init
    git checkout -B %BRANCH%
    echo   [OK] Git 仓库已初始化
) else (
    echo [1/6] Git 仓库已存在，跳过初始化
)

:: 2. 配置远程仓库
echo [2/6] 配置远程仓库...
for /f "tokens=*" %%i in ('git remote get-url origin 2^>nul') do set EXISTING=%%i
if defined EXISTING (
    if not "!EXISTING!"=="%REMOTE_URL%" (
        echo   [!] 远程地址不匹配，更新中...
        git remote set-url origin %REMOTE_URL%
    ) else (
        echo   [OK] 远程已配置，跳过
    )
) else (
    git remote add origin %REMOTE_URL%
    echo   [OK] 已添加远程仓库
)

:: 3. 暂存所有文件
echo [3/6] 暂存所有文件...
git add -A
echo   [OK] 所有文件已暂存

:: 4. 显示变更摘要
echo [4/6] 变更摘要:
for /f "tokens=*" %%i in ('git status --short') do (
    echo   %%i
    set /a COUNT+=1
)
if not defined COUNT (
    echo   [!] 没有变更，无需提交
    exit /b 0
)
echo   总计: !COUNT! 个变更文件

:: 5. 输入提交信息
echo.
echo ========================================
echo   请输入本次改动简介:
echo   (直接回车则使用默认信息)
echo ========================================
set /p USER_MSG="Message: "

set TS=%date:~0,4%-%date:~5,2%-%date:~8,2% %time:~0,8%
if "%USER_MSG%"=="" (
    set COMMIT_MSG=zhukong 工程更新 - %TS%
) else (
    set COMMIT_MSG=%USER_MSG% (%TS%)
)

:: 6. 提交
echo.
echo [5/6] 提交: !COMMIT_MSG!
git commit -m "!COMMIT_MSG!"
if %errorlevel% neq 0 (
    echo   [X] 提交失败!
    exit /b 1
)
echo   [OK] 提交成功

:: 6. 推送到远程
echo [6/6] 推送到远程仓库...
git pull --rebase origin %BRANCH% 2>nul
git push -u origin %BRANCH% 2>&1
if %errorlevel% neq 0 (
    echo   [X] 推送失败!
    echo.
    echo   可能的原因:
    echo   1. GitHub 登录过期 -- 运行 gh auth login
    echo   2. 无网络 -- 检查网络连接
    echo   3. 远程有冲突 -- 先运行: git pull origin %BRANCH% --rebase
    exit /b 1
)

echo   [OK] 推送成功!
echo.
echo ========================================
echo   [DONE] 工程已成功上传到 GitHub！
echo   仓库: %REMOTE_URL%
echo ========================================
echo.
echo 克隆命令:
echo   git clone %REMOTE_URL%
