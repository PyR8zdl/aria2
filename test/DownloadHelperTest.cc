#include "download_helper.h"

#include <iostream>
#include <string>
#include <deque>
#include <algorithm>

#include <cppunit/extensions/HelperMacros.h>

#include "RequestGroup.h"
#include "DownloadContext.h"
#include "Option.h"
#include "array_fun.h"
#include "prefs.h"
#include "Exception.h"
#include "Util.h"

namespace aria2 {

class DownloadHelperTest:public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(DownloadHelperTest);
  CPPUNIT_TEST(testCreateRequestGroupForUri);
  CPPUNIT_TEST(testCreateRequestGroupForUri_parameterized);
  CPPUNIT_TEST(testCreateRequestGroupForUri_BitTorrent);
  CPPUNIT_TEST(testCreateRequestGroupForUri_Metalink);
  CPPUNIT_TEST(testCreateRequestGroupForUriList);
  CPPUNIT_TEST(testCreateRequestGroupForBitTorrent);
  CPPUNIT_TEST(testCreateRequestGroupForMetalink);
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp() {}

  void tearDown() {}

  void testCreateRequestGroupForUri();
  void testCreateRequestGroupForUri_parameterized();
  void testCreateRequestGroupForUri_BitTorrent();
  void testCreateRequestGroupForUri_Metalink();
  void testCreateRequestGroupForUriList();
  void testCreateRequestGroupForBitTorrent();
  void testCreateRequestGroupForMetalink();
};


CPPUNIT_TEST_SUITE_REGISTRATION(DownloadHelperTest);

void DownloadHelperTest::testCreateRequestGroupForUri()
{
  std::string array[] = {
    "http://alpha/file",
    "http://bravo/file",
    "http://charlie/file"
  };
  std::deque<std::string> uris(&array[0], &array[arrayLength(array)]);
  Option op;
  op.put(PREF_SPLIT, "3");
  op.put(PREF_DIR, "/tmp");
  op.put(PREF_OUT, "file.out");
  {
    std::deque<SharedHandle<RequestGroup> > result;
    
    createRequestGroupForUri(result, &op, uris);
    
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.size());
    SharedHandle<RequestGroup> group = result[0];
    std::deque<std::string> uris;
    group->getURIs(uris);
    CPPUNIT_ASSERT_EQUAL((size_t)3, uris.size());
    for(size_t i = 0; i < arrayLength(array); ++i) {
      CPPUNIT_ASSERT_EQUAL(array[i], uris[i]);
    }
    CPPUNIT_ASSERT_EQUAL((unsigned int)3, group->getNumConcurrentCommand());
    SharedHandle<DownloadContext> ctx = group->getDownloadContext();
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp"), ctx->getDir());
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp/file.out"),
			 ctx->getActualBasePath());
  }
  op.put(PREF_SPLIT, "5");
  {
    std::deque<SharedHandle<RequestGroup> > result;
    
    createRequestGroupForUri(result, &op, uris);
    
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.size());
    SharedHandle<RequestGroup> group = result[0];
    std::deque<std::string> uris;
    group->getURIs(uris);
    CPPUNIT_ASSERT_EQUAL((size_t)5, uris.size());
    for(size_t i = 0; i < arrayLength(array); ++i) {
      CPPUNIT_ASSERT_EQUAL(array[i], uris[i]);
    }
    for(size_t i = 0; i < 5-arrayLength(array); ++i) {
      CPPUNIT_ASSERT_EQUAL(array[i], uris[i+arrayLength(array)]);
    }
    CPPUNIT_ASSERT_EQUAL((unsigned int)5, group->getNumConcurrentCommand());
  }
  op.put(PREF_SPLIT, "2");
  {
    std::deque<SharedHandle<RequestGroup> > result;
    
    createRequestGroupForUri(result, &op, uris);
    
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.size());
    SharedHandle<RequestGroup> group = result[0];
    std::deque<std::string> uris;
    group->getURIs(uris);
    CPPUNIT_ASSERT_EQUAL((size_t)3, uris.size());
    for(size_t i = 0; i < arrayLength(array); ++i) {
      CPPUNIT_ASSERT_EQUAL(array[i], uris[i]);
    }
    CPPUNIT_ASSERT_EQUAL((unsigned int)2, group->getNumConcurrentCommand());
  }
  op.put(PREF_FORCE_SEQUENTIAL, V_TRUE);
  {
    std::deque<SharedHandle<RequestGroup> > result;
    
    createRequestGroupForUri(result, &op, uris);
    
    CPPUNIT_ASSERT_EQUAL((size_t)3, result.size());

    // for alpha server
    SharedHandle<RequestGroup> alphaGroup = result[0];
    std::deque<std::string> alphaURIs;
    alphaGroup->getURIs(alphaURIs);
    CPPUNIT_ASSERT_EQUAL((size_t)2, alphaURIs.size());
    for(size_t i = 0; i < 2; ++i) {
      CPPUNIT_ASSERT_EQUAL(array[0], uris[0]);
    }
    CPPUNIT_ASSERT_EQUAL((unsigned int)2,
			 alphaGroup->getNumConcurrentCommand());
    SharedHandle<DownloadContext> alphaCtx = alphaGroup->getDownloadContext();
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp"), alphaCtx->getDir());
    // See the value of PREF_OUT is not used as a file name.
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp/index.html"),
			 alphaCtx->getActualBasePath());


  }
}

