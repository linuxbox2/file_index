// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab ft=cpp

#include "bucket_cache.h"
#include <filesystem>
#include <string>
#include <stdint.h>
#include <gtest/gtest.h>

namespace {
  std::string bucket_root = "bucket_root";
  std::string database_root = "lmdb_root";
  uint32_t max_buckets = 100;
  uint8_t max_lanes = 1;
  uint8_t lmdb_count = 3;
  std::string bucket1_name = "stanley";
  std::string bucket1_marker = ""; // start at the beginning
} // anonymous ns

TEST(BucketCache, SetupTDir1)
{
}

TEST(BucketCache, ListTDir1)
{
  BucketCache bc{bucket_root, database_root, max_buckets, max_lanes, lmdb_count};
  bc.list_bucket(bucket1_name, bucket1_marker); // XXX need a lambda to handle the output
}

int main (int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
