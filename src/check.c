#define WIN32_LEAN_AND_MEAN
#include "mapping.h"
#include "prompt_path.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do { if(!(x)) { printf("FAIL line %d: %s\n",__LINE__,#x); return 1; } ++checks; } while(0)
int main(int argc,char **argv) {
    int checks=0;
    char prompt[256];
    CHECK(prompt_replacement(NULL,prompt)==0);
    CHECK(prompt_replacement("ICON_IK_JOY1.dds",prompt)==0);
    CHECK(prompt_replacement("..\\..\\data\\flash\\textures\\SC6IconsPC\\ICON_IK_JOY1.dds",prompt)==1);
    CHECK(!strcmp(prompt,"..\\..\\data\\flash\\textures\\SC6IconsPS3\\ICON_IK_JOY1.dds"));
    CHECK(prompt_replacement("flash/pc_textures/SMIPause/icon_ik_joy16.dds",prompt)==16);
    CHECK(prompt_replacement("flash\\textures\\SC6IconsPS3\\ICON_IK_JOY1.dds",prompt)==0);
    CHECK(prompt_replacement("flash\\textures\\SC6IconsPC\\ICON_IK_JOY0.dds",prompt)==0);
    CHECK(prompt_replacement("flash\\textures\\SC6IconsPC\\ICON_IK_JOY17.dds",prompt)==0);
    CHECK(prompt_replacement("flash\\textures\\SC6IconsPC\\ICON_IK_JOY1.dds.bak",prompt)==0);
    CHECK(prompt_replacement("flash\\textures\\SC6IconsPC\\ICON_IK_KEY_A.dds",prompt)==0);
    bool buttons[SDL_GAMEPAD_BUTTON_COUNT]={0}; Sint16 axes[6]={0};
    CHECK(sizeof(XINPUT_STATE)==16); CHECK(sizeof(XINPUT_GAMEPAD)==12);
    CHECK(flip_y(-32768)==32767); CHECK(flip_y(32767)==-32768);
    CHECK(trigger_byte(0)==0); CHECK(trigger_byte(-32768)==0); CHECK(trigger_byte(32767)==255);
    for(int v=0;v<32767;v++) CHECK(trigger_byte(v)<=trigger_byte(v+1));
    axes[0]=8192; axes[1]=-16384; axes[2]=-24576; axes[3]=32767; axes[4]=16384; axes[5]=32767;
    buttons[SDL_GAMEPAD_BUTTON_SOUTH]=true; buttons[SDL_GAMEPAD_BUTTON_START]=true; buttons[SDL_GAMEPAD_BUTTON_DPAD_UP]=true;
    XINPUT_GAMEPAD g=translate(axes,buttons);
    CHECK(g.sThumbLX==8192 && g.sThumbLY==16383 && g.sThumbRX==-24576 && g.sThumbRY==-32768);
    CHECK(g.bLeftTrigger==128 && g.bRightTrigger==255);
    CHECK(g.wButtons==(XINPUT_GAMEPAD_A|XINPUT_GAMEPAD_START|XINPUT_GAMEPAD_DPAD_UP));
    const WORD expected[]={0x1000,0x2000,0x4000,0x8000,0x20,0,0x10,0x40,0x80,0x100,0x200,1,2,4,8};
    for(int b=0;b<15;b++) { memset(buttons,0,sizeof(buttons));buttons[b]=true;g=translate(axes,buttons);CHECK(g.wButtons==expected[b]); }
    HMODULE dll=argc>2?LoadLibraryA(argv[2]):LoadLibraryW(L"xinput1_3.dll"); if(!dll) {printf("DLL load failed: %lu\n",GetLastError()); return 1;}
    char loaded[MAX_PATH];GetModuleFileNameA(dll,loaded,MAX_PATH);printf("Loaded: %s\n",loaded);
    typedef DWORD (WINAPI *Get)(DWORD,XINPUT_STATE*);
    typedef DWORD (WINAPI *Caps)(DWORD,DWORD,XINPUT_CAPABILITIES*);
    typedef DWORD (WINAPI *Set)(DWORD,XINPUT_VIBRATION*);
    typedef void (WINAPI *Enable)(BOOL);
    Get get=(Get)GetProcAddress(dll,"XInputGetState");
    Caps caps=(Caps)GetProcAddress(dll,"XInputGetCapabilities");
    Set set=(Set)GetProcAddress(dll,"XInputSetState"); Enable enable=(Enable)GetProcAddress(dll,"XInputEnable");
    CHECK(get && caps && set && enable);
    CHECK((FARPROC)get==GetProcAddress(dll,(LPCSTR)2)); CHECK((FARPROC)set==GetProcAddress(dll,(LPCSTR)3)); CHECK((FARPROC)caps==GetProcAddress(dll,(LPCSTR)4));
    XINPUT_STATE state={0}; XINPUT_CAPABILITIES c={0};
    CHECK(get(4,&state)==ERROR_BAD_ARGUMENTS); CHECK(get(0,NULL)==ERROR_BAD_ARGUMENTS);
    CHECK(caps(0,2,&c)==ERROR_BAD_ARGUMENTS); CHECK(set(0,NULL)==ERROR_BAD_ARGUMENTS);
    DWORD result=get(0,&state);
    printf("Automated ABI/mapping checks passed: %d\nSlot 0 status=%lu (0=connected,1167=disconnected)\n",checks,result);
    if(!result) {
        CHECK(caps(0,0,&c)==0); CHECK(c.Type==XINPUT_DEVTYPE_GAMEPAD);
        printf("Capabilities type=%u subtype=%u flags=%u\n",c.Type,c.SubType,c.Flags);
        enable(FALSE); CHECK(get(0,&state)==0);
        XINPUT_GAMEPAD zero={0}; CHECK(memcmp(&state.Gamepad,&zero,sizeof(zero))==0); enable(TRUE);
    }
    wchar_t system[MAX_PATH]; GetSystemDirectoryW(system,MAX_PATH); wcscat(system,L"\\xinput1_3.dll"); HMODULE sys=LoadLibraryW(system);
    Get original=sys?(Get)GetProcAddress(sys,"XInputGetState"):NULL;
    if(original) for(DWORD i=1;i<4;i++) { XINPUT_STATE a,b; CHECK(get(i,&a)==original(i,&b)); }
    fflush(stdout);
    int seconds=argc>1?atoi(argv[1]):2; DWORD prev=0; BOOL first=TRUE; ULONGLONG end=GetTickCount64()+(ULONGLONG)seconds*1000;
    while(GetTickCount64()<end) {
        DWORD r=get(0,&state);
        if(!r && (first || state.dwPacketNumber!=prev)) {
            printf("packet=%lu buttons=%04x LT=%u RT=%u LX=%d LY=%d RX=%d RY=%d\n",state.dwPacketNumber,state.Gamepad.wButtons,state.Gamepad.bLeftTrigger,state.Gamepad.bRightTrigger,state.Gamepad.sThumbLX,state.Gamepad.sThumbLY,state.Gamepad.sThumbRX,state.Gamepad.sThumbRY);
            prev=state.dwPacketNumber;first=FALSE;fflush(stdout);
        }
        Sleep(100);
    }
    printf("PASS (%d checks). This is a diagnostic process, not an in-game test.\n",checks);
    return 0;
}