void DownloadHelperTest::testCreateRequestGroupForUri_parameterized()
{
  std::string array[] = {
    "http://{alpha, bravo}/file",
    "http://charlie/file"
  };
  std::deque<std::string> uris(&array[0], &array[arrayLength(array)]);
  Option op;
  op.put(PREF_SPLIT, "3");
  op.put(PREF_DIR, "/tmp");
  op.put(PREF_OUT, "file.out");
  op.put(PREF_PARAMETERIZED_URI, V_TRUE);
  {
    std::deque<SharedHandle<RequestGroup> > result;
    
    createRequestGroupForUri(result, &op, uris);
    
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.size());
    SharedHandle<RequestGroup> group = result[0];
    std::deque<std::string> uris;
    group->getURIs(uris);
    CPPUNIT_ASSERT_EQUAL((size_t)3, uris.size());

    CPPUNIT_ASSERT_EQUAL(std::string("http://alpha/file"), uris[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("http://bravo/file"), uris[1]);
    CPPUNIT_ASSERT_EQUAL(std::string("http://charlie/file"), uris[2]);

    CPPUNIT_ASSERT_EQUAL((unsigned int)3, group->getNumConcurrentCommand());
    SharedHandle<DownloadContext> ctx = group->getDownloadContext();
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp"), ctx->getDir());
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp/file.out"),
			 ctx->getActualBasePath());
  }
}

void DownloadHelperTest::testCreateRequestGroupForUri_BitTorrent()
{
  std::string array[] = {
    "http://alpha/file",
    "test.torrent",
    "http://bravo/file",
    "http://charlie/file"
  };
  std::deque<std::string> uris(&array[0], &array[arrayLength(array)]);
  Option op;
  op.put(PREF_SPLIT, "3");
  op.put(PREF_DIR, "/tmp");
  op.put(PREF_OUT, "file.out");
  {
    std::deque<SharedHandle<RequestGroup> > result;
    
    createRequestGroupForUri(result, &op, uris);
    
    CPPUNIT_ASSERT_EQUAL((size_t)2, result.size());
    SharedHandle<RequestGroup> group = result[0];
    std::deque<std::string> uris;
    group->getURIs(uris);
    CPPUNIT_ASSERT_EQUAL((size_t)3, uris.size());

    CPPUNIT_ASSERT_EQUAL(array[0], uris[0]);
    CPPUNIT_ASSERT_EQUAL(array[2], uris[1]);
    CPPUNIT_ASSERT_EQUAL(array[3], uris[2]);

    CPPUNIT_ASSERT_EQUAL((unsigned int)3, group->getNumConcurrentCommand());
    SharedHandle<DownloadContext> ctx = group->getDownloadContext();
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp"), ctx->getDir());
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp/file.out"),
			 ctx->getActualBasePath());

    SharedHandle<RequestGroup> torrentGroup = result[1];
    std::deque<std::string> auxURIs;
    torrentGroup->getURIs(auxURIs);
    CPPUNIT_ASSERT(auxURIs.empty());
    CPPUNIT_ASSERT_EQUAL((unsigned int)3,
			 torrentGroup->getNumConcurrentCommand());
    SharedHandle<DownloadContext> btctx = torrentGroup->getDownloadContext();
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp"), btctx->getDir());
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp/aria2-test"),
			 btctx->getActualBasePath());    
  }
}

