#include "gtest/gtest.h"
#include <iostream>
#include <thread>

void thread_task() {
    std::cout << "hello thread. Thread Id is "
              << std::this_thread::get_id() << std::endl;
}

TEST(hello_socket, demo) {
    std::thread t(thread_task);
    t.join();
}

// As with much of the C++ standard library, std::thread works with any callable
// type, so you can pass an instance of a class with a function call operator
// to the std::thread constructor instead:

namespace {
void print_hello() { std::cout << "hello!\n"; }

class background_task {
public:
  void operator()() const {
    print_hello();
  }
};

class thread_guard {
private:
  std::thread &t_;

public:
  explicit thread_guard(std::thread &t) : t_(t) {}
  ~thread_guard() {
    if (t_.joinable()) {
      std::cout << "thread_guard Destructor.\n";
      t_.join();
    }
  }

  thread_guard(thread_guard const &) = delete;
  thread_guard &operator=(thread_guard const &) = delete;
};

struct func {
  int &i_;
  explicit func(int &i) : i_(i) {}
  void operator()() {
    for (unsigned j = 0; j < 100; ++j)
      print_hello();
  }
};

}

TEST(thread_management, functor_test) {
  background_task f;
  std::vector<std::thread> thread_pool;

  // 这里用花括号，用小括号，有点类似于函数调用，不是很清晰
  thread_pool.emplace_back(std::thread{f});
  // using lambda expression instead of functor/function.
  thread_pool.emplace_back(std::thread{[] { print_hello(); }});

  for (auto &t : thread_pool)
    t.join();

  // Once you've called join(), the std::thread object is no longer joinable,
  // and joinable() will return false.
  for (auto &t : thread_pool)
    std::cout << (t.joinable() ? "TRUE" : "FALSE") << ' ';
  std::cout << '\n';
}

TEST(thread_management, raii_test) {
  // One way of doing this is to use the standard Resource Acquisition Is
  // Initialization (RAII) idiom and provide a class that does the join() in
  // its destructor.

  int some_local_state = 0;
  func my_func(some_local_state);
  std::thread t{my_func};

  thread_guard g{t};
}

namespace thread_detach {
// Calling detach() on a std::thread object leaves the thread to run in the
// background, with no direct means of communicating with it. It's no longer
// possible to wait for that thread to complete; if a thread becomes detached,
// it isn't possible to obtain a std::thread object that references it, so it
// can no longer be joined. Detached threads truly run in the background;
// ownership and control are passed over to the C++ Runtime Library, which
// ensures that the resources associated with the thread are correctly
// reclaimed when the thread exits.
// Detached threads are often called daemon threads after the UNIX concept of
// a daemon process that runs in the background without any explicit user
// interface.

bool done_editing() { return true; }

enum command_type {
  open_new_document,
  command_max
};

struct user_command {
  command_type type = open_new_document;
  int operators = 1;
};

user_command get_user_input() { return user_command{}; }

void edit_document(std::string const &filename) {
  // open_document_and_display_gui(filename)

#if 0
  while (!done_editing()) {
    auto cmd = get_user_input();
    if (cmd.type == )

    if

  }
#endif

}

}