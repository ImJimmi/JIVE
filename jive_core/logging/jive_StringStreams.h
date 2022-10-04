#pragma once

namespace juce
{
    String& operator<<(String& str, const Image& image);
    String& operator<<(String& str, const var& value);

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
} // namespace juce
