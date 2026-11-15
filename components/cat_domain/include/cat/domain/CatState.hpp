
#pragma once

#include <cstdint>

namespace cat::domain {

enum class CatActivity : std::uint8_t {
    Sleeping,
    Idle,
    Eating,
    Playing
};

class CatState final {
public:
    CatActivity activity() const noexcept;
    void set_activity(CatActivity activity) noexcept;

private:
    CatActivity activity_{CatActivity::Sleeping};
};

}
