#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "prompt_path.h"

typedef void *(__attribute__((thiscall)) *Lookup)(void *,const char *);
static Lookup original_lookup;
extern BOOL controller_uses_playstation_prompts(void);
const char *prompt_hook_status="Prompt texture hook not installed (diagnostic or unsupported game).";
volatile LONG prompt_redirect_mask;
volatile LONG prompt_missing_mask;
static void *__attribute__((fastcall)) lookup(void *manager,void *unused,const char *path) {
    (void)unused;
    char replacement[256];int button=prompt_replacement(path,replacement);
    if(button && controller_uses_playstation_prompts()) {
        void *entry=original_lookup(manager,replacement);
        if(entry) { InterlockedOr(&prompt_redirect_mask,1L<<(button-1));return entry; }
        InterlockedOr(&prompt_missing_mask,1L<<(button-1));
    }
    return original_lookup(manager,path);
}
void install_prompt_hook(BYTE *base,DWORD image_size,DWORD timestamp) {
    DWORD rva=0;
    if(timestamp==0x5245f25a && image_size==0x3159000) rva=0x5ab20;
    if(timestamp==0x5245f416 && image_size==0x30dd000) rva=0x309040;
    if(!rva) return;
    BYTE *target=base+rva;
    const BYTE first[]={0x55,0x8b,0xec,0x6a,0xff,0x68};
    const BYTE middle[]={0x64,0xa1,0,0,0,0,0x50,0x83,0xec,0x1c,0x53,0x56,0x57,0xa1};
    const BYTE last[]={0x33,0xc5,0x50,0x8d,0x45,0xf4,0x64,0xa3,0,0,0,0,0x8b,0xf1,0x8b,0x45,0x08};
    if(memcmp(target,first,sizeof(first)) || memcmp(target+10,middle,sizeof(middle)) || memcmp(target+28,last,sizeof(last))) {
        prompt_hook_status="Prompt texture hook refused: archive lookup signature mismatch.";return;
    }
    /* Ten whole bytes: push ebp; mov ebp,esp; push -1; push SEH handler.
     * The absolute SEH handler is already relocated by the Windows loader. */
    BYTE *trampoline=VirtualAlloc(NULL,15,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(!trampoline) return;
    memcpy(trampoline,target,10);trampoline[10]=0xe9;
    DWORD jump=(DWORD)((target+10)-(trampoline+15));memcpy(trampoline+11,&jump,4);
    DWORD old;
    if(!VirtualProtect(trampoline,15,PAGE_EXECUTE_READ,&old)) { VirtualFree(trampoline,0,MEM_RELEASE);return; }
    if(!VirtualProtect(target,10,PAGE_EXECUTE_READWRITE,&old)) { VirtualFree(trampoline,0,MEM_RELEASE);return; }
    original_lookup=(Lookup)trampoline;
    BYTE patch[10]={0xe9,0,0,0,0,0x90,0x90,0x90,0x90,0x90};
    jump=(DWORD)((BYTE*)&lookup-(target+5));memcpy(patch+1,&jump,4);memcpy(target,patch,10);
    DWORD unused;VirtualProtect(target,10,old,&unused);
    FlushInstructionCache(GetCurrentProcess(),target,10);FlushInstructionCache(GetCurrentProcess(),trampoline,15);
    prompt_hook_status="Automatic controller prompt hook installed; DualSense uses PS3 artwork, native XInput keeps Xbox artwork.";
}
