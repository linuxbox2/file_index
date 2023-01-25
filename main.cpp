// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab ft=cpp

#include "bucket_cache.h"
#include <stdint.h>

namespace {
  string bucket_root = "bucket_root";
  string database_root = "lmdb_root";
  uint32_t max_buckets = 100;
  uint8_t max_lanes = 1;
  uint8_t lmdb_count = 3;
  string bucket1_name = "stanley";
  string bucket1_marker = ""; // start at the beginning
} // anonymous ns

int main (int argc, char *argv[])
{
    BucketCache bc{bucket_root, database_root, max_buckets, max_lanes, lmdb_count};
    bc.list_bucket(bucket1_name, bucket1_marker); // XXX need a lambda to handle the output
}
