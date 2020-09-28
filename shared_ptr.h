#pragma once

#include <control_block.h>

template <typename T>
struct weak_ptr;

template <typename T>
struct shared_ptr {
  // constructors
  constexpr shared_ptr() noexcept : control(nullptr), ptr(nullptr) {}

  constexpr explicit shared_ptr(std::nullptr_t) noexcept : shared_ptr() {}

  template <class Y>
  explicit shared_ptr(Y* p) : shared_ptr(p, std::default_delete<Y>()){}

  template <class Y, class Deleter>
  shared_ptr(Y* p, Deleter d) : control(new not_init_block<Y, Deleter>(p, d)), ptr(p) {
    increase_control();
  }

  template <class Deleter>
  shared_ptr(std::nullptr_t p, Deleter d) : control(nullptr), ptr(p) {}

  template <class Y>
  shared_ptr(const shared_ptr<Y>& r, T* p) noexcept : control(r.control), ptr(p) {
    increase_control();
  }

  template <class Y>
  shared_ptr(shared_ptr<Y>&& r, T* p) noexcept : shared_ptr() {
    r.swap(*this);
    ptr = p;
  }

  shared_ptr(const shared_ptr& r) noexcept : control(r.control), ptr(r.ptr) {
    increase_control();
  }

  template <class Y>
  shared_ptr(const shared_ptr<Y>& r) noexcept : control(r.control), ptr(r.ptr) {
    increase_control();
  }

  shared_ptr(shared_ptr&& r) noexcept : shared_ptr() {
    r.swap(*this);
  }

  template <class Y>
  explicit shared_ptr(const weak_ptr<Y>& r) : control(r.control), ptr(r.ptr) {
    increase_control();
  }

  // destructor
  ~shared_ptr() {
    if (control == nullptr) {
      return;
    }

    control->shared_counter--;
    if (control->shared_counter == 0) {
      control->delete_object();
      if (control->weak_counter == 0) {
        delete control;
      }
    }
  }

  // operator=
  shared_ptr& operator=(const shared_ptr& r) noexcept {
    if (*this == r) {
      return *this;
    }

    shared_ptr<T>(r).swap(*this);
    return *this;
  }

  template <class Y>
  shared_ptr& operator=(const shared_ptr<Y>& r) noexcept {
    shared_ptr(r).swap(*this);
    return *this;
  }

  shared_ptr& operator=(shared_ptr&& r) noexcept {
    shared_ptr<T>(std::move(r)).swap(*this);
    return *this;
  }

  template <class Y>
  shared_ptr& operator=(shared_ptr<Y>&& r) noexcept {
    shared_ptr<T>(std::move(r)).swap(*this);
    return *this;
  }

  // modifiers
  void reset() noexcept {
    shared_ptr().swap(*this);
  }

  template <class Y>
  void reset(Y* p) {
    return reset(p, std::default_delete<Y>());
  }

  template <class Y, class Deleter>
  void reset(Y* p, Deleter d) {
    shared_ptr<T>(p, std::move(d)).swap(*this);
  }

  void swap(shared_ptr& r) noexcept {
    std::swap(control, r.control);
    std::swap(ptr, r.ptr);
  }

  // observers
  T* get() const noexcept {
    return ptr;
  }

  T& operator*() const noexcept {
    return *ptr;
  }

  T* operator->() const noexcept {
    return ptr;
  }

  T& operator[](std::ptrdiff_t idx) const {
    return ptr[idx];
  }

  size_t use_count() const noexcept {
    return control == nullptr ? 0 : control->shared_counter;
  }

  explicit operator bool() const noexcept {
    return ptr != nullptr;
  }

 private:
  void increase_control() {
    if (control != nullptr) {
      control->shared_counter++;
    }
  }


  template <typename Y>
  friend class weak_ptr;

  template <typename Y>
  friend class shared_ptr;


  control_block* control;
  T* ptr;
};

