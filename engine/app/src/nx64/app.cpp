// Some good notes on the app flow
//    https://developer.nintendo.com/html/online-docs/nx-en/g1kr9vj6-en/Packages/SDK/NintendoSDK/Documents/Package/contents/Pages/Page_110932904.html
//    https://developer.nintendo.com/html/online-docs/nx-en/g1kr9vj6-en/Packages/SDK/NintendoSDK/Documents/Package/contents/Pages/Page_105383443.html
// Exiting the program:
//    https://developer.nintendo.com/html/online-docs/nx-en/g1kr9vj6-en/Packages/SDK/NintendoSDK/Documents/Package/contents/Pages/Page_162179114.html

#include <app/app.h>
#include "app_test.h"
#include <stdio.h>

#include <nn/fs.h>
#include <nn/os.h>

extern "C" int main(int argc, char** argv);

extern "C" void nnMain()
{
	// https://developer.nintendo.com/html/online-docs/nx-en/g1kr9vj6-en/Packages/SDK/NintendoSDK/Documents/Package/contents/external.html?file=../../Api/HtmlNX/index.html
    size_t mount_rom_cache_size = 0;
    nn::fs::QueryMountRomCacheSize(&mount_rom_cache_size);
    void* mount_rom_cache_buffer = malloc(mount_rom_cache_size);
    nn::fs::MountRom("data", mount_rom_cache_buffer, mount_rom_cache_size);

    dmApp::TestInitialize(); // nop in the correct library

    int r = main(nn::os::GetHostArgc(), nn::os::GetHostArgv());
    (void)r;

    dmApp::TestFinalize();

    nn::fs::Unmount("data");
    free(mount_rom_cache_buffer);
}
