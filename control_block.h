#pragma once

struct control_block {
  size_t shared_counter = 0;
  size_t weak_counter = 0;

  virtual void delete_object() = 0;
  virtual ~control_block() = default;
};

template <typename T, typename Deleter>
struct not_init_block : control_block, Deleter {
  T* ptr;

  not_init_block(T* p, Deleter d) : ptr(p), Deleter(std::move(d)) {}

  void delete_object() override {
    static_cast<Deleter&>(*this)(ptr);
  }
};

template <typename T>
struct init_block : control_block{
  typename std::aligned_storage<sizeof(T), alignof(T)> data;

  template <typename ...Args>
  explicit init_block(Args&& ...args) {
    new (&data) T(std::forward<Args>(args)...);
  }

  void delete_object() override {
    reinterpret_cast<T*>(&data)->~T();
  }
};
