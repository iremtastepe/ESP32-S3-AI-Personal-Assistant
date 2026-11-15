
#include "cat/application/TaskService.hpp"
#include "cat/domain/CatState.hpp"

namespace cat::application {

void TaskService::reward_cat_for_completed_task(
    cat::domain::CatState& cat
) const noexcept
{
    cat.set_activity(cat::domain::CatActivity::Eating);
}

}
