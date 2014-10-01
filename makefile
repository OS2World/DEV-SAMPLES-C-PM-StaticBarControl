# To add new extensions to the inference rules uncomment and redefine this:
#.SUFFIXES:
#
#.SUFFIXES: \
#    .C .obj .rc .res 

# compiler, linker, resource compiler, resource binder MACRO

CC = icc.exe
CL = ilink.exe
RC = rc.exe
RB = rc.exe

# compiler and linker flags
# Debug version
!ifdef DEBUG
# DLL version (subsystem)
!ifdef DLL
CFLAGS = /Ss /Ti /Rn /Ge- /G5 /C
LFLAGS = /DE /E:2 /PM:PM /PACKD /A:4 /OPTF /NOL /M /L
# EXE version (subsystem)
!else
CFLAGS = /Ss /Ti /Rn  /G5 /C
LFLAGS = /DE /E:2 /PM:PM /PACKD /A:4 /OPTF /NOL /M /L
!endif
# RELEASE version
!else
# DLL version (subsystem)
!ifdef DLL
CFLAGS = /Ss /O /Oc /Ol /Rn /Ge- /G5 /C
LFLAGS = /E:2 /PM:PM /PACKD /A:4 /OPTF /NOL /M /L
# EXE version (subsystem)
!else
CFLAGS = /Ss /O /Oc /Ol /Rn  /G5 /C
LFLAGS = /E:2 /PM:PM /PACKD /A:4 /OPTF /NOL /M /L
!endif
!endif

RCFLAGS = -r
RBFLAGS = -x2

.rc.res:
   $(RC) $(RCFLAGS) $<

.C.obj:
   $(CC) $(CFLAGS) $<

all: bar.exe

bar.exe: ctrlutil.obj superclass.obj bar.res
    $(CL) @<<
    $(LFLAGS)
    /O:bar.exe
    ctrlutil.obj
    superclass.obj
    <<
    $(RB) $(RBFLAGS) bar.res

bar.res: bar.rc superclass.h

ctrlutil.obj: ctrlutil.C dllmain.h ApiExPM.h ctrlutil.h

superclass.obj: superclass.C dllmain.h superclass.h ApiExPM.h ctrlutil.h
