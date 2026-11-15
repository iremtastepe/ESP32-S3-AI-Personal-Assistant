
#pragma once

namespace cat::domain {
class CatState;
}

namespace cat::application {

class TaskService final {
public:
    void reward_cat_for_completed_task(
        cat::domain::CatState& cat
    ) const noexcept;
};

}
