// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab ft=cpp

#include "notify.h"
#ifdef linux
#include <sys/inotify.h>
#endif

namespace file::listing {

  std::unique_ptr<Notify> Notify::factory(Notifiable* c, std::string& bucket_root)
  {
#ifdef linux
    return std::unique_ptr<Notify>(new Inotify(c, bucket_root));
#endif /* linux */
    return nullptr;
  } /* Notify::factory */

} // namespace file::listing
