/* Blacklist enumerates DirectInput devices and WMI-classifies them BEFORE XInput.
 * Recognize only Sony DualSense product GUIDs; preserve the original classifier
 * for every other device. No disk patch. Refuse unknown executable revisions. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <wchar.h>
extern void controller_gate_notice(void);
typedef int (__cdecl *Classifier)(const GUID *);
static Classifier original_classifier;
typedef int (WINAPI *Enumerator)(const void *,void *);
static Enumerator original_enumerator;
static DWORD *device_count;
static BYTE *instance_guids,*product_guids;
static int WINAPI enumerate(const void *device,void *context) {
    const BYTE *bytes=(const BYTE*)device;
    DWORD product; memcpy(&product,bytes+20,4);
    DWORD n=*device_count;
    if((product==0x0ce6054c || product==0x0df2054c) && n>0 && n<4) {
        /* Blacklist pairs XInput slot 0 with the FIRST DirectInput device,
         * even if that device failed its XInput classifier. Keep the Sony
         * GUID pair first, retaining the keyboard/other devices afterward. */
        memmove(instance_guids+16,instance_guids,n*16);
        memmove(product_guids+16,product_guids,n*16);
        memcpy(instance_guids,bytes+4,16);memcpy(product_guids,bytes+20,16);
        *device_count=n+1;
        return n+1<4;
    }
    return original_enumerator(device,context);
}
static void *detour(BYTE *target,void *replacement) {
    BYTE *trampoline=VirtualAlloc(NULL,14,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(!trampoline) return NULL;
    memcpy(trampoline,target,9);trampoline[9]=0xe9;
    DWORD jump=(DWORD)((target+9)-(trampoline+14));memcpy(trampoline+10,&jump,4);
    DWORD old;
    if(!VirtualProtect(trampoline,14,PAGE_EXECUTE_READ,&old)) { VirtualFree(trampoline,0,MEM_RELEASE);return NULL; }
    if(!VirtualProtect(target,9,PAGE_EXECUTE_READWRITE,&old)) { VirtualFree(trampoline,0,MEM_RELEASE);return NULL; }
    BYTE patch[9]={0xe9,0,0,0,0,0x90,0x90,0x90,0x90};
    jump=(DWORD)((BYTE*)replacement-(target+5));memcpy(patch+1,&jump,4);memcpy(target,patch,9);
    DWORD unused;VirtualProtect(target,9,old,&unused);
    FlushInstructionCache(GetCurrentProcess(),target,9);FlushInstructionCache(GetCurrentProcess(),trampoline,14);
    return trampoline;
}
const char *detection_hook_status="No game detection hook (diagnostic or unsupported executable).";
static int __cdecl classify(const GUID *product) {
    if(product && (product->Data1==0x0ce6054c || product->Data1==0x0df2054c)) {
        controller_gate_notice();
        return 1;
    }
    return original_classifier(product);
}
BOOL WINAPI DllMain(HINSTANCE self,DWORD reason,LPVOID reserved) {
    (void)reserved;
    if(reason!=DLL_PROCESS_ATTACH) return TRUE;
    DisableThreadLibraryCalls(self);
    BYTE *base=(BYTE*)GetModuleHandleW(NULL);
    IMAGE_DOS_HEADER *dos=(IMAGE_DOS_HEADER*)base;
    if(!base || dos->e_magic!=IMAGE_DOS_SIGNATURE) return TRUE;
    IMAGE_NT_HEADERS32 *pe=(IMAGE_NT_HEADERS32*)(base+dos->e_lfanew);
    if(pe->Signature!=IMAGE_NT_SIGNATURE || pe->FileHeader.Machine!=IMAGE_FILE_MACHINE_I386) return TRUE;
    wchar_t path[MAX_PATH]; GetModuleFileNameW(NULL,path,MAX_PATH);
    wchar_t *name=wcsrchr(path,L'\\'); name=name?name+1:path;
    DWORD rva=0,enum_rva=0;
    if(!_wcsicmp(name,L"Blacklist_DX11_game.exe") && pe->FileHeader.TimeDateStamp==0x5245f25a && pe->OptionalHeader.SizeOfImage==0x3159000) {
        rva=0x16f1160;enum_rva=0x16f1460;device_count=(DWORD*)(base+0x304e98c);instance_guids=base+0x304e928;product_guids=base+0x304e8e8;
    }
    if(!_wcsicmp(name,L"Blacklist_game.exe") && pe->FileHeader.TimeDateStamp==0x5245f416 && pe->OptionalHeader.SizeOfImage==0x30dd000) {
        rva=0x16ca810;enum_rva=0x16cab10;device_count=(DWORD*)(base+0x2f5a14c);instance_guids=base+0x2f5a0b8;product_guids=base+0x2f5a0f8;
    }
    if(!rva) return TRUE;
    /* 40 bytes of classifier prologue; ignore only its relocated cookie address. */
    static const BYTE signature[]={0x55,0x8b,0xec,0x81,0xec,0xb8,0,0,0,0xa1,0,0,0,0,0x33,0xc5,0x89,0x45,0xfc,0x53,0x56,0x57,0x33,0xdb,0x6a,0x4c,0x8d,0x85,0x4c,0xff,0xff,0xff,0x53,0x50,0x89,0x5d,0xac,0x89,0x5d,0xb4};
    BYTE *target=base+rva;
    for(unsigned i=0;i<sizeof(signature);i++) if((i<10 || i>13) && target[i]!=signature[i]) {
        detection_hook_status="Detection hook refused: classifier signature mismatch."; return TRUE;
    }
    BYTE *enum_target=base+enum_rva;
    static const BYTE enum_head[]={0x55,0x8b,0xec,0x8b,0x15};
    static const BYTE enum_tail[]={0x83,0xfa,0x04,0x7d,0x42,0x8b,0x4d,0x08,0xf3,0x0f,0x7e,0x41,0x04,0x8b,0xc2,0xc1,0xe0,0x04};
    DWORD referenced_counter;memcpy(&referenced_counter,enum_target+5,4);
    if(memcmp(enum_target,enum_head,sizeof(enum_head)) || memcmp(enum_target+9,enum_tail,sizeof(enum_tail)) || referenced_counter!=(DWORD)device_count) {
        detection_hook_status="Detection hook refused: enumeration signature mismatch.";return TRUE;
    }
    original_classifier=(Classifier)detour(target,(void*)&classify);
    original_enumerator=(Enumerator)detour(enum_target,(void*)&enumerate);
    detection_hook_status=original_classifier && original_enumerator ?
        "Verified Blacklist identity and device-order hooks installed (DualSense only, process memory only)." : "ERROR: detection hook installation failed.";
    return TRUE;
}
