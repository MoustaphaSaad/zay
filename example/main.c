#pragma once
#include <stdio.h>
#include <stdlib.h>

#define MyFive do {\
	return 5;\
}while(0)

inline static int foo()
{
	MyFive;
}

int main()
{
	return 0;
}