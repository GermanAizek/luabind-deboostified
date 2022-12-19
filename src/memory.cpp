#define LUABIND_BUILDING

#include <luabind/config.hpp>

namespace luabind
{
    allocator_func allocator = nullptr;
    void* allocator_context = nullptr;
}
