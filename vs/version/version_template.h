#ifndef _AUTOVERSION_H_
#define _AUTOVERSION_H_

#define AUTO_UPDATE_VERSION

#define VER_MAJOR 1
#define VER_MINOR 0
#define VER_REVISION $WCREV$

#if 0
char *Revision = "$WCREV$";
char *Modified = "$WCMODS?Modified:Not modified$";
char *Date     = "$WCDATE$";
char *RevRange = "$WCRANGE$";
char *Mixed    = "$WCMIXED?Mixed revision WC:Not mixed$";
char *URL      = "$WCURL$";
#endif

#if $WCMODS?1:0$
#error Source is modified
#endif

#endif //!_AUTOVERSION_H_