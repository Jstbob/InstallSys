#include<Windows.h>
#include<winternl.h>
#include<memory.h>
#include<stdio.h>
#include<string>
#include<winnt.h>
#include<winver.h>
#include"printsys.h"

#pragma comment(lib,"ntdll.lib")
#pragma comment(lib, "version.lib")

typedef struct _SYSTEM_MODULE
{
	HANDLE Section;
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR FullPathName[256];
} SYSTEM_MODULE, * PSYSTEM_MODULE;


typedef struct _SYSTEM_MODULE_INFORMATION {
	ULONG                ModulesCount;
	SYSTEM_MODULE        Modules[1];
} SYSTEM_MODULE_INFORMATION, * PSYSTEM_MODULE_INFORMATION;

typedef struct LANGANDCODEPAGE {
	WORD langID;            // language ID  
	WORD charset;           // character set (code page) 
}Translate, * lpTranslate;

std::string GetPath(char* name) {
	std::string old(name);
	std::string filePath;
	if (old.find("\\??") == 0) {
		filePath = old.substr(4);
	}
	else {
		std::string re_old("\\SystemRoot");
		std::string re_new("\\Windows");
		auto npos = old.find(re_old) + re_old.length();
		filePath = std::string("C:") + re_new + old.substr(npos);
	}
	return filePath;
}

std::string GetCompanyName(std::string filePath) {
	DWORD fileInfoSize = GetFileVersionInfoSizeA(filePath.c_str(), NULL);
	if (0 == fileInfoSize) {
		return std::string("");
	}

	PVOID buff = (PVOID)malloc(fileInfoSize);
	if (!GetFileVersionInfoA(filePath.c_str(), NULL, fileInfoSize, buff)) {
		free(buff);
		RaiseException(1, 0, 0, NULL);
	}

	lpTranslate tr;
	UINT puLen;
	if (!VerQueryValueA(buff, "\\VarFileInfo\\Translation", (LPVOID*)&tr, &puLen)) {
		free(buff);
		RaiseException(1, 0, 0, NULL);
	}

	char strInfo[50];
	sprintf_s(strInfo, "\\StringFileInfo\\%04x%04x\\CompanyName", tr->langID, tr->charset);

	LPVOID lpBuffer;
	if (!VerQueryValueA(buff, strInfo, &lpBuffer, &puLen)) {
		free(buff);
		RaiseException(1, 0, 0, NULL);
	}

	std::string company((char*)lpBuffer);
	free(buff);
	return company;
}


bool PrintSysCompany() {
	const int MAX_COUNT = 1024 * 1024;
	PSYSTEM_MODULE_INFORMATION modeinfo = (PSYSTEM_MODULE_INFORMATION)malloc(MAX_COUNT);
	NTSTATUS status = NtQuerySystemInformation((SYSTEM_INFORMATION_CLASS)0xb, modeinfo, MAX_COUNT, NULL);
	if (!NT_SUCCESS(status)) {
		RaiseException(1, 0, 0, NULL);
	}

	SYSTEM_MODULE module;
	printf("%-30s%-40s%-30s\n", "Sysname", "Company", "Path");
	for (ULONG i = 0; i < modeinfo->ModulesCount; i++) {
		module = modeinfo->Modules[i];
		std::string path = GetPath((char*)module.FullPathName);
		std::string companyName = GetCompanyName(path);
		printf("%-30s%-40s%-30s\n", (char*)(module.FullPathName + module.OffsetToFileName),
			companyName.c_str(), path.c_str());
	}
	free(modeinfo);
	return TRUE;
}


bool PrintExternSys() {
	const int MAX_COUNT = 1024 * 1024;
	PSYSTEM_MODULE_INFORMATION modeinfo = (PSYSTEM_MODULE_INFORMATION)malloc(MAX_COUNT);
	NTSTATUS status = NtQuerySystemInformation((SYSTEM_INFORMATION_CLASS)0xb, modeinfo, MAX_COUNT, NULL);
	if (!NT_SUCCESS(status)) {
		RaiseException(1, 0, 0, NULL);
	}

	SYSTEM_MODULE module;
	printf("%-30s%-30s\n", "Sysname", "Path");
	char buff[4] = { 0, };
	for (ULONG i = 0; i < modeinfo->ModulesCount; i++) {
		module = modeinfo->Modules[i];
		memcpy_s(buff, 3, module.FullPathName, 3);
		if (strcmp(buff, "\\??") == 0) {
			printf("%-30s%-30s\n", (char*)(module.FullPathName + module.OffsetToFileName), (char*)module.FullPathName);
		}
	}
	free(modeinfo);
	return TRUE;
}