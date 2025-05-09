#include "includes.h"
#include "bt_ble.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ble_app_bss")
#pragma data_seg(".ble_app_data")
#pragma const_seg(".ble_app_text_const")
#pragma code_seg(".ble_app_text")
#endif

#if (TRANS_DATA_HID_EN)
#if 0
extern char __VERSION_BEGIN[];
extern char __VERSION_END[];

_INLINE_
int app_version_check()
{
    char *version;

    puts("=================Version===============\n");
    for (version = __VERSION_BEGIN; version < __VERSION_END;) {
        version += 4;
        printf("%s\n", version);
        version += strlen(version) + 1;
    }
    puts("=======================================\n");

    return 0;
}

int lib_btstack_version(void)
{
    return 0;
}
int lib_media_version(void)
{
    return 0;
}
#endif
#endif

