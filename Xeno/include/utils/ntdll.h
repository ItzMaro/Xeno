#pragma once

// I have no clue who the original owner of this file is

#include <Windows.h>

#define NTSTATUS DWORD
enum THREADINFOCLASS {
	ThreadBasicInformation = 0,
	ThreadTimes = 1,
	ThreadPriority = 2,
	ThreadBasePriority = 3,
	ThreadAffinityMask = 4,
	ThreadImpersonationToken = 5,
	ThreadDescriptorTableEntry = 6,
	ThreadEnableAlignmentFaultFixup = 7,
	ThreadEventPair_Reusable = 8,
	ThreadQuerySetWin32StartAddress = 9,
	ThreadZeroTlsCell = 10,
	ThreadPerformanceCount = 11,
	ThreadAmILastThread = 12,
	ThreadIdealProcessor = 13,
	ThreadPriorityBoost = 14,
	ThreadSetTlsArrayAddress = 15,
	ThreadIsIoPending = 16,
	ThreadHideFromDebugger = 17,
	ThreadBreakOnTermination = 18,
	ThreadSwitchLegacyState = 19,
	ThreadIsTerminated = 20,
	ThreadLastSystemCall = 21,
	ThreadIoPriority = 22,
	ThreadCycleTime = 23,
	ThreadPagePriority = 24,
	ThreadActualBasePriority = 25,
	ThreadTebInformation = 26,
	ThreadCSwitchMon = 27,
	ThreadCSwitchPmu = 28,
	ThreadWow64Context = 29,
	ThreadGroupInformation = 30,
	ThreadUmsInformation = 31,
	ThreadCounterProfiling = 32,
	ThreadIdealProcessorEx = 33,
	ThreadCpuAccountingInformation = 34,
	ThreadSuspendCount = 35,
	ThreadHeterogeneousCpuPolicy = 36,
	ThreadContainerId = 37,
	ThreadNameInformation = 38,
	ThreadSelectedCpuSets = 39,
	ThreadSystemThreadInformation = 40,
	ThreadActualGroupAffinity = 41,
	ThreadDynamicCodePolicyInfo = 42,
	ThreadExplicitCaseSensitivity = 43,
	ThreadWorkOnBehalfTicket = 44,
	ThreadSubsystemInformation = 45,
	ThreadDbgkWerReportActive = 46,
	ThreadAttachContainer = 47,
	MaxThreadInfoClass = 48
};
typedef enum _SYSTEM_INFORMATION_CLASS
{
    SystemBasicInformation = 0,
    SystemProcessorInformation = 1,             // obsolete...delete
    SystemPerformanceInformation = 2,
    SystemTimeOfDayInformation = 3,
    SystemPathInformation = 4,
    SystemProcessInformation = 5,
    SystemCallCountInformation = 6,
    SystemDeviceInformation = 7,
    SystemProcessorPerformanceInformation = 8,
    SystemFlagsInformation = 9,
    SystemCallTimeInformation = 10,
    SystemModuleInformation = 11,
    SystemLocksInformation = 12,
    SystemStackTraceInformation = 13,
    SystemPagedPoolInformation = 14,
    SystemNonPagedPoolInformation = 15,
    SystemHandleInformation = 16,
    SystemObjectInformation = 17,
    SystemPageFileInformation = 18,
    SystemVdmInstemulInformation = 19,
    SystemVdmBopInformation = 20,
    SystemFileCacheInformation = 21,
    SystemPoolTagInformation = 22,
    SystemInterruptInformation = 23,
    SystemDpcBehaviorInformation = 24,
    SystemFullMemoryInformation = 25,
    SystemLoadGdiDriverInformation = 26,
    SystemUnloadGdiDriverInformation = 27,
    SystemTimeAdjustmentInformation = 28,
    SystemSummaryMemoryInformation = 29,
    SystemMirrorMemoryInformation = 30,
    SystemPerformanceTraceInformation = 31,
    SystemObsolete0 = 32,
    SystemExceptionInformation = 33,
    SystemCrashDumpStateInformation = 34,
    SystemKernelDebuggerInformation = 35,
    SystemContextSwitchInformation = 36,
    SystemRegistryQuotaInformation = 37,
    SystemExtendServiceTableInformation = 38,
    SystemPrioritySeperation = 39,
    SystemVerifierAddDriverInformation = 40,
    SystemVerifierRemoveDriverInformation = 41,
    SystemProcessorIdleInformation = 42,
    SystemLegacyDriverInformation = 43,
    SystemCurrentTimeZoneInformation = 44,
    SystemLookasideInformation = 45,
    SystemTimeSlipNotification = 46,
    SystemSessionCreate = 47,
    SystemSessionDetach = 48,
    SystemSessionInformation = 49,
    SystemRangeStartInformation = 50,
    SystemVerifierInformation = 51,
    SystemVerifierThunkExtend = 52,
    SystemSessionProcessInformation = 53,
    SystemLoadGdiDriverInSystemSpace = 54,
    SystemNumaProcessorMap = 55,
    SystemPrefetcherInformation = 56,
    SystemExtendedProcessInformation = 57,
    SystemRecommendedSharedDataAlignment = 58,
    SystemComPlusPackage = 59,
    SystemNumaAvailableMemory = 60,
    SystemProcessorPowerInformation = 61,
    SystemEmulationBasicInformation = 62,
    SystemEmulationProcessorInformation = 63,
    SystemExtendedHandleInformation = 64,
    SystemLostDelayedWriteInformation = 65,
    SystemBigPoolInformation = 66,
    SystemSessionPoolTagInformation = 67,
    SystemSessionMappedViewInformation = 68,
    SystemHotpatchInformation = 69,
    SystemObjectSecurityMode = 70,
    SystemWatchdogTimerHandler = 71,
    SystemWatchdogTimerInformation = 72,
    SystemLogicalProcessorInformation = 73,
    SystemWow64SharedInformation = 74,
    SystemRegisterFirmwareTableInformationHandler = 75,
    SystemFirmwareTableInformation = 76,
    SystemModuleInformationEx = 77,
    SystemVerifierTriageInformation = 78,
    SystemSuperfetchInformation = 79,
    SystemMemoryListInformation = 80,
    SystemFileCacheInformationEx = 81,
    MaxSystemInfoClass = 82  // MaxSystemInfoClass should always be the last enum

} SYSTEM_INFORMATION_CLASS;
typedef enum _PROCESSINFOCLASS
{
    ProcessBasicInformation, // q: PROCESS_BASIC_INFORMATION, PROCESS_EXTENDED_BASIC_INFORMATION
    ProcessQuotaLimits, // qs: QUOTA_LIMITS, QUOTA_LIMITS_EX
    ProcessIoCounters, // q: IO_COUNTERS
    ProcessVmCounters, // q: VM_COUNTERS, VM_COUNTERS_EX, VM_COUNTERS_EX2
    ProcessTimes, // q: KERNEL_USER_TIMES
    ProcessBasePriority, // s: KPRIORITY
    ProcessRaisePriority, // s: ULONG
    ProcessDebugPort, // q: HANDLE
    ProcessExceptionPort, // s: PROCESS_EXCEPTION_PORT (requires SeTcbPrivilege)
    ProcessAccessToken, // s: PROCESS_ACCESS_TOKEN
    ProcessLdtInformation, // qs: PROCESS_LDT_INFORMATION // 10
    ProcessLdtSize, // s: PROCESS_LDT_SIZE
    ProcessDefaultHardErrorMode, // qs: ULONG
    ProcessIoPortHandlers, // (kernel-mode only) // s: PROCESS_IO_PORT_HANDLER_INFORMATION
    ProcessPooledUsageAndLimits, // q: POOLED_USAGE_AND_LIMITS
    ProcessWorkingSetWatch, // q: PROCESS_WS_WATCH_INFORMATION[]; s: void
    ProcessUserModeIOPL, // qs: ULONG (requires SeTcbPrivilege)
    ProcessEnableAlignmentFaultFixup, // s: BOOLEAN
    ProcessPriorityClass, // qs: PROCESS_PRIORITY_CLASS
    ProcessWx86Information, // qs: ULONG (requires SeTcbPrivilege) (VdmAllowed)
    ProcessHandleCount, // q: ULONG, PROCESS_HANDLE_INFORMATION // 20
    ProcessAffinityMask, // (q >WIN7)s: KAFFINITY, qs: GROUP_AFFINITY
    ProcessPriorityBoost, // qs: ULONG
    ProcessDeviceMap, // qs: PROCESS_DEVICEMAP_INFORMATION, PROCESS_DEVICEMAP_INFORMATION_EX
    ProcessSessionInformation, // q: PROCESS_SESSION_INFORMATION
    ProcessForegroundInformation, // s: PROCESS_FOREGROUND_BACKGROUND
    ProcessWow64Information, // q: ULONG_PTR
    ProcessImageFileName, // q: UNICODE_STRING
    ProcessLUIDDeviceMapsEnabled, // q: ULONG
    ProcessBreakOnTermination, // qs: ULONG
    ProcessDebugObjectHandle, // q: HANDLE // 30
    ProcessDebugFlags, // qs: ULONG
    ProcessHandleTracing, // q: PROCESS_HANDLE_TRACING_QUERY; s: PROCESS_HANDLE_TRACING_ENABLE[_EX] or void to disable
    ProcessIoPriority, // qs: IO_PRIORITY_HINT
    ProcessExecuteFlags, // qs: ULONG (MEM_EXECUTE_OPTION_*)
    ProcessTlsInformation, // PROCESS_TLS_INFORMATION // ProcessResourceManagement
    ProcessCookie, // q: ULONG
    ProcessImageInformation, // q: SECTION_IMAGE_INFORMATION
    ProcessCycleTime, // q: PROCESS_CYCLE_TIME_INFORMATION // since VISTA
    ProcessPagePriority, // qs: PAGE_PRIORITY_INFORMATION
    ProcessInstrumentationCallback, // s: PVOID or PROCESS_INSTRUMENTATION_CALLBACK_INFORMATION // 40
    ProcessThreadStackAllocation, // s: PROCESS_STACK_ALLOCATION_INFORMATION, PROCESS_STACK_ALLOCATION_INFORMATION_EX
    ProcessWorkingSetWatchEx, // q: PROCESS_WS_WATCH_INFORMATION_EX[]; s: void
    ProcessImageFileNameWin32, // q: UNICODE_STRING
    ProcessImageFileMapping, // q: HANDLE (input)
    ProcessAffinityUpdateMode, // qs: PROCESS_AFFINITY_UPDATE_MODE
    ProcessMemoryAllocationMode, // qs: PROCESS_MEMORY_ALLOCATION_MODE
    ProcessGroupInformation, // q: USHORT[]
    ProcessTokenVirtualizationEnabled, // s: ULONG
    ProcessConsoleHostProcess, // qs: ULONG_PTR // ProcessOwnerInformation
    ProcessWindowInformation, // q: PROCESS_WINDOW_INFORMATION // 50
    ProcessHandleInformation, // q: PROCESS_HANDLE_SNAPSHOT_INFORMATION // since WIN8
    ProcessMitigationPolicy, // s: PROCESS_MITIGATION_POLICY_INFORMATION
    ProcessDynamicFunctionTableInformation, // s: PROCESS_DYNAMIC_FUNCTION_TABLE_INFORMATION
    ProcessHandleCheckingMode, // qs: ULONG; s: 0 disables, otherwise enables
    ProcessKeepAliveCount, // q: PROCESS_KEEPALIVE_COUNT_INFORMATION
    ProcessRevokeFileHandles, // s: PROCESS_REVOKE_FILE_HANDLES_INFORMATION
    ProcessWorkingSetControl, // s: PROCESS_WORKING_SET_CONTROL (requires SeDebugPrivilege)
    ProcessHandleTable, // q: ULONG[] // since WINBLUE
    ProcessCheckStackExtentsMode, // qs: ULONG // KPROCESS->CheckStackExtents (CFG)
    ProcessCommandLineInformation, // q: UNICODE_STRING // 60
    ProcessProtectionInformation, // q: PS_PROTECTION
    ProcessMemoryExhaustion, // s: PROCESS_MEMORY_EXHAUSTION_INFO // since THRESHOLD
    ProcessFaultInformation, // s: PROCESS_FAULT_INFORMATION
    ProcessTelemetryIdInformation, // q: PROCESS_TELEMETRY_ID_INFORMATION
    ProcessCommitReleaseInformation, // qs: PROCESS_COMMIT_RELEASE_INFORMATION
    ProcessDefaultCpuSetsInformation, // qs: SYSTEM_CPU_SET_INFORMATION[5]
    ProcessAllowedCpuSetsInformation, // qs: SYSTEM_CPU_SET_INFORMATION[5]
    ProcessSubsystemProcess,
    ProcessJobMemoryInformation, // q: PROCESS_JOB_MEMORY_INFO
    ProcessInPrivate, // q: BOOLEAN; s: void // ETW // since THRESHOLD2 // 70
    ProcessRaiseUMExceptionOnInvalidHandleClose, // qs: ULONG; s: 0 disables, otherwise enables
    ProcessIumChallengeResponse,
    ProcessChildProcessInformation, // q: PROCESS_CHILD_PROCESS_INFORMATION
    ProcessHighGraphicsPriorityInformation, // qs: BOOLEAN (requires SeTcbPrivilege)
    ProcessSubsystemInformation, // q: SUBSYSTEM_INFORMATION_TYPE // since REDSTONE2
    ProcessEnergyValues, // q: PROCESS_ENERGY_VALUES, PROCESS_EXTENDED_ENERGY_VALUES
    ProcessPowerThrottlingState, // qs: POWER_THROTTLING_PROCESS_STATE
    ProcessReserved3Information, // ProcessActivityThrottlePolicy // PROCESS_ACTIVITY_THROTTLE_POLICY
    ProcessWin32kSyscallFilterInformation, // q: WIN32K_SYSCALL_FILTER
    ProcessDisableSystemAllowedCpuSets, // s: BOOLEAN // 80
    ProcessWakeInformation, // q: PROCESS_WAKE_INFORMATION
    ProcessEnergyTrackingState, // qs: PROCESS_ENERGY_TRACKING_STATE
    ProcessManageWritesToExecutableMemory, // MANAGE_WRITES_TO_EXECUTABLE_MEMORY // since REDSTONE3
    ProcessCaptureTrustletLiveDump,
    ProcessTelemetryCoverage, // q: TELEMETRY_COVERAGE_HEADER; s: TELEMETRY_COVERAGE_POINT
    ProcessEnclaveInformation,
    ProcessEnableReadWriteVmLogging, // qs: PROCESS_READWRITEVM_LOGGING_INFORMATION
    ProcessUptimeInformation, // q: PROCESS_UPTIME_INFORMATION
    ProcessImageSection, // q: HANDLE
    ProcessDebugAuthInformation, // since REDSTONE4 // 90
    ProcessSystemResourceManagement, // s: PROCESS_SYSTEM_RESOURCE_MANAGEMENT
    ProcessSequenceNumber, // q: ULONGLONG
    ProcessLoaderDetour, // since REDSTONE5
    ProcessSecurityDomainInformation, // q: PROCESS_SECURITY_DOMAIN_INFORMATION
    ProcessCombineSecurityDomainsInformation, // s: PROCESS_COMBINE_SECURITY_DOMAINS_INFORMATION
    ProcessEnableLogging, // qs: PROCESS_LOGGING_INFORMATION
    ProcessLeapSecondInformation, // qs: PROCESS_LEAP_SECOND_INFORMATION
    ProcessFiberShadowStackAllocation, // s: PROCESS_FIBER_SHADOW_STACK_ALLOCATION_INFORMATION // since 19H1
    ProcessFreeFiberShadowStackAllocation, // s: PROCESS_FREE_FIBER_SHADOW_STACK_ALLOCATION_INFORMATION
    ProcessAltSystemCallInformation, // s: PROCESS_SYSCALL_PROVIDER_INFORMATION // since 20H1 // 100
    ProcessDynamicEHContinuationTargets, // s: PROCESS_DYNAMIC_EH_CONTINUATION_TARGETS_INFORMATION
    ProcessDynamicEnforcedCetCompatibleRanges, // s: PROCESS_DYNAMIC_ENFORCED_ADDRESS_RANGE_INFORMATION // since 20H2
    ProcessCreateStateChange, // since WIN11
    ProcessApplyStateChange,
    ProcessEnableOptionalXStateFeatures, // s: ULONG64 // optional XState feature bitmask
    ProcessAltPrefetchParam, // since 22H1
    ProcessAssignCpuPartitions,
    ProcessPriorityClassEx, // s: PROCESS_PRIORITY_CLASS_EX
    ProcessMembershipInformation, // q: PROCESS_MEMBERSHIP_INFORMATION
    ProcessEffectiveIoPriority, // q: IO_PRIORITY_HINT
    ProcessEffectivePagePriority, // q: ULONG
    MaxProcessInfoClass
} PROCESSINFOCLASS;

