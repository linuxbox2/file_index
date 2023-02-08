// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab ft=cpp

#include "bucket_cache.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <string_view>
#include <random>
#include <ranges>
#include <stdint.h>

#undef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY 1
#include <fmt/format.h>

#include <gtest/gtest.h>

namespace {
  std::string bucket_root = "bucket_root";
  std::string database_root = "lmdb_root";
  uint32_t max_buckets = 100;
  uint8_t max_lanes = 1;
  uint8_t lmdb_count = 3;
  std::string bucket1_name = "stanley";
  std::string bucket1_marker = ""; // start at the beginning

  std::random_device rd;
  std::mt19937 mt(rd());

  std::string tdir1{"tdir1"};
  std::uniform_int_distribution<> dist_1m(1, 1000000);
  BucketCache* bc{nullptr};
  std::vector<std::string> bvec;
} // anonymous ns

namespace sf = std::filesystem;

TEST(BucketCache, SetupTDir1)
{
  sf::path tp{sf::path{bucket_root} / tdir1};
  sf::remove_all(tp);
  sf::create_directory(tp);

  /* generate 100K unique files in random order */
  std::string fbase{"file_"};
  for (int ix = 0; ix < 100000; ++ix) {
  retry:
    auto n = dist_1m(mt);
    sf::path ttp{tp / fmt::format("{}{}", fbase, n)};
    if (sf::exists(ttp)) {
      goto retry;
    } else {
      std::ofstream ofs(ttp);
      ofs << "data for " << ttp << std::endl;
      ofs.close();
    }
  } /* for 100K */
} /* SetupTDir1 */

TEST(BucketCache, InitBucketCache)
{
  bc = new BucketCache{bucket_root, database_root, max_buckets, max_lanes, lmdb_count};
}

TEST(BucketCache, ListTDir1)
{
  bc->list_bucket(tdir1, bucket1_marker); // XXX need a lambda to handle the output
}

TEST(BucketCache, ListTDir2)
{
  bc->list_bucket(tdir1, bucket1_marker); // XXX need a lambda to handle the output
}

TEST(BucketCache, ListTDir3)
{
  bc->list_bucket(tdir1, bucket1_marker); // XXX need a lambda to handle the output
}

TEST(BucketCache, SetupRecycle1)
{
  int nbuckets = 5;
  int nfiles = 10;

  bvec = [&]() {
    std::vector<std::string> v;
    for (int ix = 0; ix < nbuckets; ++ix) {
      v.push_back(fmt::format("recyle_{}", ix));
    }
    return v;
  }();

  for (auto& bucket : bvec) {
    sf::path tp{sf::path{bucket_root} / bucket};
    sf::remove_all(tp);
    sf::create_directory(tp);

    std::string fbase{"file_"};
    for (int ix = 0; ix < nfiles; ++ix) {
    retry:
      auto n = dist_1m(mt);
      sf::path ttp{tp / fmt::format("{}{}", fbase, n)};
      if (sf::exists(ttp)) {
	goto retry;
      } else {
	std::ofstream ofs(ttp);
	ofs << "data for " << ttp << std::endl;
	ofs.close();
      }
    } /* for buckets */
  }
} /* SetupTDir1 */

TEST(BucketCache, InitBucketCacheRecycle1)
{
  bc = new BucketCache{bucket_root, database_root, 1, 1, 1};
}

TEST(BucketCache, ListNRecycle1)
{
  /* the effect is to allocate a Bucket cache entry once, then recycle n-1 times */
  for (auto& bucket : bvec) {
    bc->list_bucket(bucket, bucket1_marker);
  }
  ASSERT_EQ(bc->recycle_count, 4);
}

TEST(BucketCache, TearDownBucketCacheRecycle1)
{
  delete bc;
  bc = nullptr;
}

int main (int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
