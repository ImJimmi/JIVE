#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace juce
{
    // LCOV_EXCL_START
    String& operator<<(String& str, const Colour& colour);
    String& operator<<(String& str, const Font& font);
    String& operator<<(String& str, const Image& image);
    String& operator<<(String& str, const var& value);
    String& operator<<(String& str, const Time& time);
    String& operator<<(String& str, long double value);
    String& operator<<(String& str, RelativeTime relativeTime);
    String& operator<<(String& str, juce::Grid::Px px);
    String& operator<<(String& str, juce::Grid::TrackInfo track);

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
    String& operator<<(String& str, const Line<T>& line)
    {
        return str << "juce::Line<" << typeid(T).name() << "> { "
                   << line.getStart()
                   << ", "
                   << line.getEnd()
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
    String& operator<<(String& str, const Array<T>& array)
    {
        str << "juce::Array<" << typeid(T).name() << "> { ";

        for (const auto& value : array)
            str << value << ", ";

        str << " }";
        return str;
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

    template <typename T>
    String& operator<<(String& str, const juce::BorderSize<T>& border)
    {
        return str << "juce::BorderSize<"
                   << typeid(T).name()
                   << "> { "
                   << border.getTop() << ", "
                   << border.getLeft() << ", "
                   << border.getBottom() << ", "
                   << border.getRight()
                   << " }";
    }
    // LCOV_EXCL_STOP
} // namespace juce
