@echo off
:: appname ������
:: inputname �����ļ���
:: outputname ����ļ���
:: resultname �������̨����ض����ļ���

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