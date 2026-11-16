#include "jive_StringStreams.h"

namespace juce
{
    // LCOV_EXCL_START
    String& operator<<(String& str, const Colour& colour)
    {
        // clang-format off
        static constexpr std::array knownColourNames{ "aliceblue","antiquewhite","aqua","aquamarine","azure","beige","bisque","black","blanchedalmond","blue","blueviolet","brown","burlywood","cadetblue","chartreuse","chocolate","coral","cornflowerblue","cornsilk","crimson","cyan","darkblue","darkcyan","darkgoldenrod","darkgrey","darkgreen","darkkhaki","darkmagenta","darkolivegreen","darkorange","darkorchid","darkred","darksalmon","darkseagreen","darkslateblue","darkslategrey","darkturquoise","darkviolet","deeppink","deepskyblue","dimgrey","dodgerblue","firebrick","floralwhite","forestgreen","fuchsia","gainsboro","ghostwhite","gold","goldenrod","grey","green","greenyellow","honeydew","hotpink","indianred","indigo","ivory","khaki","lavender","lavenderblush","lawngreen","lemonchiffon","lightblue","lightcoral","lightcyan","lightgoldenrodyellow","lightgreen","lightgrey","lightpink","lightsalmon","lightseagreen","lightskyblue","lightslategrey","lightsteelblue","lightyellow","lime","limegreen","linen","magenta","maroon","mediumaquamarine","mediumblue","mediumorchid","mediumpurple","mediumseagreen","mediumslateblue","mediumspringgreen","mediumturquoise","mediumvioletred","midnightblue","mintcream","mistyrose","moccasin","navajowhite","navy","oldlace","olive","olivedrab","orange","orangered","orchid","palegoldenrod","palegreen","paleturquoise","palevioletred","papayawhip","peachpuff","peru","pink","plum","powderblue","purple","rebeccapurple","red","rosybrown","royalblue","saddlebrown","salmon","sandybrown","seagreen","seashell","sienna","silver","skyblue","slateblue","slategrey","snow","springgreen","steelblue","tan","teal","thistle","tomato","turquoise","violet","wheat","white","whitesmoke","yellow","yellowgreen" };
        // clang-format on

        for (const auto& colourName : knownColourNames)
        {
            if (juce::Colours::findColourForName(colourName, juce::Colours::transparentBlack) == colour)
                return str << "juce::Colours::" << colourName;
        }

        return str << "juce::Colour{ 0x" << colour.toDisplayString(true) << " }";
    }

    String& operator<<(String& str, const Font& font)
    {
        return str << "juce::Font{ \"" << font.toString() << "\" }";
    }

    String& operator<<(String& str, const Image& image)
    {
        return str << "juce::Image{ "
                   << image.getWidth()
                   << ", "
                   << image.getHeight()
                   << " }";
    }

    String& operator<<(String& str, const var& value)
    {
        if (value.isString())
            return str << "juce::var{ \"" << value.toString() << "\" }";

        return str << "juce::var{ " << value.toString() << " }";
    }

    String& operator<<(String& str, const Time& time)
    {
        static constexpr auto includeDividers = true;
        return str << "juce::Time{ "
                   << time.toISO8601(includeDividers)
                   << " }";
    }

    String& operator<<(String& str, long double value)
    {
        return str << juce::String{ static_cast<double>(value) };
    }

    String& operator<<(String& str, RelativeTime relativeTime)
    {
        return str << juce::String{ relativeTime.inSeconds() } << "s";
    }

    String& operator<<(String& str, juce::Grid::Px px)
    {
        return str << "juce::Grid::Px{ " << px.pixels << " }";
    }

    String& operator<<(String& str, juce::Grid::TrackInfo track)
    {
        return str << "juce::Grid::Track{ " << track.getSize() << " }";
    }

    String& operator<<(String& str, const juce::ValueTree& tree)
    {
        return str << tree.toXmlString(juce::XmlElement::TextFormat{}
                                           .withoutHeader()
                                           .singleLine());
    }
    // LCOV_EXCL_STOP
} // namespace juce
