#include "Error.hpp"
#include <cstdio>
#include <cstdarg>
#include <iostream>
#include <windows.h>

#define SPRINT_BUF_SIZE 1024
static char sprint_buf[SPRINT_BUF_SIZE];
static HANDLE hstdout;

void log_system_init()
{
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	//freopen("CON", "w", stdout);

	//HANDLE hstdin = GetStdHandle(STD_INPUT_HANDLE);
	hstdout = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hstdout, &csbi);
}

void info(const char *fmt, ...)
{
	SetConsoleTextAttribute(hstdout, 0x0F);
	std::cout << "[info]";
	va_list args;
	va_start(args, fmt);
	//int n = vsprintf(sprint_buf, fmt, args);
	int n = vsprintf_s(sprint_buf, fmt, args);
	va_end(args);
	//write(stdout, sprint_buf, n);
	std::cout.write(sprint_buf, n);
}

void warning(const char *fmt, ...)
{
	SetConsoleTextAttribute(hstdout, 0x0E);
	std::cout << "[warning]";
	va_list args;
	va_start(args, fmt);
	//int n = vsprintf(sprint_buf, fmt, args);
	int n = vsprintf_s(sprint_buf, fmt, args);
	va_end(args);
	//write(stdout, sprint_buf, n);
	std::cout.write(sprint_buf, n);
}

void error(const char *fmt, ...)
{
	SetConsoleTextAttribute(hstdout, 0x0C);
	std::cout << "[error]";
	va_list args;
	va_start(args, fmt);
	//int n = vsprintf(sprint_buf, fmt, args);
	int n = vsprintf_s(sprint_buf, fmt, args);
	va_end(args);
	//write(stdout, sprint_buf, n);
	std::cout.write(sprint_buf, n);
}