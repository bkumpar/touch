// touch.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include <ctime>
#include <locale>
#include <iomanip>

#include "BArgParser.h"
#include "BArgItem.h"

#define CONSLOE_OUTPUT true

DWORD touchFile(std::wstring fileName, FILETIME *lpCreationTime, FILETIME *lpLastAccessTime, FILETIME *lpLastWriteTime )
{
    HANDLE  hFile = CreateFile(
        (LPCWSTR)fileName.c_str()  /*LPCWSTR lpFileName */ 
        , FILE_WRITE_ATTRIBUTES             /*DWORD                 dwDesiredAccess*/ 
        , 0                                 /*DWORD                 dwShareMode*/
        , NULL                              /*LPSECURITY_ATTRIBUTES lpSecurityAttributes*/
        , OPEN_EXISTING                     /*DWORD                 dwCreationDisposition*/
        , FILE_ATTRIBUTE_NORMAL             /*DWORD                 dwFlagsAndAttributes*/
        , NULL                              /*HANDLE                hTemplateFile*/ );
    DWORD lastError;
    if(hFile!=0)
    {
        lastError = GetLastError();
        if(lastError)
        {
            std::wcerr << L"CreateFile error = 0x"  << std::hex << lastError << std::endl;
            return lastError;
        }
    }
    BOOL retval = SetFileTime( hFile, lpCreationTime, lpLastAccessTime, lpLastWriteTime);
    if( retval!=TRUE)
    {
        lastError = GetLastError();
        if(lastError)
        {
            std::wcerr << L"SetFileTime error = 0x"  << std::hex << lastError << std::endl;
            return lastError;
        }
    }
    retval = CloseHandle(hFile);
    if( retval!=TRUE)
    {
        lastError = GetLastError();
        if(lastError)
        {
            std::wcerr << L"CloseHandle error = 0x"  << std::hex << lastError << std::endl;
            return lastError;
        }
    }
    return S_OK;
}

 void showMessage(std::wstring errorMessage )
 {
    if(CONSLOE_OUTPUT)
        std::wcout << errorMessage  << std::endl;
    else 
        MessageBox(NULL, errorMessage.c_str(), L"Touch.exe", MB_OK | MB_ICONINFORMATION);
 }

int _tmain(int argc, _TCHAR* argv[])
{
    FILETIME timeStamp;
    FILETIME *lpCreationTime;
    FILETIME *lpLastAccessTime;
    FILETIME *lpLastWriteTime;    

    BArgParser bap(argc, argv, L"touch - change create, access and write time of given file(s).\nUsage: touch [options] file name(s)\nOptions:") ;
    BArgItem<void> arg1(L"-h", L"--help", false, L"Show help and exit.");
    bap.add(&arg1);
    BArgItem<void> arg2(L"-c", L"--creation-time", false, L"Set creation time to value specified in -t argument");
    bap.add(&arg2);
    BArgItem<void> arg3(L"-a", L"--last-access-time", false, L"Set last access time to value specified in -t argument");
    bap.add(&arg3);
    BArgItem<void> arg4(L"-w", L"--last-write-time", false, L"Set last write time to value specified in -t argument");
    bap.add(&arg4);
    BArgItem<time_t> arg5(L"-t", L"--time", true, L"Timestamp to apply in -a -c -w switches. If -t ommited, current timestamp is used.");
    bap.add(&arg5);
    bap.parse();

    if(argc == 1 || arg1.found())
    {
        showMessage(bap.help());
        return 0;
    }

    if(arg5.found())
    {
        time_t tt  = arg5.value();
		LONGLONG ll = Int32x32To64(tt, 10000000) + 116444736000000000;
		timeStamp.dwLowDateTime = (DWORD) ll;
		timeStamp.dwHighDateTime = ll >>32;
    }
    else
    {   // Gets the current system time
        SYSTEMTIME st;
        GetSystemTime(&st);                     
        SystemTimeToFileTime(&st, &timeStamp);  
    }

    // set creation time
    lpCreationTime = arg2.found() ? &timeStamp : NULL;

    // set last access time if parameter -a presented or creation time changed
    lpLastAccessTime = arg3.found() || arg2.found() ? &timeStamp : NULL;

    // set last write time if parameter -w presented or creation time changed
    lpLastWriteTime  = arg4.found() || arg2.found()? &timeStamp : NULL;


	std::vector<std::wstring> tailArguments;
	bap.getTailArguments(tailArguments);
	int retval = -1;
	if(tailArguments.size()>0)
	{
		for( int i = 0; i<tailArguments.size(); i++)
		{
			std::wstring fileName(tailArguments[i]) ;
			retval = touchFile(fileName, lpCreationTime, lpLastAccessTime, lpLastWriteTime );
			if(retval!=S_OK)
			{
				return retval;
			}
		}
	}
    else
    {
        showMessage( L"Missing file name(s)");
    }

	return S_OK;
}

