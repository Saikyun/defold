#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <gtest/gtest.h>
#include "../dlib/sys.h"
#include "../dlib/path.h"
#include "../dlib/log.h"

TEST(dmSys, Mkdir)
{
    dmSys::Result r;
    r = dmSys::Mkdir("tmp/dir", 0777);
    ASSERT_EQ(dmSys::RESULT_OK, r);

    r = dmSys::Mkdir("tmp/dir", 0777);
    ASSERT_EQ(dmSys::RESULT_EXIST, r);

    r = dmSys::Rmdir("tmp/dir");
    ASSERT_EQ(dmSys::RESULT_OK, r);
}

TEST(dmSys, Unlink)
{
    dmSys::Result r;
    r = dmSys::Unlink("tmp/afile");
    ASSERT_EQ(dmSys::RESULT_NOENT, r);

    FILE* f = fopen("tmp/afile", "wb");
    ASSERT_NE((FILE*) 0, f);
    fclose(f);

    r = dmSys::Unlink("tmp/afile");
    ASSERT_EQ(dmSys::RESULT_OK, r);
}

TEST(dmSys, GetApplicationSupportPathBuffer)
{
    char path[4];
    path[3] = '!';
    dmSys::Result result = dmSys::GetApplicationSupportPath("testing", path, 3);
    ASSERT_EQ(dmSys::RESULT_INVAL, result);
    ASSERT_EQ('\0', path[2]);
    ASSERT_EQ('!', path[3]);
}

TEST(dmSys, GetApplicationSupportPath)
{
    char path[1024];
    dmSys::Result result = dmSys::GetApplicationSupportPath("testing", path, sizeof(path));
    ASSERT_EQ(dmSys::RESULT_OK, result);
    struct stat stat_data;
    int ret = stat(path, &stat_data);
    ASSERT_EQ(0, ret);
    ASSERT_EQ(S_IFDIR, stat_data.st_mode & S_IFDIR);
}

int g_Argc;
char** g_Argv;

TEST(dmSys, GetResourcesPath)
{
    char path[DMPATH_MAX_PATH];
    dmSys::GetResourcesPath(g_Argc, g_Argv, path, sizeof(path));
    printf("GetResourcesPath: '%s'\n", path);
}

TEST(dmSys, GetSystemInfo)
{
    dmSys::SystemInfo info;
    dmSys::GetSystemInfo(&info);

    dmLogInfo("DeviceModel: '%s'", info.m_DeviceModel);
    dmLogInfo("SystemName: '%s'", info.m_SystemName);
    dmLogInfo("SystemVersion: '%s'", info.m_SystemVersion);
    dmLogInfo("Language: '%s'", info.m_Language);
    dmLogInfo("Device Language: '%s'", info.m_DeviceLanguage);
    dmLogInfo("Territory: '%s'", info.m_Territory);
    dmLogInfo("GMT offset: '%d'", info.m_GmtOffset);
    dmLogInfo("Device identifier: '%d'", info.m_DeviceIdentifier);
}

TEST(dmSys, LoadResource)
{
    char buffer[1024 * 100];
    dmSys::Result r;
    uint32_t size;
    r = dmSys::LoadResource("does_not_exists", buffer, sizeof(buffer), &size);
    ASSERT_EQ(dmSys::RESULT_NOENT, r);
    r = dmSys::ResourceSize("does_not_exists", &size);
    ASSERT_EQ(dmSys::RESULT_NOENT, r);

    r = dmSys::LoadResource(".", buffer, sizeof(buffer), &size);
    ASSERT_EQ(dmSys::RESULT_NOENT, r);
    r = dmSys::ResourceSize("does_not_exists", &size);
    ASSERT_EQ(dmSys::RESULT_NOENT, r);

    r = dmSys::LoadResource("wscript", 0, 0, &size);
    ASSERT_EQ(dmSys::RESULT_INVAL, r);

    r = dmSys::LoadResource("wscript", buffer, sizeof(buffer), &size);
    ASSERT_EQ(dmSys::RESULT_OK, r);
    uint32_t size2;
    r = dmSys::ResourceSize("wscript", &size2);
    ASSERT_EQ(dmSys::RESULT_OK, r);
    ASSERT_EQ(size, size2);
    ASSERT_GT(size, 0);
}

int main(int argc, char **argv)
{
    g_Argc = argc;
    g_Argv = argv;
    system("python src/test/test_sys.py");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

