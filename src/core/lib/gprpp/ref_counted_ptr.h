/*
 *
 * Copyright 2017 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef GRPC_CORE_LIB_GPRPP_REF_COUNTED_PTR_H
#define GRPC_CORE_LIB_GPRPP_REF_COUNTED_PTR_H

#include <grpc/support/port_platform.h>

#include <utility>

#include "src/core/lib/gprpp/memory.h"

namespace grpc_core {

// A smart pointer class for objects that provide IncrementRefCount() and
// Unref() methods, such as those provided by the RefCounted base class.
template <typename T>
class RefCountedPtr {
 public:
  RefCountedPtr() {}
  RefCountedPtr(std::nullptr_t) {}

  // If value is non-null, we take ownership of a ref to it.
  template <typename Y>
  explicit RefCountedPtr(Y* value) {
    value_ = value;
  }

  // Move ctors.
  RefCountedPtr(RefCountedPtr&& other) {
    value_ = other.value_;
    other.value_ = nullptr;
  }
  template <typename Y>
  RefCountedPtr(RefCountedPtr<Y>&& other) {
    value_ = other.value_;
    other.value_ = nullptr;
  }

  // Move assignment.
  RefCountedPtr& operator=(RefCountedPtr&& other) {
    if (value_ != nullptr) value_->Unref();
    value_ = other.value_;
    other.value_ = nullptr;
    return *this;
  }
  template <typename Y>
  RefCountedPtr& operator=(RefCountedPtr<Y>&& other) {
    if (value_ != nullptr) value_->Unref();
    value_ = other.value_;
    other.value_ = nullptr;
    return *this;
  }

  // Copy ctors.
  RefCountedPtr(const RefCountedPtr& other) {
    if (other.value_ != nullptr) other.value_->IncrementRefCount();
    value_ = other.value_;
  }
  template <typename Y>
  RefCountedPtr(const RefCountedPtr<Y>& other) {
    if (other.value_ != nullptr) other.value_->IncrementRefCount();
    value_ = other.value_;
  }

  // Copy assignment.
  RefCountedPtr& operator=(const RefCountedPtr& other) {
    // Note: Order of reffing and unreffing is important here in case value_
    // and other.value_ are the same object.
    if (other.value_ != nullptr) other.value_->IncrementRefCount();
    if (value_ != nullptr) value_->Unref();
    value_ = other.value_;
    return *this;
  }
  template <typename Y>
  RefCountedPtr& operator=(const RefCountedPtr<Y>& other) {
    // Note: Order of reffing and unreffing is important here in case value_
    // and other.value_ are the same object.
    if (other.value_ != nullptr) other.value_->IncrementRefCount();
    if (value_ != nullptr) value_->Unref();
    value_ = other.value_;
    return *this;
  }

  ~RefCountedPtr() {
    if (value_ != nullptr) value_->Unref();
  }

  // If value is non-null, we take ownership of a ref to it.
  template <typename Y>
  void reset(Y* value) {
    if (value_ != nullptr) value_->Unref();
    value_ = value;
  }

  void reset() {
    if (value_ != nullptr) value_->Unref();
    value_ = nullptr;
  }

  // TODO(roth): This method exists solely as a transition mechanism to allow
  // us to pass a ref to idiomatic C code that does not use RefCountedPtr<>.
  // Once all of our code has been converted to idiomatic C++, this
  // method should go away.
  T* release() {
    T* value = value_;
    value_ = nullptr;
    return value;
  }

  T* get() const { return value_; }

  T& operator*() const { return *value_; }
  T* operator->() const { return value_; }

  template <typename Y>
  bool operator==(const RefCountedPtr<Y>& other) const {
    return value_ == other.value_;
  }

  template <typename Y>
  bool operator==(const Y* other) const {
    return value_ == other;
  }

  bool operator==(std::nullptr_t) const { return value_ == nullptr; }

  template <typename Y>
  bool operator!=(const RefCountedPtr<Y>& other) const {
    return value_ != other.value_;
  }

  template <typename Y>
  bool operator!=(const Y* other) const {
    return value_ != other;
  }

  bool operator!=(std::nullptr_t) const { return value_ != nullptr; }

 private:
  template <typename Y>
  friend class RefCountedPtr;

  T* value_ = nullptr;
};

template <typename T, typename... Args>
inline RefCountedPtr<T> MakeRefCounted(Args&&... args) {
  return RefCountedPtr<T>(New<T>(std::forward<Args>(args)...));
}

}  // namespace grpc_core

#endif /* GRPC_CORE_LIB_GPRPP_REF_COUNTED_PTR_H */
