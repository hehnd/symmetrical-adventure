@echo off
chcp 65001 >nul
echo ========================================
echo   Codeforces 用户比赛分析工具
echo ========================================
echo.

cd /d "%~dp0"

echo 步骤1: 编译C程序...
gcc -o cf_analyzer.exe src/cf_fetcher.c lib/cJSON.c -O2
if %errorlevel% neq 0 (
    echo 编译失败！
    pause
    exit /b 1
)
echo 编译成功！
echo.

echo 步骤2: 获取Codeforces数据...
cf_analyzer.exe data/users.txt
if %errorlevel% neq 0 (
    echo 数据获取失败！
    pause
    exit /b 1
)
echo.

echo 步骤3: 打开HTML报告...
start "" output/cf_report.html
echo.
echo 完成！
pause
