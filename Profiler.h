#include <chrono>
#include <functional>
#include <iostream>
#include <type_traits>
#include <utility>


class RAIIProfiler {
  public:
    RAIIProfiler() { start = std::chrono::system_clock::now(); }
    ~RAIIProfiler() {
        auto duration = std::chrono::system_clock::now() - start;
        std::cout << "Time taken: "
                  << std::chrono::duration_cast<std::chrono::minutes>(duration)
                         .count()
                  << " min(s), "
                  << std::chrono::duration_cast<std::chrono::seconds>(duration)
                         .count()
                  << " sec(s), "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(
                         duration)
                         .count()
                  << " millisec(s).\n";
    }

  private:
    std::chrono::system_clock::time_point start;
};

template <class _Fn, class... _Args>
std::invoke_result_t<_Fn, _Args...>
Profile(_Fn &&f,
        _Args &&...args) noexcept(std::is_nothrow_invocable_v<_Fn, _Args...>) {
    RAIIProfiler profiler;
    return std::invoke(std::forward<_Fn>(f), std::forward<_Args>(args)...);
}