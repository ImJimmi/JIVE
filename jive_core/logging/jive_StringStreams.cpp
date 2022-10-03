#pragma once

namespace juce
{
    String& operator<<(String& str, const Image& image)
    {
        return str << "juce::Image { "
                   << image.getWidth()
                   << ", "
                   << image.getHeight()
                   << " }";
    }

    String& operator<<(String& str, const var& value)
    {
        return str << "juce::var{ " << value.toString() << " }";
    }
} // namespace juce
