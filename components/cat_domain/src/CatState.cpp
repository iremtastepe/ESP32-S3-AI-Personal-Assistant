
#include "cat/domain/CatState.hpp"

namespace cat::domain {

CatActivity CatState::activity() const noexcept
{
    return activity_;
}

void CatState::set_activity(CatActivity activity) noexcept
{
    activity_ = activity;
}

}
