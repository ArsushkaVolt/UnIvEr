#include "pch.h"
#include "file.h"

DLL_EXPORT fstream* open(char* path, bool mode)
{
    fstream* file;
    if (mode) {
        file = new fstream();
        file->open(path, ios_base::in);
    }
    else
    {
        file = new fstream();
        file->open(path, ios_base::out | ios_base::trunc);
    }
    if (!file->is_open())
        throw invalid_argument("File Open Error");
    return file;
}

DLL_EXPORT void close(fstream* file)
{
    if (!file->is_open())
        throw invalid_argument("file not open");
    file->close();
    delete file;
}

DLL_EXPORT bool read(fstream* file, int num, char* word)
{
    if (!file->is_open())
        throw invalid_argument("file not open");
    file->clear();
    file->seekg(0);
    if (num<1 || num>length(file))
        return false;
    string res;
    int i = 0;
    while (i < num)
    {
        *file >> res;
        i++;
    }
    strcpy_s(word, 255,res.c_str());
    file->clear();
    file->seekg(0);
    return true;
}

DLL_EXPORT void write(fstream* file, char* text)
{
    if (!file->is_open())
        throw invalid_argument("file not open");
    file->clear();
    file->seekg(0);
    file->write(text, strlen(text));
}

DLL_EXPORT int length(fstream* file)
{
    if (!file->is_open())
        throw invalid_argument("file not open");
    int res = 0;
    string tmp;
    while (!file->eof())
    {
        *file >> tmp;
        res++;
    }
    file->clear();
    file->seekg(0);
    return res;
}
