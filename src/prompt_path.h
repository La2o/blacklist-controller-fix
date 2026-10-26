#pragma once
#include <string.h>
#include <stdio.h>

/* Only button DDS assets in the Flash UI. Never redirect keyboard artwork,
 * ordinary world textures or arbitrary files with a matching basename. */
static int prompt_replacement(const char *path,char out[256]) {
    if(!path) return 0;
    char normalized[1024];size_t n=strlen(path);
    if(n>=sizeof(normalized)) return 0;
    for(size_t i=0;i<=n;i++) {
        char c=path[i];normalized[i]=c=='/'?'\\':c>='A'&&c<='Z'?c+('a'-'A'):c;
    }
    if(!strstr(normalized,"flash\\textures\\") && !strstr(normalized,"flash\\pc_textures\\")) return 0;
    if(strstr(normalized,"sc6iconsps3\\")) return 0;
    char *base=strrchr(normalized,'\\');if(!base) return 0;base++;
    const char *prefix="icon_ik_joy";size_t len=strlen(prefix);
    if(strncmp(base,prefix,len)) return 0;
    const char *digits=base+len;int button=0;
    if(*digits<'1'||*digits>'9') return 0;
    while(*digits>='0'&&*digits<='9') { button=button*10+(*digits++-'0');if(button>16) return 0; }
    if(button<1||strcmp(digits,".dds")) return 0;
    snprintf(out,256,"..\\..\\data\\flash\\textures\\SC6IconsPS3\\ICON_IK_JOY%d.dds",button);
    return button;
}
