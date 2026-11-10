#pragma once

namespace jive
{
    // Calls the callable for each of the given input values.
    // The values can each be of a different type, the callable just needs to
    // support different input types, e.g. [](auto&){ ... }.
    template <typename Callable, typename... Value>
    void forEach(Callable&& callable, Value&... values)
    {
        (callable(values), ...);
    }
} // namespace jive
