#pragma once
#define AUTO_PRODUCT_REVISION $WCREV$
#define AUTO_PRODUCT_REVISION_STR "$WCREV$"

#define AUTO_FILE_REVISION $WCREV$
#define AUTO_FILE_REVISION_STR "$WCREV$"

#if $VER_ERROR$
#error get version fail
#endif