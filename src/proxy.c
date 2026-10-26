/* Local XInput 1.3 implementation for Blacklist. No input injection or binary patches. */
#define WIN32_LEAN_AND_MEAN
#include "mapping.h"
#include <stdio.h>

static INIT_ONCE once=INIT_ONCE_STATIC_INIT;
static SRWLOCK lock=SRWLOCK_INIT;
static HMODULE original;
static XINPUT_STATE cached;
static XINPUT_VIBRATION rumble;
static BOOL connected, enabled=TRUE, has_rumble;
static DWORD battery_type=BATTERY_TYPE_UNKNOWN,battery_level=BATTERY_LEVEL_FULL;
static HANDLE ready;
static char log_path[MAX_PATH];
extern const char *detection_hook_status;
extern const char *prompt_hook_status;
extern volatile LONG prompt_redirect_mask;
extern volatile LONG prompt_missing_mask;
static void log_line(const char *s) {
    FILE *f=fopen(log_path,"a");
    if(f) { fprintf(f,"[%lu pid=%lu] %s\n",GetTickCount(),GetCurrentProcessId(),s); fclose(f); }
}
static FARPROC native(const char *name) { return original ? GetProcAddress(original,name) : NULL; }
static DWORD WINAPI poll_controller(void *unused) {
    (void)unused;
    /* Avoid recursion into this proxy and duplicate Steam/virtual controller paths. */
    SDL_SetHintWithPriority(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,"1",SDL_HINT_OVERRIDE);
    SDL_SetHintWithPriority(SDL_HINT_JOYSTICK_THREAD,"1",SDL_HINT_OVERRIDE);
    SDL_SetHintWithPriority(SDL_HINT_JOYSTICK_HIDAPI,"1",SDL_HINT_OVERRIDE);
    SDL_SetHintWithPriority(SDL_HINT_JOYSTICK_HIDAPI_PS5,"1",SDL_HINT_OVERRIDE);
    SDL_SetHintWithPriority(SDL_HINT_JOYSTICK_ENHANCED_REPORTS,"1",SDL_HINT_OVERRIDE);
    SDL_SetHintWithPriority(SDL_HINT_XINPUT_ENABLED,"0",SDL_HINT_OVERRIDE);
    SDL_SetHintWithPriority(SDL_HINT_JOYSTICK_RAWINPUT,"0",SDL_HINT_OVERRIDE);
    if(!SDL_InitSubSystem(SDL_INIT_GAMEPAD)) {
        log_line(SDL_GetError()); SetEvent(ready); return 1;
    }
    log_line("SDL gamepad backend initialized; looking for Sony DualSense/Edge.");
    SDL_Gamepad *pad=NULL;
    ULONGLONG next_scan=0;
    XINPUT_VIBRATION last={0};
    BOOL previous_enabled=TRUE;
    LONG reported_prompts=0;
    for(;;) {
        LONG prompts=prompt_redirect_mask;
        if(prompts!=reported_prompts) {
            char message[160];snprintf(message,sizeof(message),"PlayStation prompt textures used: button mask %04lx (archive fallback mask %04lx).",(unsigned long)prompts,(unsigned long)prompt_missing_mask);
            log_line(message);reported_prompts=prompts;
        }
        SDL_UpdateGamepads();
        if(pad && !SDL_GamepadConnected(pad)) {
            AcquireSRWLockExclusive(&lock); connected=FALSE; ZeroMemory(&rumble,sizeof(rumble)); ZeroMemory(&cached.Gamepad,sizeof(cached.Gamepad)); ReleaseSRWLockExclusive(&lock);
            SDL_CloseGamepad(pad); pad=NULL; next_scan=0; log_line("DualSense disconnected.");
        }
        if(!pad && GetTickCount64()>=next_scan) {
            next_scan=GetTickCount64()+1000;
            int count=0; SDL_JoystickID *ids=SDL_GetGamepads(&count);
            for(int i=0;ids && i<count;i++) {
                if(SDL_GetGamepadVendorForID(ids[i])==0x054c &&
                   (SDL_GetGamepadProductForID(ids[i])==0x0ce6 || SDL_GetGamepadProductForID(ids[i])==0x0df2)) {
                    pad=SDL_OpenGamepad(ids[i]); if(pad) break;
                }
            }
            SDL_free(ids);
            if(pad) {
                char message[320]; snprintf(message,sizeof(message),"Connected: %s; VID=%04x PID=%04x; assigned XInput slot 0.",
                    SDL_GetGamepadName(pad),SDL_GetGamepadVendor(pad),SDL_GetGamepadProduct(pad)); log_line(message);
                BOOL r=SDL_GetBooleanProperty(SDL_GetGamepadProperties(pad),SDL_PROP_GAMEPAD_CAP_RUMBLE_BOOLEAN,FALSE);
                AcquireSRWLockExclusive(&lock); has_rumble=r; ReleaseSRWLockExclusive(&lock);
                ZeroMemory(&last,sizeof(last)); previous_enabled=FALSE;
            }
        }
        if(pad) {
            Sint16 axes[6]; bool buttons[SDL_GAMEPAD_BUTTON_COUNT];
            for(int i=0;i<6;i++) axes[i]=SDL_GetGamepadAxis(pad,(SDL_GamepadAxis)i);
            for(int i=0;i<SDL_GAMEPAD_BUTTON_COUNT;i++) buttons[i]=SDL_GetGamepadButton(pad,(SDL_GamepadButton)i);
            XINPUT_GAMEPAD g=translate(axes,buttons);
            int percent=-1; SDL_PowerState power=SDL_GetGamepadPowerInfo(pad,&percent);
            AcquireSRWLockExclusive(&lock);
            if(!connected || memcmp(&g,&cached.Gamepad,sizeof(g))) { cached.dwPacketNumber++; cached.Gamepad=g; }
            connected=TRUE;
            battery_type=(power==SDL_POWERSTATE_ON_BATTERY || power==SDL_POWERSTATE_CHARGING) ? BATTERY_TYPE_NIMH : BATTERY_TYPE_WIRED;
            battery_level=percent<0 ? BATTERY_LEVEL_FULL : percent<=5 ? BATTERY_LEVEL_EMPTY : percent<=20 ? BATTERY_LEVEL_LOW : percent<=70 ? BATTERY_LEVEL_MEDIUM : BATTERY_LEVEL_FULL;
            XINPUT_VIBRATION requested=rumble; BOOL active=enabled, supports_rumble=has_rumble;
            ReleaseSRWLockExclusive(&lock);
            if(!active) ZeroMemory(&requested,sizeof(requested));
            if(supports_rumble && (memcmp(&requested,&last,sizeof(last)) || active!=previous_enabled)) {
                SDL_RumbleGamepad(pad,requested.wLeftMotorSpeed,requested.wRightMotorSpeed,0xffffffffu);
                last=requested;
            }
            previous_enabled=active;
        }
        SetEvent(ready);
        Sleep(4);
    }
}
static BOOL CALLBACK initialize(PINIT_ONCE o,void *p,void **ctx) {
    (void)o;(void)p;(void)ctx;
    HMODULE self=NULL; wchar_t path[MAX_PATH];
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,(LPCWSTR)&initialize,&self);
    GetModuleFileNameA(self,log_path,MAX_PATH);
    char *slash=strrchr(log_path,'\\'); if(slash) strcpy(slash+1,"Blacklist-controller.log");
    UINT n=GetSystemDirectoryW(path,MAX_PATH);
    if(n && n+15<MAX_PATH) { wcscat(path,L"\\xinput1_3.dll"); original=LoadLibraryW(path); }
    log_line(original ? "Proxy initialized; system XInput loaded by absolute path." : "Proxy initialized; system XInput unavailable.");
    log_line(detection_hook_status);
    log_line(prompt_hook_status);
    ready=CreateEventW(NULL,TRUE,FALSE,NULL);
    HANDLE thread=CreateThread(NULL,0,poll_controller,NULL,0,NULL);
    if(thread) { CloseHandle(thread); WaitForSingleObject(ready,3000); }
    else log_line("ERROR: controller worker could not start.");
    return TRUE;
}
static void init(void) { InitOnceExecuteOnce(&once,initialize,NULL,NULL); }
BOOL controller_uses_playstation_prompts(void) {
    init();AcquireSRWLockShared(&lock);BOOL result=connected;ReleaseSRWLockShared(&lock);
    static LONG reported=-1;
    if(InterlockedExchange(&reported,result)!=result)
        log_line(result?"Controller prompt style: PlayStation (DualSense detected).":"Controller prompt style: Xbox (native XInput; no DualSense).");
    return result;
}
void controller_gate_notice(void) { init(); log_line("DualSense accepted by the game's controller identity classifier."); }
static BOOL sony(DWORD i) { return i==0 && connected; }
DWORD WINAPI proxyGetState(DWORD i,XINPUT_STATE *s) {
    if(!s || i>3) return ERROR_BAD_ARGUMENTS; init();
    AcquireSRWLockShared(&lock);
    if(sony(i)) { *s=cached; if(!enabled) ZeroMemory(&s->Gamepad,sizeof(s->Gamepad)); ReleaseSRWLockShared(&lock); return ERROR_SUCCESS; }
    ReleaseSRWLockShared(&lock);
    typedef DWORD (WINAPI *Fn)(DWORD,XINPUT_STATE*); Fn f=(Fn)native("XInputGetState"); return f?f(i,s):ERROR_DEVICE_NOT_CONNECTED;
}
DWORD WINAPI proxySetState(DWORD i,XINPUT_VIBRATION *v) {
    if(!v || i>3) return ERROR_BAD_ARGUMENTS; init(); AcquireSRWLockExclusive(&lock);
    if(sony(i)) { rumble=*v; ReleaseSRWLockExclusive(&lock); return ERROR_SUCCESS; }
    ReleaseSRWLockExclusive(&lock);
    typedef DWORD (WINAPI *Fn)(DWORD,XINPUT_VIBRATION*); Fn f=(Fn)native("XInputSetState"); return f?f(i,v):ERROR_DEVICE_NOT_CONNECTED;
}
DWORD WINAPI proxyGetCapabilities(DWORD i,DWORD flags,XINPUT_CAPABILITIES *c) {
    if(!c || i>3 || (flags & ~XINPUT_FLAG_GAMEPAD)) return ERROR_BAD_ARGUMENTS; init(); AcquireSRWLockShared(&lock);
    if(sony(i)) {
        ZeroMemory(c,sizeof(*c)); c->Type=XINPUT_DEVTYPE_GAMEPAD; c->SubType=XINPUT_DEVSUBTYPE_GAMEPAD;
        c->Flags=has_rumble?XINPUT_CAPS_FFB_SUPPORTED:0; c->Gamepad.wButtons=0xf3ff;
        c->Gamepad.bLeftTrigger=c->Gamepad.bRightTrigger=255;
        c->Gamepad.sThumbLX=c->Gamepad.sThumbLY=c->Gamepad.sThumbRX=c->Gamepad.sThumbRY=32767;
        if(has_rumble) c->Vibration.wLeftMotorSpeed=c->Vibration.wRightMotorSpeed=65535;
        ReleaseSRWLockShared(&lock); return ERROR_SUCCESS;
    }
    ReleaseSRWLockShared(&lock);
    typedef DWORD (WINAPI *Fn)(DWORD,DWORD,XINPUT_CAPABILITIES*); Fn f=(Fn)native("XInputGetCapabilities"); return f?f(i,flags,c):ERROR_DEVICE_NOT_CONNECTED;
}
void WINAPI proxyEnable(BOOL v) {
    init(); AcquireSRWLockExclusive(&lock); enabled=v; ReleaseSRWLockExclusive(&lock);
    typedef void (WINAPI *Fn)(BOOL); Fn f=(Fn)native("XInputEnable"); if(f) f(v);
}
DWORD WINAPI proxyBattery(DWORD i,BYTE type,XINPUT_BATTERY_INFORMATION *b) {
    if(!b || i>3 || type>BATTERY_DEVTYPE_HEADSET) return ERROR_BAD_ARGUMENTS; init(); AcquireSRWLockShared(&lock);
    if(sony(i)) { b->BatteryType=type==BATTERY_DEVTYPE_GAMEPAD?(BYTE)battery_type:BATTERY_TYPE_DISCONNECTED; b->BatteryLevel=(BYTE)battery_level; ReleaseSRWLockShared(&lock); return ERROR_SUCCESS; }
    ReleaseSRWLockShared(&lock);
    typedef DWORD (WINAPI *Fn)(DWORD,BYTE,XINPUT_BATTERY_INFORMATION*); Fn f=(Fn)native("XInputGetBatteryInformation"); return f?f(i,type,b):ERROR_DEVICE_NOT_CONNECTED;
}
DWORD WINAPI proxyAudio(DWORD i,GUID *render,GUID *capture) {
    if(i>3) return ERROR_BAD_ARGUMENTS; init(); AcquireSRWLockShared(&lock); BOOL ours=sony(i); ReleaseSRWLockShared(&lock);
    if(ours) { if(render) ZeroMemory(render,sizeof(*render)); if(capture) ZeroMemory(capture,sizeof(*capture)); return ERROR_DEVICE_NOT_CONNECTED; }
    typedef DWORD (WINAPI *Fn)(DWORD,GUID*,GUID*); Fn f=(Fn)native("XInputGetDSoundAudioDeviceGuids"); return f?f(i,render,capture):ERROR_DEVICE_NOT_CONNECTED;
}
DWORD WINAPI proxyKeystroke(DWORD i,DWORD reserved,XINPUT_KEYSTROKE *k) {
    if(!k || (i>3 && i!=XUSER_INDEX_ANY) || reserved) return ERROR_BAD_ARGUMENTS; init();
    AcquireSRWLockShared(&lock); BOOL ours=connected; ReleaseSRWLockShared(&lock);
    /* Blacklist does not import this API. No synthesized text/gamepad keystroke queue. */
    if(i==0 && ours) return ERROR_EMPTY;
    typedef DWORD (WINAPI *Fn)(DWORD,DWORD,XINPUT_KEYSTROKE*); Fn f=(Fn)native("XInputGetKeystroke");
    if(i==XUSER_INDEX_ANY && ours) { for(DWORD n=1;n<4;n++) { DWORD r=f?f(n,0,k):ERROR_DEVICE_NOT_CONNECTED; if(r==ERROR_SUCCESS) return r; } return ERROR_EMPTY; }
    return f?f(i,reserved,k):ERROR_DEVICE_NOT_CONNECTED;
}


