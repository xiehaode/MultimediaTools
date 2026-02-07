#include "pch.h"
#include <Windows.h>
#include <Python.h>
#include <stdio.h>

int main() {
    printf("Testing Python C API linking...\n");
    
    // Test basic Python initialization
    Py_Initialize();
    printf("Python initialized successfully\n");
    
    // Test some basic Python C API functions
    printf("Python version: %s\n", Py_GetVersion());
    printf("Python platform: %s\n", Py_GetPlatform());
    
    Py_Finalize();
    printf("Python finalized successfully\n");
    
    return 0;
}