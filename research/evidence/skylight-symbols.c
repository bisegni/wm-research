#include <dlfcn.h>
#include <stdio.h>
int main(void) { void *h=dlopen("/System/Library/PrivateFrameworks/SkyLight.framework/SkyLight",RTLD_LAZY|RTLD_LOCAL); if(!h){puts(dlerror());return 1;} const char *names[]={"SLSMainConnectionID","SLSNewConnection","SLSGetWindowBounds","SLSMoveWindow","SLSOrderWindow","SLSSetWindowLevel","SLSRegisterConnectionNotifyProc","SLSNewWindow","SLSReleaseWindow"};for(int i=0;i<9;i++) printf("%s %s\n",names[i],dlsym(h,names[i])?"PRESENT":"ABSENT");return 0;}
