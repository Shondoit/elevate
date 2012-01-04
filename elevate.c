#include <windows.h>
#include <tchar.h>
#include <ctype.h>
#include <stdio.h>
#include <shlwapi.h>

int WINAPI _tWinMain(
	HINSTANCE hInstance,
	HINSTANCE prevInstance,
	LPTSTR lpCmdLine,
	int nCmdShow
) {
	//Get the required executable path (or filename).
	LPTSTR arg0 = (LPTSTR)malloc(MAX_PATH * sizeof(TCHAR));
	//Set arg0 to contain the first argument.
	_tcsncpy(arg0, lpCmdLine, MAX_PATH);
	PathRemoveArgs(arg0);
	PathUnquoteSpaces(arg0);
	//Set lpCmdLine to only contain the arguments
	//striparg(&lpCmdLine, (LPTSTR*)&arg0, MAX_PATH);
	PathGetArgs(lpCmdLine);
	
	//If the given argument is relative, search for it in the current directory
	if (PathIsRelative(arg0)) {
		//Save our original relative name.
		LPTSTR lpRelPath = arg0;

		//Allocate memory and receive the current directory.
		int dirlen = GetCurrentDirectory(0, NULL);
		LPTSTR lpCurDir = (LPTSTR)malloc((dirlen + 1) * sizeof(TCHAR));
		GetCurrentDirectory(dirlen, lpCurDir);

		//Change our executable path to an absolute path using the current dir.
		arg0 = (LPTSTR)malloc(MAX_PATH * sizeof(TCHAR));
		PathCombine(arg0, lpCurDir, lpRelPath);
		
		//We don't need this anymore, free it.
		free(lpCurDir);
		
		//If the file can be found in the current directory we'll use this.
		//Otherwise we'll look for it in %PATH%
		if (PathFileExists(arg0)) {
			//arg0 is what we want. We don't need lpRelPath anymore.
			free(lpRelPath);
		} else {
			//Clear our absolute path.
			free(arg0);
			//Use our relative path to look for in %PATH%
			PathFindOnPath(lpRelPath, NULL);
			//Set arg0, since this is what we'll use.
			arg0 = lpRelPath;
		}
	}

	//Normalize our path.
	LPTSTR lpFile = (LPTSTR)malloc(MAX_PATH * sizeof(TCHAR));
	GetFullPathName(arg0, MAX_PATH, lpFile, NULL);

	//We've got our normalized path. arg0 is not needed anymore.
	free(arg0);

	//Create our ShellExecuteInfo and set all the required parameters.
	SHELLEXECUTEINFO lpExecInfo;
	ZeroMemory(&lpExecInfo, sizeof(lpExecInfo));

	lpExecInfo.cbSize = sizeof(lpExecInfo);
	lpExecInfo.fMask =
		SEE_MASK_NOCLOSEPROCESS |
		SEE_MASK_DOENVSUBST |
		SEE_MASK_FLAG_NO_UI;
	lpExecInfo.hwnd = NULL;
	lpExecInfo.lpFile = lpFile;
	lpExecInfo.lpParameters = lpCmdLine;
	lpExecInfo.lpDirectory = NULL;
	lpExecInfo.nShow = SW_SHOWNORMAL;

	//We'll start with "runas", since this is what we want.
	lpExecInfo.lpVerb = TEXT("runas");
	//Execute, giving our parameters.
	if (!ShellExecuteEx(&lpExecInfo)) {
		//Clean up our left-over file name
		free(lpFile);

		//If the executable does not support "runas", give up and return error.
		return GetLastError();
	}
	//If the process is running succesfully, patiently wait for it.
	WaitForSingleObject(lpExecInfo.hProcess, INFINITE);

	//Get the exit code to return to our caller.
	DWORD exitCode;
	GetExitCodeProcess(lpExecInfo.hProcess, &exitCode);

	//Clean up our left-over file name
	free(lpFile);

	//Return the exit code given by our child process.
	return exitCode;
}
