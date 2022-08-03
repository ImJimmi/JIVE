#pragma once

namespace juce
{
    template <typename T>
    JUCE_API String& JUCE_CALLTYPE operator<<(String& str, const Rectangle<T>& rect)
    {
        return str << "juce::Rectangle<" << typeid(T).name() << "> { "
                   << rect.getX()
                   << ", " << rect.getY()
                   << ", " << rect.getWidth()
                   << ", " << rect.getHeight()
                   << " }";
    }

    template <typename T>
    JUCE_API String& JUCE_CALLTYPE operator<<(String& str, const Point<T>& point)
    {
        return str << "juce::Point<" << typeid(T).name() << "> { "
                   << point.getX()
                   << ", "
                   << point.getY()
                   << " }";
    }

    JUCE_API String& JUCE_CALLTYPE operator<<(String& str, const Image& image)
    {
        return str << "juce::Image { "
                   << image.getWidth()
                   << ", "
                   << image.getHeight()
                   << " }";
    }
} // namespace juce
