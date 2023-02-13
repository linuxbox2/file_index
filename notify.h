// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab ft=cpp

#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <filesystem>
#include "unordered_dense.h"
#ifdef linux
#include <sys/inotify.h>
#endif
#undef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY 1
#include <fmt/format.h>

namespace file::listing {

  using namespace std::chrono_literals;
  namespace sf = std::filesystem;
  
  class Notify
  {
    sf::path rp;

    Notify(std::string& bucket_root)
      : rp(bucket_root)
      {}

    friend class Inotify;
  public:
    static std::unique_ptr<Notify> factory(std::string& bucket_root);
    
    virtual int add_watch(const std::string& dname) = 0;
    virtual int remove_watch(const std::string& dname) = 0;
    virtual ~Notify()
      {}
  }; /* Notify */

#ifdef linux
  class Inotify : public Notify
  {

    using wd_map_t = ankerl::unordered_dense::map<std::string, int>;
    static constexpr uint32_t aw_mask = IN_MOVE|IN_DONT_FOLLOW|IN_ONLYDIR|IN_MASK_ADD;

    int fd;
    std::thread thrd;
    wd_map_t wd_map;
    // vector of events?
    bool shutdown{false};

    void ev_loop() {
      while(! shutdown) {
	std::this_thread::sleep_for(200ms); // wait_for
      }
    }

    Inotify(std::string& bucket_root)
      : Notify(bucket_root)
      {
	fd = inotify_init1(0);
	if (fd == -1) {
	  std::cerr << fmt::format("{} inotify_init1 failed with {}", __func__, fd) << std::endl;
	  exit(1);
	}
      }

    friend class Notify;
  public:
    virtual int add_watch(const std::string& dname) override {
      sf::path wp{rp / dname};
      int wd = inotify_add_watch(fd, wp.c_str(), aw_mask);
      if (wd == -1) {
	std::cerr << fmt::format("{} inotify_add_watch {} failed with {}", __func__, dname, wd) << std::endl;
      } else {
	wd_map.insert(wd_map_t::value_type(dname, wd));
      }
      return wd;
    }

    virtual int remove_watch(const std::string& dname) override {
      int r{0};
      const auto& elt = wd_map.find(dname);
      if (elt != wd_map.end()) {
	auto& wd = elt->second;
	r = inotify_rm_watch(fd, wd);
	if (r == -1) {
	  std::cerr << fmt::format("{} inotify_rm_watch {} failed with {}", __func__, dname, wd) << std::endl;
	}
      }
      return r;
    }

    virtual ~Inotify() {
      thrd.join();
    }
  };
#endif /* linux */

} // namespace file::listing
