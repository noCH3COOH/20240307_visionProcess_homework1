#include "main.h"
#include <iostream>
#include <fstream>

using namespace std;

int main()
{
    ifstream src;
    src.open("test.yuz", ios::binary);
    if(!src.is_open())
    {
        cout << "[ERROR] 文件未打开" << endl;
        return 1;
    }
    
    

    return 0;
}