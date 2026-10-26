#pragma once
#include <windows.h>
#include <string.h>

/* Keep real controller GUID pairs ahead of peripheral joystick interfaces,
 * preserving their order. At capacity, replace only an unprioritized tail. */
static void insert_priority_controller(BYTE *instances,BYTE *products,DWORD *count,DWORD *priority,
                                      const void *instance,const void *product) {
    DWORD n=*count,index=*priority;
    if(n>4 || index>=4) return;
    if(index>n) index=n;
    DWORD end=n<4?n:n-1;
    if(end>index) {
        memmove(instances+(index+1)*16,instances+index*16,(end-index)*16);
        memmove(products+(index+1)*16,products+index*16,(end-index)*16);
    }
    memcpy(instances+index*16,instance,16);memcpy(products+index*16,product,16);
    if(n<4) *count=n+1;
    *priority=index+1;
}
