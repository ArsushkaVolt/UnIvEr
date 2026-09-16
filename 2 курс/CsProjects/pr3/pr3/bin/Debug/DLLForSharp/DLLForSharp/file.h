#include <fstream>
#include <string>
#include <iostream>

#ifdef DLLFORSHARP_EXPORTS
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __declspec(dllimport)
#endif

using namespace std;

extern "C" DLL_EXPORT fstream* open(char* path, bool mode);
extern "C" DLL_EXPORT void close(fstream* file);
extern "C" DLL_EXPORT bool read(fstream* file, int num, char* word);
extern "C" DLL_EXPORT void write(fstream* file, char* text);
extern "C" DLL_EXPORT int length(fstream* file);