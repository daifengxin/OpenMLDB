//
// tablet_impl_test.cc
// Copyright (C) 2017 4paradigm.com
// Author wangtaize 
// Date 2017-04-05
//

#include "tablet/tablet_impl.h"
#include "proto/tablet.pb.h"
#include "base/kv_iterator.h"
#include "gtest/gtest.h"
#include "logging.h"
#include "timer.h"

namespace rtidb {
namespace tablet {

class MockClosure : public ::google::protobuf::Closure {

public:
    MockClosure() {}
    ~MockClosure() {}
    void Run() {}

};

class TabletImplTest : public ::testing::Test {

public:
    TabletImplTest() {}
    ~TabletImplTest() {}
};

TEST_F(TabletImplTest, TTL) {
    uint64_t now = ::baidu::common::timer::get_micros() / 1000;
    TabletImpl tablet;
    tablet.Init();
    ::rtidb::api::CreateTableRequest request;
    request.set_name("t0");
    request.set_tid(1);
    request.set_pid(1);
    // 1 minutes
    request.set_ttl(1);
    ::rtidb::api::CreateTableResponse response;
    MockClosure closure;
    tablet.CreateTable(NULL, &request, &response,
            &closure);
    ASSERT_EQ(0, response.code());
    {
        ::rtidb::api::PutRequest prequest;
        prequest.set_pk("test1");
        prequest.set_time(now);
        prequest.set_value("test1");
        prequest.set_tid(1);
        prequest.set_pid(1);
        ::rtidb::api::PutResponse presponse;
        tablet.Put(NULL, &prequest, &presponse,
                &closure);
        ASSERT_EQ(0, presponse.code());

    }
    {
        ::rtidb::api::PutRequest prequest;
        prequest.set_pk("test1");
        prequest.set_time(now - 2 * 60 * 1000);
        prequest.set_value("test2");
        prequest.set_tid(1);
        prequest.set_pid(1);
        ::rtidb::api::PutResponse presponse;
        tablet.Put(NULL, &prequest, &presponse,
                &closure);
        ASSERT_EQ(0, presponse.code());
    }
    {
        ::rtidb::api::ScanRequest sr;
        sr.set_tid(1);
        sr.set_pid(1);
        sr.set_pk("test1");
        sr.set_st(now);
        sr.set_et(now - 3 * 60 * 1000);
        ::rtidb::api::ScanResponse* srp = new ::rtidb::api::ScanResponse();
        tablet.Scan(NULL, &sr, srp, &closure);
        ASSERT_EQ(1, srp->count());
        ::rtidb::base::KvIterator it(srp);
        ASSERT_TRUE(it.Valid());
        ASSERT_EQ(now , it.GetKey());
        ASSERT_EQ("test1", it.GetValue().ToString());
        it.Next();
        ASSERT_FALSE(it.Valid());
    }

}



TEST_F(TabletImplTest, CreateTable) {
    TabletImpl tablet;
    tablet.Init();
    {
        ::rtidb::api::CreateTableRequest request;
        request.set_name("t0");
        request.set_tid(1);
        request.set_pid(1);
        request.set_ttl(0);
        ::rtidb::api::CreateTableResponse response;
        MockClosure closure;
        tablet.CreateTable(NULL, &request, &response,
                &closure);
        ASSERT_EQ(0, response.code());
        request.set_name("");
        tablet.CreateTable(NULL, &request, &response,
                &closure);
        ASSERT_EQ(8, response.code());
    }
    {
        ::rtidb::api::CreateTableRequest request;
        request.set_name("t0");
        request.set_ttl(0);
        ::rtidb::api::CreateTableResponse response;
        MockClosure closure;
        tablet.CreateTable(NULL, &request, &response,
                &closure);
        ASSERT_EQ(8, response.code());
    }

}

TEST_F(TabletImplTest, Put) {
    TabletImpl tablet;

    tablet.Init();
    ::rtidb::api::CreateTableRequest request;
    request.set_name("t0");
    request.set_tid(1);
    request.set_pid(1);
    request.set_ttl(0);
    ::rtidb::api::CreateTableResponse response;
    MockClosure closure;
    tablet.CreateTable(NULL, &request, &response,
            &closure);
    ASSERT_EQ(0, response.code());

    ::rtidb::api::PutRequest prequest;
    prequest.set_pk("test1");
    prequest.set_time(9527);
    prequest.set_value("test0");
    prequest.set_tid(2);
    prequest.set_pid(2);
    ::rtidb::api::PutResponse presponse;
    tablet.Put(NULL, &prequest, &presponse,
            &closure);
    ASSERT_EQ(10, presponse.code());
    prequest.set_tid(1);
    prequest.set_pid(1);
    tablet.Put(NULL, &prequest, &presponse,
            &closure);
    ASSERT_EQ(0, presponse.code());
}

TEST_F(TabletImplTest, Scan) {
    TabletImpl tablet;

    tablet.Init();
    ::rtidb::api::CreateTableRequest request;
    request.set_name("t0");
    request.set_tid(1);
    request.set_pid(1);
    request.set_ttl(0);
    ::rtidb::api::CreateTableResponse response;
    MockClosure closure;
    tablet.CreateTable(NULL, &request, &response,
            &closure);
    ASSERT_EQ(0, response.code());
    ::rtidb::api::ScanRequest sr;
    sr.set_tid(2);
    sr.set_pk("test1");
    sr.set_st(9528);
    sr.set_et(9527);
    sr.set_limit(10);
    ::rtidb::api::ScanResponse srp;
    tablet.Scan(NULL, &sr, &srp, &closure);
    ASSERT_EQ(0, srp.pairs().size());
    ASSERT_EQ(10, srp.code());

    sr.set_tid(1);
    sr.set_pid(1);
    tablet.Scan(NULL, &sr, &srp, &closure);
    ASSERT_EQ(0, srp.code());
    ASSERT_EQ(0, srp.count());

    {
        ::rtidb::api::PutRequest prequest;
        prequest.set_pk("test1");
        prequest.set_time(9527);
        prequest.set_value("test0");
        prequest.set_tid(2);
        ::rtidb::api::PutResponse presponse;
        tablet.Put(NULL, &prequest, &presponse,
                &closure);

        ASSERT_EQ(10, presponse.code());
        prequest.set_tid(1);
        prequest.set_pid(1);
        tablet.Put(NULL, &prequest, &presponse,
                &closure);

        ASSERT_EQ(0, presponse.code());

    }
    {
        ::rtidb::api::PutRequest prequest;
        prequest.set_pk("test1");
        prequest.set_time(9528);
        prequest.set_value("test0");
        prequest.set_tid(2);
        ::rtidb::api::PutResponse presponse;
        tablet.Put(NULL, &prequest, &presponse,
                &closure);

        ASSERT_EQ(10, presponse.code());
        prequest.set_tid(1);
        prequest.set_pid(1);

        tablet.Put(NULL, &prequest, &presponse,
                &closure);

        ASSERT_EQ(0, presponse.code());

    }
    tablet.Scan(NULL, &sr, &srp, &closure);
    ASSERT_EQ(0, srp.code());
    ASSERT_EQ(1, srp.count());

}

TEST_F(TabletImplTest, GC) {
    TabletImpl tablet;

    tablet.Init();
    ::rtidb::api::CreateTableRequest request;
    request.set_name("t0");
    request.set_tid(1);
    request.set_pid(1);
    request.set_ttl(1);
    ::rtidb::api::CreateTableResponse response;
    MockClosure closure;
    tablet.CreateTable(NULL, &request, &response,
            &closure);
    ASSERT_EQ(0, response.code());

    ::rtidb::api::PutRequest prequest;
    prequest.set_pk("test1");
    prequest.set_time(9527);
    prequest.set_value("test0");
    prequest.set_tid(1);
    prequest.set_pid(1);
    ::rtidb::api::PutResponse presponse;
    tablet.Put(NULL, &prequest, &presponse,
            &closure);
    uint64_t now = ::baidu::common::timer::get_micros() / 1000;
    prequest.set_time(now);
    tablet.Put(NULL, &prequest, &presponse,
            &closure);
    ::rtidb::api::ScanRequest sr;
    sr.set_tid(1);
    sr.set_pid(1);
    sr.set_pk("test1");
    sr.set_st(now);
    sr.set_et(9527);
    sr.set_limit(10);
    ::rtidb::api::ScanResponse srp;
    tablet.Scan(NULL, &sr, &srp, &closure);
    ASSERT_EQ(0, srp.code());
    ASSERT_EQ(1, srp.count());

}

TEST_F(TabletImplTest, DropTable) {
    TabletImpl tablet;
    tablet.Init();
    MockClosure closure;
    ::rtidb::api::DropTableRequest dr;
    dr.set_tid(1);
    dr.set_pid(1);
    ::rtidb::api::DropTableResponse drs;
    tablet.DropTable(NULL, &dr, &drs, &closure);
    ASSERT_EQ(-1, drs.code());

    ::rtidb::api::CreateTableRequest request;
    request.set_name("t0");
    request.set_tid(1);
    request.set_pid(1);
    request.set_ttl(1);
    ::rtidb::api::CreateTableResponse response;
    tablet.CreateTable(NULL, &request, &response,
            &closure);
    ASSERT_EQ(0, response.code());

    tablet.DropTable(NULL, &dr, &drs, &closure);
    ASSERT_EQ(0, drs.code());

    ::rtidb::api::PutRequest prequest;
    prequest.set_pk("test1");
    prequest.set_time(9527);
    prequest.set_value("test0");
    prequest.set_tid(1);
    ::rtidb::api::PutResponse presponse;
    tablet.Put(NULL, &prequest, &presponse,
            &closure);
    ASSERT_EQ(10, presponse.code());
}

TEST_F(TabletImplTest, DropTableFollower) {
    TabletImpl tablet;
    tablet.Init();
    MockClosure closure;
    ::rtidb::api::DropTableRequest dr;
    dr.set_tid(1);
    dr.set_pid(1);
    ::rtidb::api::DropTableResponse drs;
    tablet.DropTable(NULL, &dr, &drs, &closure);
    ASSERT_EQ(-1, drs.code());

    ::rtidb::api::CreateTableRequest request;
    request.set_name("t0");
    request.set_tid(1);
    request.set_pid(1);
    request.set_ttl(1);
    request.set_mode(::rtidb::api::TableMode::kTableLeader);
    request.add_replicas("127.0.0.1:9527");
    ::rtidb::api::CreateTableResponse response;
    tablet.CreateTable(NULL, &request, &response,
            &closure);
    ASSERT_EQ(0, response.code());
    ::rtidb::api::PutRequest prequest;
    prequest.set_pk("test1");
    prequest.set_time(9527);
    prequest.set_value("test0");
    prequest.set_tid(1);
    prequest.set_pid(1);
    ::rtidb::api::PutResponse presponse;
    tablet.Put(NULL, &prequest, &presponse,
            &closure);
    //ReadOnly
    ASSERT_EQ(20, presponse.code());

    tablet.DropTable(NULL, &dr, &drs, &closure);
    ASSERT_EQ(0, drs.code());
    prequest.set_pk("test1");
    prequest.set_time(9527);
    prequest.set_value("test0");
    prequest.set_tid(1);
    prequest.set_pid(1);
    tablet.Put(NULL, &prequest, &presponse,
            &closure);
    ASSERT_EQ(10, presponse.code());
}



}
}

int main(int argc, char** argv) {
    ::baidu::common::SetLogLevel(::baidu::common::DEBUG);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}