/*typedef struct _SECURITY_DESCRIPTOR {
    BYTE                        Revision;
    BYTE                        Sbz1;
    SECURITY_DESCRIPTOR_CONTROL Control;
    PSID                        Owner;
    PSID                        Group;
    PACL                        Sacl;
    PACL                        Dacl;
} SECURITY_DESCRIPTOR, * PISECURITY_DESCRIPTOR;
typedef struct _SECURITY_ATTRIBUTES {
    DWORD  nLength;
    LPVOID lpSecurityDescriptor;
    BOOL   bInheritHandle;
} SECURITY_ATTRIBUTES, * PSECURITY_ATTRIBUTES, * LPSECURITY_ATTRIBUTES;
*/
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, * PUNICODE_STRING;

typedef enum _OBJECT_INFORMATION_CLASS
{
    ObjectTypeInformation = 2,
    ObjectTypesInformation = 3
} OBJECT_INFORMATION_CLASS, * POBJECT_INFORMATION_CLASS;
typedef struct _SYSTEM_HANDLE
{
	ULONG ProcessId;
	BYTE ObjectTypeNumber;
	BYTE Flags;
	USHORT Handle;
	PVOID Object;
	ACCESS_MASK GrantedAccess;
} SYSTEM_HANDLE, * PSYSTEM_HANDLE;
typedef struct _SYSTEM_HANDLE_INFORMATION
{
	ULONG HandleCount;
	SYSTEM_HANDLE Handles[1];
} SYSTEM_HANDLE_INFORMATION, * PSYSTEM_HANDLE_INFORMATION;

