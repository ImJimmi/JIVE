#pragma once

namespace jive
{
    template <class... Variants>
    struct Visitor : Variants...
    {
        using Variants::operator()...;
    };

    template <class... Variants>
    Visitor(Variants...) -> Visitor<Variants...>;
} // namespace jive
