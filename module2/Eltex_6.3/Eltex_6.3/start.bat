@echo off
mkdir libs 2>nul

echo Building libraries...
gcc -c -fPIC functions\add.c -o functions\add.o
gcc -shared -fPIC functions\add.o -o libs\add.dll

gcc -c -fPIC functions\subtract.c -o functions\subtract.o
gcc -shared -fPIC functions\subtract.o -o libs\subtract.dll

gcc -c -fPIC functions\multiply.c -o functions\multiply.o
gcc -shared -fPIC functions\multiply.o -o libs\multiply.dll

gcc -c -fPIC functions\fadd.c -o functions\fadd.o
gcc -shared -fPIC functions\fadd.o -o libs\fadd.dll

gcc -c -fPIC functions\fsubtract.c -o functions\fsubtract.o
gcc -shared -fPIC functions\fsubtract.o -o libs\fsubtract.dll

gcc -c -fPIC functions\fmultiply.c -o functions\fmultiply.o
gcc -shared -fPIC functions\fmultiply.o -o libs\fmultiply.dll

gcc -c -fPIC functions\fdivide.c -o functions\fdivide.o
gcc -shared -fPIC functions\fdivide.o -o libs\fdivide.dll

echo Building main program...
gcc main.c -o calculator.exe

echo Build complete! Run with: calculator.exe

timeout /t 10 /nobreak