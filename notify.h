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
#include <limits>
#include <cstdlib>
#include "unordered_dense.h"
#include <unistd.h>
#include <poll.h>
#ifdef linux
#include <sys/inotify.h>
#include <sys/eventfd.h>
#endif
#undef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY 1
#include <fmt/format.h>

namespace file::listing {

  using namespace std::chrono_literals;
  namespace sf = std::filesystem;

  template<typename T>
  class Notifiable
  {
    enum class EventType : uint8_t
    {
      ADD = 0,
      REMOVE,
      INVALIDATE
    };

    struct Event
    {
      EventType type;
      std::optional<std::string_view> name;
    };
    
    virtual int notify(std::vector<Event>) = 0;
  };

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
    
    static constexpr uint32_t rd_size = 8192;
    static constexpr uint64_t sig_shutdown = std::numeric_limits<uint64_t>::max() - 0xdeadbeef;
    static constexpr uint32_t aw_mask = IN_MOVE|IN_DONT_FOLLOW|IN_ONLYDIR|IN_MASK_ADD;

    int fd, efd;
    std::thread thrd;
    wd_map_t wd_map;
    bool shutdown{false};

    class AlignedBuf
    {
      char* m;
    public:
      AlignedBuf() {
	m = static_cast<char*>(aligned_alloc(__alignof__(struct inotify_event), rd_size));
	if (! m) [[unlikely]] {
	  std::cerr << fmt::format("{} buffer allocation failure", __func__) << std::endl;
	  abort();
	}
      }
      ~AlignedBuf() {
	std::free(m);
      }
      char* get() {
	return m;
      }
    }; /* AlignedBuf */
    
    void ev_loop() {
      std::unique_ptr<AlignedBuf> up_buf = std::make_unique<AlignedBuf>();
      struct inotify_event* event;
      char* buf = up_buf.get()->get();
      ssize_t len;
      int n;

      nfds_t nfds{2};
      struct pollfd fds[2] = {{fd, POLLIN}, {efd, POLLIN}};

      while(! shutdown) {
	n = poll(fds, nfds, -1); /* for up to 10 fds, poll is fast as epoll */
	if (shutdown) {
	  return;
	}
	if (n == -1) {
	  if (errno = EINTR) {
	    continue;
	  }
	  // XXX
	}
	if (n > 0) {
	  len = read(fd, buf, rd_size);
	  if (len == -1) {
	    continue; // hopefully, was EAGAIN
	  }
	  for (char* ptr = buf; ptr < buf + len;
	       ptr += sizeof(struct inotify_event) + event->len) {
	    event = reinterpret_cast<struct inotify_event*>(ptr);
	    if (event->mask & IN_Q_OVERFLOW) [[unlikely]] {
	      /* cache blown, invalidate */
	      /* TODO: implement */
	    } else {
	      if ((event->mask & IN_CREATE) ||
		  (event->mask & IN_MOVED_TO)) {
		/* new object in dir */
	      } else if ((event->mask & IN_DELETE) ||
			 (event->mask & IN_MOVED_FROM)) {
		/* object removed from dir */
	      
	      }
	    } /* !overflow */
	  } /* events */
	} /* n > 0 */
      }
    } /* ev_loop */

    Inotify(std::string& bucket_root)
      : Notify(bucket_root)
      {
	fd = inotify_init1(IN_NONBLOCK);
	if (fd == -1) {
	  std::cerr << fmt::format("{} inotify_init1 failed with {}", __func__, fd) << std::endl;
	  exit(1);
	}
	efd = eventfd(0, EFD_NONBLOCK);
      }

    void signal_shutdown() {
      uint64_t msg{sig_shutdown};
      (void) write(efd, &msg, sizeof(uint64_t));
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
      shutdown = true;
      signal_shutdown();
      thrd.join();
    }
  };
#endif /* linux */

} // namespace file::listing
