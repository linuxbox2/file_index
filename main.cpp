// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab ft=cpp

#include "bucket_cache.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <string_view>
#include <random>
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
} // anonymous ns

namespace sf = std::filesystem;

TEST(BucketCache, SetupTDir1)
{
  sf::path tp{sf::path{bucket_root} / tdir1};
  sf::remove_all(tp);
  sf::create_directory(tp);

  /* generate 1M unique files in random order */
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
  } /* for 1M */
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

TEST(BucketCache, TearDownBucketCache) 
{
  delete bc;
  bc = nullptr;
}

int main (int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
