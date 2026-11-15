
#include "cat/application/TaskStore.hpp"

namespace cat::application {

TaskStore::TaskStore()
    : tasks_{
        {1, "Ders çalış", "2026-11-10", "09:00", false, false},
        {2, "Kitap oku", "2026-11-10", "14:30", true, false},
        {3, "Projeye devam et", "2026-11-10", nullptr, false, false},
        {4, "İngilizce çalış", "2026-11-11", nullptr, false, false},
        {5, "Alışveriş yap", "2026-11-11", nullptr, false, false},
        {6, "Proje sunumu hazırla", "2026-11-17", nullptr, false, false}
    }
{
}

TaskStore& TaskStore::instance() noexcept
{
    static TaskStore store;
    return store;
}

const Task* TaskStore::tasks() const noexcept
{
    return tasks_;
}

std::size_t TaskStore::count() const noexcept
{
    return 6;
}

bool TaskStore::toggle_completed(int task_id) noexcept
{
    for (Task& task : tasks_) {
        if (task.id == task_id) {
            task.completed = !task.completed;
            return true;
        }
    }

    return false;
}

} // namespace cat::application
