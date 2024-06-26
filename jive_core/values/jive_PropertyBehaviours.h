#pragma once

namespace jive
{
    enum class Inheritance
    {
        inheritFromParent,
        inheritFromAncestors,
        doNotInherit,
    };

    enum class Accumulation
    {
        accumulate,
        doNotAccumulate,
    };

    enum class Responsiveness
    {
        respondToChanges,
        ignoreChanges
    };
} // namespace jive