void DownloadHelperTest::testCreateRequestGroupForUri_Metalink()
{
  std::string array[] = {
    "http://alpha/file",
    "http://bravo/file",
    "http://charlie/file",
    "test.xml"
  };
  std::deque<std::string> uris(&array[0], &array[arrayLength(array)]);
  Option op;
  op.put(PREF_SPLIT, "3");
  op.put(PREF_METALINK_SERVERS, "2");
  op.put(PREF_DIR, "/tmp");
  op.put(PREF_OUT, "file.out");
  {
    std::deque<SharedHandle<RequestGroup> > result;
    
    createRequestGroupForUri(result, &op, uris);
    
    // group1: http://alpha/file, ...
    // group2-7: 6 file entry in Metalink and 1 torrent file download
    CPPUNIT_ASSERT_EQUAL((size_t)7, result.size());
    SharedHandle<RequestGroup> group = result[0];
    std::deque<std::string> uris;
    group->getURIs(uris);
    CPPUNIT_ASSERT_EQUAL((size_t)3, uris.size());
    for(size_t i = 0; i < 3; ++i) {
      CPPUNIT_ASSERT_EQUAL(array[i], uris[i]);
    }
    CPPUNIT_ASSERT_EQUAL((unsigned int)3, group->getNumConcurrentCommand());
    SharedHandle<DownloadContext> ctx = group->getDownloadContext();
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp"), ctx->getDir());
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp/file.out"),
			 ctx->getActualBasePath());

    SharedHandle<RequestGroup> aria2052Group = result[1];
    CPPUNIT_ASSERT_EQUAL((unsigned int)1, // because of maxconnections attribute
			 aria2052Group->getNumConcurrentCommand());
    SharedHandle<DownloadContext> aria2052Ctx =
      aria2052Group->getDownloadContext();
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp"), aria2052Ctx->getDir());
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp/aria2-0.5.2.tar.bz2"),
			 aria2052Ctx->getActualBasePath());
    
    SharedHandle<RequestGroup> aria2051Group = result[2];
    CPPUNIT_ASSERT_EQUAL((unsigned int)2,
			 aria2051Group->getNumConcurrentCommand());
  }
}

