#include "threadpool_impl.hpp"

#include "logging.hpp"

namespace process {
  namespace threadpool {
    Threadpool_impl::Threadpool_impl()
        : Threadpool_impl(std::thread::hardware_concurrency()) {
      logging::Log(logging::LogLevel::debug) << logging::LOG_START;
      logging::Log(logging::LogLevel::debug) << logging::LOG_END;
    }

    Threadpool_impl::Threadpool_impl(int nr_threads)
        : m_nr_threads(nr_threads), m_nr_running(0), m_should_stop(false) {
      logging::Log(logging::LogLevel::debug) << logging::LOG_START;
      logging::Log(logging::LogLevel::debug) << logging::LOG_HEADER << "Starting with " << m_nr_threads
                       << " threads";
      logging::Log(logging::LogLevel::debug) << logging::LOG_END;
    }

    void Threadpool_impl::start() {
      logging::Log(logging::LogLevel::debug) << logging::LOG_START;

      for (int i = 0; i < m_nr_threads; i++) {
        m_threads.emplace_back(std::thread(&Threadpool_impl::thread_run, this));
      }

      logging::Log(logging::LogLevel::debug) << logging::LOG_END;
    }

    void Threadpool_impl::stop() {
      logging::Log(logging::LogLevel::debug) << logging::LOG_START;

      {
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        m_should_stop = true;
      }
      m_mutex_condition.notify_all();

      for (auto &thread : m_threads) {
        thread.join();
      }
      m_threads.clear();

      logging::Log(logging::LogLevel::debug) << logging::LOG_END;
    }

    void Threadpool_impl::add_to_queue(const int &id,
                                      std::shared_ptr<IProcess> process) {
      logging::Log(logging::LogLevel::debug) << logging::LOG_START;
      {
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        m_queue.push(std::make_pair(id, process));
      }
      m_mutex_condition.notify_one();

      logging::Log(logging::LogLevel::debug) << logging::LOG_END;
    }

    int Threadpool_impl::get_nr_queued() {
      logging::Log(logging::LogLevel::debug) << logging::LOG_START;

      int nr_queued;
      {
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        nr_queued = static_cast<int>(m_queue.size());
      }

      logging::Log(logging::LogLevel::debug) << logging::LOG_END;
      return nr_queued;
    }

    int Threadpool_impl::get_nr_running() {
      logging::Log(logging::LogLevel::debug) << logging::LOG_START;
      logging::Log(logging::LogLevel::debug) << logging::LOG_END;
      return m_nr_running;
    }

    bool Threadpool_impl::busy() {
      logging::Log(logging::LogLevel::debug) << logging::LOG_START;

      bool isbusy;
      {
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        isbusy = (!m_queue.empty()) || (m_nr_running > 0);
      }

      logging::Log(logging::LogLevel::debug) << logging::LOG_END;
      return isbusy;
    }

    void Threadpool_impl::thread_run() {
      logging::Log(logging::LogLevel::debug) << logging::LOG_START;

      while (true) {
        int id = -1;
        std::shared_ptr<IProcess> process;

        {
          std::unique_lock<std::mutex> lock(m_queue_mutex);
          m_mutex_condition.wait(
              lock, [this] { return !m_queue.empty() || m_should_stop; });
          if (m_should_stop) {
            return;
          }

          logging::Log(logging::LogLevel::debug) << logging::LOG_HEADER << "Getting id and process to execute";
          id      = m_queue.front().first;
          process = m_queue.front().second;
          m_queue.pop();
        }

        if ((process != nullptr)) {
          logging::Log(logging::LogLevel::debug) << logging::LOG_HEADER << "Executing process with id " << id;
          m_nr_running++;
          try {
            process->execute(id);

          } catch (const ProcessException &e) {
            logging::Log(logging::LogLevel::error)
                << logging::LOG_HEADER << "Process with id " << id
                << " failed with a process exception: " << e.what();
          } catch (std::exception &e) {
            logging::Log(logging::LogLevel::error)
                << logging::LOG_HEADER << "Process with id " << id
                << " failed with a generic exception: " << e.what();
          }
          m_nr_running--;
        } else {
          logging::Log(logging::LogLevel::error) << logging::LOG_HEADER << "Could not start process with id "
                              << id << ". Process is null or id is invalid";
        }
      }

      logging::Log(logging::LogLevel::debug) << logging::LOG_END;
    }

  } // namespace threadpool
}
