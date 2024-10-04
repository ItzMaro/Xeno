#include "ntdll.h"
NtQuerySystemInformation_t* NtQuerySystemInformation;
NtDuplicateObject_t* NtDuplicateObject;
NtQueryObject_t* NtQueryObject;
NtQueryInformationThread_t* NtQueryInformationThread;
NtQueryInformationProcess_t* NtQueryInformationProcess;
NtOpenSection_t* NtOpenSection;
NtQuerySection_t* NtQuerySection;
NtCreateSection_t* NtCreateSection;
NtMapViewOfSection_t* NtMapViewOfSection;
NtUnMapViewOfSection_t* NtUnMapViewOfSection;
NtUnlockVirtualMemory_t* NtUnlockVirtualMemory;
NtLockVirtualMemory_t* NtLockVirtualMemory;
NtSuspendProcess_t* NtSuspendProcess;
NtResumeProcess_t* NtResumeProcess;
NtReadVirtualMemory_t* NtReadVirtualMemory;
NtWriteVirtualMemory_t* NtWriteVirtualMemory;

void NTDLL_INIT_FCNS(HMODULE ntdll) {
	NtQuerySystemInformation = (NtQuerySystemInformation_t *)GetProcAddress(ntdll,"NtQuerySystemInformation");
	NtDuplicateObject = (NtDuplicateObject_t*)GetProcAddress(ntdll, "NtDuplicateObject");
	NtQueryObject = (NtQueryObject_t*)GetProcAddress(ntdll, "NtQueryObject");
	NtQueryInformationThread = (NtQueryInformationThread_t*)GetProcAddress(ntdll, "NtQueryInformationThread");
	NtQueryInformationProcess = (NtQueryInformationProcess_t*)GetProcAddress(ntdll, "NtQueryInformationProcess");
	NtOpenSection = (NtOpenSection_t*)GetProcAddress(ntdll, "NtOpenSection");
	NtQuerySection = (NtQuerySection_t*)GetProcAddress(ntdll, "NtQuerySection");
	NtCreateSection = (NtCreateSection_t*)GetProcAddress(ntdll, "NtCreateSection");
	NtMapViewOfSection = (NtMapViewOfSection_t*)GetProcAddress(ntdll, "NtMapViewOfSection");
	NtUnMapViewOfSection = (NtUnMapViewOfSection_t*)GetProcAddress(ntdll, "NtUnMapViewOfSection");
	NtUnlockVirtualMemory = (NtUnlockVirtualMemory_t*)GetProcAddress(ntdll, "NtUnlockVirtualMemory");
	NtLockVirtualMemory = (NtLockVirtualMemory_t*)GetProcAddress(ntdll, "NtLockVirtualMemory");
	NtSuspendProcess = (NtSuspendProcess_t*)GetProcAddress(ntdll, "NtSuspendProcess");
	NtResumeProcess = (NtResumeProcess_t*)GetProcAddress(ntdll, "NtResumeProcess");
	NtReadVirtualMemory = (NtReadVirtualMemory_t*)GetProcAddress(ntdll, "NtReadVirtualMemory");
	NtWriteVirtualMemory = (NtWriteVirtualMemory_t*)GetProcAddress(ntdll, "NtWriteVirtualMemory");
}