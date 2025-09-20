@echo off
:: appname 程序名
:: inputname 输入文件名
:: outputname 输出文件名
:: resultname 程序控制台输出重定向文件名

set appname="D:\homeworkspace\network\myTCP\myTCP\Release\myTCP.exe"
set inputname="D:\homeworkspace\network\myTCP\myTCP\myTCP\input.txt"
set outputname="D:\homeworkspace\network\myTCP\myTCP\myTCP\output.txt"
set resultname="result.txt"

for /l %%i in (1,1,10) do (
    echo Test %appname% %%i:
    %appname% > %resultname% 2>&1
    fc /N %inputname% %outputname%
)
pause