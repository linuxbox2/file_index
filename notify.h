// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab ft=cpp


#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <filesystem>

namespace file::listing {

  using namespace std::chrono_literals;
  
  class Notify
  {
    std::string& bucket_root;
  public:
    Notify(std::string& bucket_root)
      : bucket_root(bucket_root)
      {}

    virtual int add_watch(std::string& dname) = 0;
    virtual int remove_watch(std::string& dname) = 0;
    virtual ~Notify()
      {}
    
    
  }; /* Notify */

  class Inotify : public Notify
  {
    std::thread thrd;
    // vector of events?
    bool shutdown{false};

    void ev_loop() {
      while(! shutdown) {
	std::this_thread::sleep_for(200ms); // wait_for
      }
    }

  public:
    Inotify(std::string& bucket_root)
      : Notify(bucket_root)
      {}

    virtual int add_watch(std::string& dname) override {
      return 0;
    }

    virtual int remove_watch(std::string& dname) override {
      return 0;
    }

    ~Inotify() {
      thrd.join();
    }
  };

} // namespace file::listing
