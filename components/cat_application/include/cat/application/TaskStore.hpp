
#pragma once

#include <cstddef>

namespace cat::application {

struct Task {
    int id;
    const char* title;
    const char* date;
    const char* time;
    bool completed;
    bool carried;
};

class TaskStore final {
public:
    static TaskStore& instance() noexcept;

    const Task* tasks() const noexcept;

    std::size_t count() const noexcept;

    bool toggle_completed(int task_id) noexcept;

private:
    TaskStore();

    Task tasks_[6];
};

} // namespace cat::application
