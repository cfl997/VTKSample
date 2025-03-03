#pragma once


#ifdef CGRENDERVIEWEXPORT
#define CGRENDERVIEW_API __declspec(dllexport) 
#else
#define CGRENDERVIEW_API __declspec(dllimport)
#endif // CGRENDERVIEWEXPORT
