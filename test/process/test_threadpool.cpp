#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "process.hpp"
#include "threadpool_impl.hpp"

#include "mock_process.hpp"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::Invoke;
using ::testing::Throw;

class TestThreadpool : public ::testing::Test {};

// Test a single threaded pool without processes to run without throwing or
// segfaulting
TEST_F(TestThreadpool, TestThreadpoolSingleThreadNoProcess) {
  threadpool::Threadpool_impl tp = threadpool::Threadpool_impl(1);

  EXPECT_NO_THROW({
    tp.start();
    tp.stop();
  });
}

// Test a multi threaded (nr based on hardware) pool without processes to run
// without throwing or segfaulting
TEST_F(TestThreadpool, TestThreadpoolSysNrThreadNoProcess) {
  threadpool::Threadpool_impl tp = threadpool::Threadpool_impl();

  EXPECT_NO_THROW({
    tp.start();
    tp.stop();
  });
}

// Test a multi threaded (nr based on hardware) pool with one process to run
// without throwing or segfaulting. Expect process to be executed.
TEST_F(TestThreadpool, TestThreadpoolSysNrThreadOneProcess) {
  int id = 1;
  std::shared_ptr<MockProcess> process(
      new MockProcess(ProcessOwner::process_manager, "testProcess1"));
  threadpool::Threadpool_impl tp = threadpool::Threadpool_impl();

  EXPECT_CALL(*process.get(), execute(id)).Times(1);

  EXPECT_NO_THROW({
    tp.start();
    tp.add_to_queue(id, process);
    while (tp.busy())
      ;
    tp.stop();
  });
}

// Test a multi threaded (nr based on hardware) pool with multiple processes to
// run without throwing or segfaulting. Expect each process to be executed.
TEST_F(TestThreadpool, TestThreadpoolSysNrThreadMultiProcess) {
  std::shared_ptr<MockProcess> process(
      new MockProcess(ProcessOwner::process_manager, "testProcess1"));
  threadpool::Threadpool_impl tp = threadpool::Threadpool_impl();
  auto f = [](int id) { std::cout << "Testing " << id << std::endl; };

  for (int id = 0; id < 100; id++) {
    EXPECT_CALL(*process.get(), execute(id)).Times(1).WillOnce(Invoke(f));
  }

  EXPECT_NO_THROW({
    tp.start();
    for (int id = 0; id < 100; id++) {
      tp.add_to_queue(id, process);
    }

    while (tp.busy())
      ;
    tp.stop();
  });

  EXPECT_EQ(0, tp.get_nr_running());
  EXPECT_EQ(0, tp.get_nr_queued());
}

// Test a multi threaded (nr based on hardware) pool with multiple processes to
// run without segfaulting. Expect each process to be executed. One process will
// throw, which should not affect execution by threadpool
TEST_F(TestThreadpool, TestThreadpoolSysNrThreadMultiProcessThrows) {
  std::shared_ptr<MockProcess> process(
      new MockProcess(ProcessOwner::process_manager, "testProcess1"));
  threadpool::Threadpool_impl tp = threadpool::Threadpool_impl();
  auto f = [](int id) { std::cout << "Testing " << id << std::endl; };

  {
    testing::InSequence s;
    EXPECT_CALL(*process.get(), execute(_)).Times(99).WillRepeatedly(Invoke(f));
    EXPECT_CALL(*process.get(), execute(_))
        .Times(1)
        .WillOnce(Throw(std::exception()));
  }

  EXPECT_NO_THROW({
    tp.start();
    for (int id = 0; id < 100; id++) {
      tp.add_to_queue(id, process);
    }

    while (tp.busy())
      ;
    tp.stop();
  });

  EXPECT_EQ(0, tp.get_nr_running());
  EXPECT_EQ(0, tp.get_nr_queued());
}

// Test a multi threaded (nr based on hardware) pool with multiple processes to
// run without throwing or segfaulting. Stop without waiting for threadpool to
// not be busy. Expect at least one process to be executed.
TEST_F(TestThreadpool, TestThreadpoolSysNrThreadMultiProcessStopNoWait) {
  std::shared_ptr<MockProcess> process(
      new MockProcess(ProcessOwner::process_manager, "testProcess1"));
  threadpool::Threadpool_impl tp = threadpool::Threadpool_impl();
  auto f = [](int id) { std::cout << "Testing " << id << std::endl; };

  EXPECT_CALL(*process.get(), execute(_))
      .Times(AtLeast(1))
      .WillRepeatedly(Invoke(f));

  EXPECT_NO_THROW({
    tp.start();
    for (int id = 0; id < 100; id++) {
      tp.add_to_queue(id, process);
    }
    tp.stop();
  });

  EXPECT_EQ(0, tp.get_nr_running());
  EXPECT_EQ(0, tp.get_nr_queued());
}