void DownloadHelperTest::testCreateRequestGroupForUriList()
{
  Option op;
  op.put(PREF_SPLIT, "3");
  op.put(PREF_INPUT_FILE, "input_uris.txt");
  op.put(PREF_DIR, "/tmp");
  op.put(PREF_OUT, "file.out");

  std::deque<SharedHandle<RequestGroup> > result;
  
  createRequestGroupForUriList(result, &op);

  CPPUNIT_ASSERT_EQUAL((size_t)2, result.size());

  SharedHandle<RequestGroup> fileGroup = result[0];
  std::deque<std::string> fileURIs;
  fileGroup->getURIs(fileURIs);
  CPPUNIT_ASSERT_EQUAL(std::string("http://alpha/file"), fileURIs[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://bravo/file"), fileURIs[1]);
  CPPUNIT_ASSERT_EQUAL(std::string("http://charlie/file"), fileURIs[2]);
  CPPUNIT_ASSERT_EQUAL((unsigned int)3, fileGroup->getNumConcurrentCommand());
  SharedHandle<DownloadContext> fileCtx = fileGroup->getDownloadContext();
  CPPUNIT_ASSERT_EQUAL(std::string("/mydownloads"), fileCtx->getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("/mydownloads/myfile.out"),
		       fileCtx->getActualBasePath());

  SharedHandle<RequestGroup> fileISOGroup = result[1];
  SharedHandle<DownloadContext> fileISOCtx = fileISOGroup->getDownloadContext();
  CPPUNIT_ASSERT_EQUAL(std::string("/tmp"), fileISOCtx->getDir());
  CPPUNIT_ASSERT_EQUAL(std::string("/tmp/file.out"),
		       fileISOCtx->getActualBasePath());
}

void DownloadHelperTest::testCreateRequestGroupForBitTorrent()
{
  std::string array[] = {
    "http://alpha/file",
    "http://bravo/file",
    "http://charlie/file"
  };

  std::deque<std::string> auxURIs(&array[0], &array[arrayLength(array)]);
  Option op;
  op.put(PREF_SPLIT, "5");
  op.put(PREF_TORRENT_FILE, "test.torrent");
  op.put(PREF_DIR, "/tmp");
  op.put(PREF_OUT, "file.out");
  {
    std::deque<SharedHandle<RequestGroup> > result;
  
    createRequestGroupForBitTorrent(result, &op, auxURIs);

    CPPUNIT_ASSERT_EQUAL((size_t)1, result.size());

    SharedHandle<RequestGroup> group = result[0];
    std::deque<std::string> uris;
    group->getURIs(uris);
    CPPUNIT_ASSERT_EQUAL((size_t)5, uris.size());
    for(size_t i = 0; i < arrayLength(array); ++i) {
      CPPUNIT_ASSERT_EQUAL(array[i], uris[i]);
    }
    for(size_t i = 0; i < 5-arrayLength(array); ++i) {
      CPPUNIT_ASSERT_EQUAL(array[i], uris[i+arrayLength(array)]);
    }
    CPPUNIT_ASSERT_EQUAL((unsigned int)5, group->getNumConcurrentCommand());
  }
  op.put(PREF_FORCE_SEQUENTIAL, V_TRUE);
  {
    std::deque<SharedHandle<RequestGroup> > result;
  
    createRequestGroupForBitTorrent(result, &op, auxURIs);

    // See --force-requencial is ignored
    CPPUNIT_ASSERT_EQUAL((size_t)1, result.size());
  }
}

void DownloadHelperTest::testCreateRequestGroupForMetalink()
{
  Option op;
  op.put(PREF_SPLIT, "3");
  op.put(PREF_METALINK_FILE, "test.xml");
  op.put(PREF_METALINK_SERVERS, "5");
  op.put(PREF_DIR, "/tmp");
  op.put(PREF_OUT, "file.out");
  {
    std::deque<SharedHandle<RequestGroup> > result;
  
    createRequestGroupForMetalink(result, &op);

    CPPUNIT_ASSERT_EQUAL((size_t)6, result.size());

    SharedHandle<RequestGroup> group = result[0];
    std::deque<std::string> uris;
    group->getURIs(uris);
    std::sort(uris.begin(), uris.end());
    CPPUNIT_ASSERT_EQUAL((size_t)2, uris.size());
    CPPUNIT_ASSERT_EQUAL(std::string("ftp://ftphost/aria2-0.5.2.tar.bz2"),
			 uris[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("http://httphost/aria2-0.5.2.tar.bz2"),
			 uris[1]);
    // See numConcurrentCommand is 1 because of maxconnections attribute.
    CPPUNIT_ASSERT_EQUAL((unsigned int)1, group->getNumConcurrentCommand());
  }
}

} // namespace aria2
