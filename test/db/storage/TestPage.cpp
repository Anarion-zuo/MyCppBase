//
// Created by anarion on 11/26/20.
//

#include <gtest/gtest.h>
#include <data/db/storage/PageManager.h>
#include <data/db/storage/BufferManager.h>
#include "function/Function.h"
#include <concurrent/base/WaitThread.h>

using namespace anarion;
using namespace anarion::db;

TEST(TestPage, TestInit) {
    size_type blockSize = 40960, pageSize = 4096, pageCount = 10, bufferCount = 15;
    FileBlockManager *blockManager = new FileBlockManager(Path(SString("./")), blockSize, pageSize, 1);
    BufferManager *bufferManager = new BufferManager(pageSize, bufferCount);
    PageManager pageManager(blockManager, 0, bufferManager, pageCount, pageSize);
}

TEST(TestPage, TestSimpleRW) {
    size_type blockSize = 40960, pageSize = 4096, pageCount = 10, bufferCount = 15;
    FileBlockManager *blockManager = new FileBlockManager(Path(SString("./")), blockSize, pageSize, 1);
    BufferManager *bufferManager = new BufferManager(pageSize, bufferCount);
    PageManager pageManager(blockManager, 0, bufferManager, pageCount, pageSize);
    pageManager.loadWriteRelease(0, 0, "12345", 5);
    char str[255] = {0};
    pageManager.loadReadRelease(0, 0, str, 5);
    ASSERT_TRUE(!strcmp(str, "12345"));
}

TEST(TestPage, TestHugeRW) {
    size_type blockSize = 40960, pageSize = 4096, pageCount = 10, bufferCount = 2;
    FileBlockManager *blockManager = new FileBlockManager(Path(SString("./")), blockSize, pageSize, 1);
    BufferManager *bufferManager = new BufferManager(pageSize, bufferCount);
    PageManager pageManager(blockManager, 0, bufferManager, pageCount, pageSize);

    const size_type stringLength = 4096;
    char character = '9';
    char str[stringLength + 1] = {character}, str2[stringLength + 1] = {character};
    for (size_type index = 0; index < stringLength; ++index) {
        str[index] = character;
        str2[index] = character;
    }
    str[stringLength] = 0;
    str2[stringLength] = 0;
    pageManager.loadWriteRelease(0, 0, str, stringLength);
    pageManager.loadReadRelease(0, 0, str, stringLength);
    ASSERT_TRUE(!strcmp(str, str2));
}

TEST(TestPage, TestEvict) {
    size_type blockSize = 40960, pageSize = 6, pageCount = 10, bufferCount = 1;
    FileBlockManager *blockManager = new FileBlockManager(Path(SString("./")), blockSize, pageSize, 1);
    BufferManager *bufferManager = new BufferManager(pageSize, bufferCount);
    PageManager pageManager(blockManager, 0, bufferManager, pageCount, pageSize);

    const char *page1 = "123456", *page2 = "abcdef";
    char buf[6];
    pageManager.loadWriteRelease(0, 0, page1, 6);
    pageManager.loadReadRelease(0, 0, buf, 6);
    ASSERT_EQ(0, strcmp(buf, page1));

    // must evict page1
    pageManager.loadWriteRelease(1, 0, page2, 6);
    pageManager.loadReadRelease(1, 0, buf, 6);
    ASSERT_EQ(0, strcmp(page2, buf));

    // page1 is still intact
    pageManager.loadReadRelease(0, 0, buf, 6);
    ASSERT_EQ(0, strcmp(buf, page1));
}

class PageFetchRoutine {
protected:
    PageManager *pageManager;
    pageno_t pageno;
    static const size_type sleepSec = 1;
public:
    PageFetchRoutine(PageManager *pageManager1, pageno_t pageno1) : pageManager(pageManager1), pageno(pageno1) {}
    PageFetchRoutine(const PageFetchRoutine &) = default;
    void operator()() {
        // load arbitrary page
        pageManager->load(pageno);
//        printf("evictable list\n", pageManager->getBufferManager().getEvictableCount());
        if (!pageManager->validBind(pageno)) {
            FAIL() << "Requesting page " << pageno << " failed. Buffer bound to another page.";
        }
//        sleep(sleepSec);
        pageManager->release(pageno);
    }
};

TEST(TestPage, Concurrent) {
    // init params
    size_type blockSize = 40960, pageSize = 6, pageCount = 4000, bufferCount = 100;
    FileBlockManager *blockManager = new FileBlockManager(Path(SString("./")), blockSize, pageSize, 1);
    BufferManager *bufferManager = new BufferManager(pageSize, bufferCount);
    PageManager pageManager(blockManager, 0, bufferManager, pageCount, pageSize);

    // prepare threads
    using thread_type = WaitThread<PageFetchRoutine>;
    Vector<thread_type> threads;
    size_type workerCount = 4000;
    threads.resize(workerCount);
    for (size_type i = 0; i < workerCount; ++i) {
        threads.emplaceBack(PageFetchRoutine(&pageManager, i));
    }
    // fetch all
    for (size_type i = 0; i < workerCount; ++i) {
        threads.get(i).startWait();
    }
    for (size_type i = 0; i < workerCount; ++i) {
        threads.get(i).launchWait();
    }
    // join all
    for (size_type i = 0; i < workerCount; ++i) {
        threads.get(i).join();
    }
    // fetch again
    for (size_type i = 0; i < workerCount; ++i) {
        threads.get(i).startWait();
    }
    for (size_type i = 0; i < workerCount; ++i) {
        threads.get(i).launchWait();
    }
    // join all
    for (size_type i = 0; i < workerCount; ++i) {
        threads.get(i).join();
    }
    if (bufferCount < workerCount) {
        ASSERT_EQ(bufferManager->getEvictableCount(), bufferCount);
    } else {
        ASSERT_EQ(workerCount, bufferManager->getEvictableCount());
    }
}
