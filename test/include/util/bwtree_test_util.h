#pragma once
#include "bwtree/bwtree.h"
#include "xxHash/xxh3.h"

struct BigKey {
  uint64_t data_[1];
};

namespace std {

template <>
struct hash<BigKey> {
  /**
   * @param key key to be hashed
   * @return hash of the key's underlying data
   */
  size_t operator()(const BigKey &key) const {
    // you're technically hashing more bytes than you need to, but hopefully key size isn't wildly over-provisioned
    return static_cast<size_t>(XXH3_64bits(reinterpret_cast<const void *>(key.data_), sizeof(uint64_t)));
  }
};

template <>
struct less<BigKey> {
  /**
   * Due to the KeySize constraints, this should be optimized to a single SIMD instruction.
   * @param lhs first key to be compared
   * @param rhs second key to be compared
   * @return true if first key is less than the second key
   */
  bool operator()(const BigKey &lhs, const BigKey &rhs) const { return std::memcmp(lhs.data_, rhs.data_, sizeof(uint64_t)) < 0; }
};

template <>
struct equal_to<BigKey> {
  /**
   * @param lhs first key to be compared
   * @param rhs second key to be compared
   * @return true if first key is equal to the second key
   */
  bool operator()(const BigKey &lhs, const BigKey &rhs) const {
    // you're technically comparing more bytes than you need to, but hopefully key size isn't wildly over-provisioned
    return std::memcmp(lhs.data_, rhs.data_, sizeof(uint64_t)) == 0;
  }
};
}  // namespace std

namespace terrier {
/**
 * Normally we restrict the scope of util files to named directories/namespaces (storage, transaction, etc.) but the
 * BwTree is a unique case because it's third party code but we want to test it rigorously. This isn't a
 * third_party_test_util because directories/files with "third_party" in their name are treated differently by the CI
 * scripts.
 */

struct BwTreeTestUtil {
  BwTreeTestUtil() = delete;

  using TreeType = third_party::bwtree::BwTree<BigKey, int64_t>;

  /**
   * Adapted from https://github.com/wangziqi2013/BwTree/blob/master/test/test_suite.cpp
   */
  static TreeType *GetEmptyTree() {
    auto *tree = new TreeType{true};

    // By default let is serve single thread (i.e. current one)
    // and assign gc_id = 0 to the current thread
    tree->UpdateThreadLocal(1);
    tree->AssignGCID(0);
    return tree;
  }
};

}  // namespace terrier