typedef struct _OBJECT_TYPE_INFORMATION
{
    UNICODE_STRING TypeName;
    ULONG TotalNumberOfHandles;
    ULONG TotalNumberOfObjects;
} OBJECT_TYPE_INFORMATION, * POBJECT_TYPE_INFORMATION;

typedef struct _OBJECT_ALL_INFORMATION
{
    ULONG NumberOfObjects;
    OBJECT_TYPE_INFORMATION ObjectTypeInformation[1];
} OBJECT_ALL_INFORMATION, * POBJECT_ALL_INFORMATION;

typedef struct _OBJECT_ATTRIBUTES {
    ULONG           Length;
    HANDLE          RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG           Attributes;
    PVOID           SecurityDescriptor;
    PVOID           SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;
typedef enum _SECTION_INFORMATION_CLASS {
    SectionBasicInformation,
    SectionImageInformation
} SECTION_INFORMATION_CLASS, * PSECTION_INFORMATION_CLASS;
typedef struct _SECTION_BASIC_INFORMATION {
    ULONG                   Unknown;
    ULONG                   SectionAttributes;
    LARGE_INTEGER           SectionSize;
} SECTION_BASIC_INFORMATION, * PSECTION_BASIC_INFORMATION;
typedef struct _SECTION_IMAGE_INFORMATION {
    PVOID                   EntryPoint;
    ULONG                   StackZeroBits;
    ULONG                   StackReserved;
    ULONG                   StackCommit;    
    ULONG                   ImageSubsystem;
    WORD                    SubSystemVersionLow;
    WORD                    SubSystemVersionHigh;
    ULONG                   Unknown1;
    ULONG                   ImageCharacteristics;
    ULONG                   ImageMachineType;
    ULONG                   Unknown2[3];
} SECTION_IMAGE_INFORMATION, * PSECTION_IMAGE_INFORMATION;
typedef enum _SECTION_INHERIT {
    ViewShare = 1,
    ViewUnmap = 2
} SECTION_INHERIT, * PSECTION_INHERIT;
typedef struct _SYSTEM_MODULE { PVOID  Reserved1;    PVOID  Reserved2;     PVOID  ImageBase;     ULONG  ImageSize;     ULONG  Flags;     USHORT Index;     USHORT NameLength;     USHORT LoadCount;     USHORT PathLength;     CHAR   ImageName[256]; }  SYSTEM_MODULE, * PSYSTEM_MODULE;
typedef struct _SYSTEM_MODULE_INFORMATION { ULONG                ModulesCount;   SYSTEM_MODULE        Modules[0]; } SYSTEM_MODULE_INFORMATION, * PSYSTEM_MODULE_INFORMATION;
typedef LONG KPRIORITY;
struct CLIENT_ID
{
    DWORD UniqueProcess; // Process ID
#ifdef _WIN64
    ULONG pad1;
#endif
    DWORD UniqueThread;  // Thread ID
#ifdef _WIN64
    ULONG pad2;
#endif
};
typedef struct {
    FILETIME ProcessorTime;    FILETIME UserTime;    FILETIME CreateTime;    ULONG WaitTime;
#ifdef _WIN64
    ULONG pad1;
#endif
    PVOID StartAddress;    CLIENT_ID Client_Id;    KPRIORITY CurrentPriority;    KPRIORITY BasePriority;    ULONG ContextSwitchesPerSec;    ULONG ThreadState;    ULONG ThreadWaitReason;    ULONG pad2;
} SYSTEM_THREAD_INFORMATION;
typedef struct {
    ULONG_PTR PeakVirtualSize;    ULONG_PTR VirtualSize;    ULONG PageFaultCount;
#ifdef _WIN64
    ULONG pad1;
#endif
    ULONG_PTR PeakWorkingSetSize;    ULONG_PTR WorkingSetSize;    ULONG_PTR QuotaPeakPagedPoolUsage;    ULONG_PTR QuotaPagedPoolUsage;    ULONG_PTR QuotaPeakNonPagedPoolUsage;    ULONG_PTR QuotaNonPagedPoolUsage;    ULONG_PTR PagefileUsage;    ULONG_PTR PeakPagefileUsage;
} VM_COUNTERS;
typedef struct {
    ULONG NextOffset;    ULONG ThreadCount;    LARGE_INTEGER WorkingSetPrivateSize;    ULONG HardFaultCount;    ULONG NumberOfThreadsHighWatermark;    ULONGLONG CycleTime;    FILETIME CreateTime;    FILETIME UserTime;    FILETIME KernelTime;    UNICODE_STRING ImageName;    KPRIORITY BasePriority;
#ifdef _WIN64
    ULONG pad1;
#endif
    ULONG ProcessId;
#ifdef _WIN64
    ULONG pad2;
#endif
    ULONG InheritedFromProcessId;
#ifdef _WIN64
    ULONG pad3;
#endif
    ULONG HandleCount;    ULONG SessionId;    ULONG_PTR UniqueProcessKey;    VM_COUNTERS VirtualMemoryCounters;    ULONG_PTR PrivatePageCount;    IO_COUNTERS IoCounters;    SYSTEM_THREAD_INFORMATION ThreadInfos[1];
} SYSTEM_PROCESS_INFORMATION;
typedef NTSTATUS (NtQuerySystemInformation_t)(SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);
typedef NTSTATUS (NtDuplicateObject_t)(HANDLE SourceProcessHandle, HANDLE SourceHandle, HANDLE TargetProcessHandle, PHANDLE TargetHandle, ACCESS_MASK DesiredAccess, ULONG HandleAttributes, ULONG Options);
typedef NTSTATUS (NtQueryObject_t)(HANDLE Handle, OBJECT_INFORMATION_CLASS ObjectInformationClass, PVOID ObjectInformation, ULONG ObjectInformationLength, PULONG ReturnLength);
typedef NTSTATUS (NtQueryInformationThread_t)(HANDLE hThreadHandle, THREADINFOCLASS ThreadInformationClass, PVOID ThreadInformation, ULONG ThreadInformationLength, PULONG ReturnLength);
typedef NTSTATUS (NtQueryInformationProcess_t)(HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass,PVOID ProcessInformation,ULONG ProcessInformationLength,PULONG ReturnLength);
typedef NTSTATUS (NtOpenSection_t)(PHANDLE SectionHandle,ACCESS_MASK DesiredAccess,POBJECT_ATTRIBUTES   ObjectAttributes);
typedef NTSTATUS (NtQuerySection_t)(HANDLE SectionHandle,SECTION_INFORMATION_CLASS InformationClass,PVOID InformationBuffer,ULONG InformationBufferSize,PULONG ResultLength);
typedef NTSTATUS (NtCreateSection_t)(PHANDLE SectionHandle,ULONG DesiredAccess,POBJECT_ATTRIBUTES ObjectAttributes,PLARGE_INTEGER MaximumSize,ULONG PageAttributess,ULONG SectionAttributes,HANDLE FileHandle);
typedef NTSTATUS (NtMapViewOfSection_t)(HANDLE SectionHandle,HANDLE ProcessHandle,PVOID* BaseAddress,ULONG ZeroBits,ULONG CommitSize,PLARGE_INTEGER SectionOffset,PULONG ViewSize, SECTION_INHERIT InheritDisposition,ULONG AllocationType,ULONG Protect);
typedef NTSTATUS (NtUnMapViewOfSection_t)(HANDLE ProcessHandle,PVOID BaseAddress);
typedef NTSTATUS (NtUnlockVirtualMemory_t)(HANDLE ProcessHandle, PVOID* BaseAddress, PSIZE_T NumberOfBytesToUnlock, ULONG LockType);
typedef NTSTATUS (NtLockVirtualMemory_t)(HANDLE ProcessHandle, PVOID* BaseAddress, PSIZE_T NumberOfBytesToLock, ULONG LockOption);
typedef NTSTATUS (NtSuspendProcess_t)(HANDLE ProcessHandle);
typedef NTSTATUS (NtResumeProcess_t)(HANDLE ProcessHandle);
typedef NTSTATUS (NtReadVirtualMemory_t)(HANDLE ProcessHandle, LPCVOID BaseAddress, PVOID Buffer, ULONG NumberOfBytesToRead, PSIZE_T NumberOfBytesRead);
typedef NTSTATUS (NtWriteVirtualMemory_t)(HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, ULONG NumberOfBytesToWrite, PSIZE_T NumberOfBytesWritten);

extern NtQuerySystemInformation_t * NtQuerySystemInformation;
extern NtDuplicateObject_t* NtDuplicateObject;
extern NtQueryObject_t* NtQueryObject;
extern NtQueryInformationThread_t* NtQueryInformationThread;
extern NtQueryInformationProcess_t* NtQueryInformationProcess;
extern NtOpenSection_t* NtOpenSection;
extern NtQuerySection_t* NtQuerySection;
extern NtCreateSection_t* NtCreateSection;
extern NtMapViewOfSection_t* NtMapViewOfSection;
extern NtUnMapViewOfSection_t* NtUnMapViewOfSection;
extern NtUnlockVirtualMemory_t* NtUnlockVirtualMemory;
extern NtLockVirtualMemory_t* NtLockVirtualMemory;
extern NtSuspendProcess_t* NtSuspendProcess;
extern NtResumeProcess_t* NtResumeProcess;
extern NtReadVirtualMemory_t* NtReadVirtualMemory;
extern NtWriteVirtualMemory_t* NtWriteVirtualMemory;

void NTDLL_INIT_FCNS(HMODULE ntdll);