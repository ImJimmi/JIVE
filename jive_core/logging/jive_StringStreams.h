#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace juce
{
    String& operator<<(String& str, const Colour& colour);
    String& operator<<(String& str, const Font& font);
    String& operator<<(String& str, const Image& image);
    String& operator<<(String& str, const var& value);
    String& operator<<(String& str, const Time& time);
    String& operator<<(String& str, long double value);
    String& operator<<(String& str, RelativeTime relativeTime);

    template <typename T>
    String& operator<<(String& str, const Point<T>& point)
    {
        return str << "juce::Point<" << typeid(T).name() << "> { "
                   << point.getX()
                   << ", "
                   << point.getY()
                   << " }";
    }

    template <typename T>
    String& operator<<(String& str, const Rectangle<T>& rect)
    {
        return str << "juce::Rectangle<" << typeid(T).name() << "> { "
                   << rect.getX()
                   << ", " << rect.getY()
                   << ", " << rect.getWidth()
                   << ", " << rect.getHeight()
                   << " }";
    }

    template <typename T>
    String& operator<<(String& str, const std::optional<T>& optional)
    {
        str << "std::optional<" << typeid(T).name() << "> { ";

        if (optional.has_value())
            str << *optional;
        else
            str << "NULL";

        return str << " }";
    }
} // namespace juce
