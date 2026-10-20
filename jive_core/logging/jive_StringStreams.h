#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace juce
{
    // LCOV_EXCL_START
    String& operator<<(String& str, Colour colour);
    String& operator<<(String& str, const Font& font);
    String& operator<<(String& str, const Image& image);
    String& operator<<(String& str, const var& value);
    String& operator<<(String& str, Time time);
    String& operator<<(String& str, long double value); // NOLINT
    String& operator<<(String& str, RelativeTime relativeTime);
    String& operator<<(String& str, juce::Grid::Px px);
    String& operator<<(String& str, const juce::Grid::TrackInfo& track);
    String& operator<<(String& str, const juce::ValueTree& tree);

    template <typename Value>
    String& operator<<(String& str, Point<Value> point)
    {
        return str << "juce::Point<" << typeid(Value).name() << "> { "
                   << point.getX()
                   << ", "
                   << point.getY()
                   << " }";
    }
    template <typename Value>
    String& operator<<(String& str, const Line<Value>& line)
    {
        return str << "juce::Line<" << typeid(Value).name() << "> { "
                   << line.getStart()
                   << ", "
                   << line.getEnd()
                   << " }";
    }

    template <typename Value>
    String& operator<<(String& str, Rectangle<Value> rect)
    {
        return str << "juce::Rectangle<" << typeid(Value).name() << "> { "
                   << rect.getX()
                   << ", " << rect.getY()
                   << ", " << rect.getWidth()
                   << ", " << rect.getHeight()
                   << " }";
    }

    template <typename Value>
    String& operator<<(String& str, const Array<Value>& array)
    {
        str << "juce::Array<" << typeid(Value).name() << "> { ";

        for (const auto& value : array)
            str << value << ", ";

        str << " }";
        return str;
    }

    template <typename Value>
    String& operator<<(String& str, const std::optional<Value>& optional)
    {
        str << "std::optional<" << typeid(Value).name() << "> { ";

        if (optional.has_value())
            str << *optional;
        else
            str << "NULL";

        return str << " }";
    }

    template <typename Value>
    String& operator<<(String& str, const juce::BorderSize<Value>& border)
    {
        return str << "juce::BorderSize<"
                   << typeid(Value).name()
                   << "> { "
                   << border.getTop() << ", "
                   << border.getLeft() << ", "
                   << border.getBottom() << ", "
                   << border.getRight()
                   << " }";
    }
    // LCOV_EXCL_STOP
} // namespace juce