// not member functions
template <class T, class... Args>
shared_ptr<T> make_shared(Args&&... args) {
  return shared_ptr<T>(new T(std::forward<Args>(args)...));
}

template <class T, class U>
bool operator==(const shared_ptr<T>& lhs, const shared_ptr<U>& rhs ) noexcept {
  return lhs.get() == rhs.get();
}

template <class T, class U>
bool operator!=(const shared_ptr<T>& lhs, const shared_ptr<U>& rhs) noexcept {
  return !(lhs == rhs);
}

template <class T>
bool operator==(const shared_ptr<T>& lhs, std::nullptr_t) noexcept {
  return !lhs;
}

template <class T>
bool operator==(std::nullptr_t, const shared_ptr<T>& rhs) noexcept {
  return !rhs;
}

template <class T>
bool operator!=(const shared_ptr<T>& lhs, std::nullptr_t) noexcept {
  return static_cast<bool>(lhs);
}

template< class T >
bool operator!=(std::nullptr_t, const shared_ptr<T>& rhs ) noexcept {
  return static_cast<bool>(rhs);
}


template <typename T>
struct weak_ptr {
  // constructors
  constexpr weak_ptr() noexcept : control(nullptr), ptr(nullptr) {}

  weak_ptr(const weak_ptr& r) noexcept : control(r.control), ptr(r.ptr) {
    increase_control();
  }

  template <class Y>
  weak_ptr(const weak_ptr<Y>& r) noexcept : control(r.control), ptr(r.ptr) {
    increase_control();
  }

  template <class Y>
  weak_ptr(const shared_ptr<Y>& r) noexcept : control(r.control), ptr(r.ptr) {
    increase_control();
  }

  weak_ptr(weak_ptr&& r) noexcept : weak_ptr() {
    r.swap(*this);
  }

  template <class Y>
  weak_ptr(weak_ptr<Y>&& r) noexcept : weak_ptr() {
    r.swap(*this);
  }

  // destructor
  ~weak_ptr()  {
    if (control == nullptr) {
      return;
    }

    control->weak_counter--;
    if (control->shared_counter == 0 && control->weak_counter == 0) {
        delete control;
    }
  }

  // operator=
  weak_ptr& operator=(const weak_ptr& r) noexcept {
    weak_ptr<T>(r).swap(*this);
    return *this;
  }

  template <class Y>
  weak_ptr& operator=(const weak_ptr<Y>& r) noexcept {
    weak_ptr<T>(r).swap(*this);
    return *this;
  }

  template<class Y>
  weak_ptr& operator=(const shared_ptr<Y>& r) noexcept {
    weak_ptr<T>(r).swap(*this);
    return *this;
  }

  weak_ptr& operator=(weak_ptr&& r) noexcept {
    weak_ptr<T>(std::move(r)).swap(*this);
    return *this;
  }

  template<class Y>
  weak_ptr& operator=(weak_ptr<Y>&& r) noexcept {
    weak_ptr<T>(std::move(r)).swap(*this);
    return *this;
  }

  // modifiers
  void reset() noexcept {
    if (control == nullptr) {
      return;
    }
    control->weak_counter--;
    control = nullptr;
  }

  void swap(weak_ptr& r) noexcept {
    std::swap(control, r.control);
    std::swap(ptr, r.ptr);
  }

  // observers
  size_t use_count() const noexcept {
    return control == nullptr ? 0 : control->shared_counter;
  }

  bool expired() const noexcept {
    return use_count() == 0;
  }

  shared_ptr<T> lock() const noexcept {
    return expired() ? shared_ptr<T>() : shared_ptr<T>(*this);
  }

 private:
  void increase_control() {
    if (control != nullptr) {
      control->weak_counter++;
    }
  }

  template <typename Y>
  friend class weak_ptr;

  template <typename Y>
  friend class shared_ptr;

  control_block* control;
  T* ptr;
};
