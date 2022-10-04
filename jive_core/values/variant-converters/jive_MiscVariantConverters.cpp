#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace juce
{
    //==================================================================================================================
    Justification VariantConverter<Justification>::fromVar(const var& v)
    {
        if (v == "centred-left")
            return Justification::centredLeft;
        else if (v == "top-left")
            return Justification::topLeft;
        else if (v == "centred-top")
            return Justification::centredTop;
        else if (v == "top-right")
            return Justification::topRight;
        else if (v == "centred-right")
            return Justification::centredRight;
        else if (v == "bottom-right")
            return Justification::bottomRight;
        else if (v == "centred-bottom")
            return Justification::centredBottom;
        else if (v == "bottom-left")
            return Justification::bottomLeft;
        else if (v == "centred")
            return Justification::centred;

        return Justification::centred;
    }

    var VariantConverter<Justification>::toVar(Justification justification)
    {
        if (justification == Justification::centredLeft)
            return "centred-left";
        else if (justification == Justification::topLeft)
            return "top-left";
        else if (justification == Justification::centredTop)
            return "centred-top";
        else if (justification == Justification::topRight)
            return "top-right";
        else if (justification == Justification::centredRight)
            return "centred-right";
        else if (justification == Justification::bottomRight)
            return "bottom-right";
        else if (justification == Justification::centredBottom)
            return "centred-bottom";
        else if (justification == Justification::bottomLeft)
            return "bottom-left";
        else if (justification == Justification::centred)
            return "centred";

        return {};
    }

    //==================================================================================================================
    Identifier VariantConverter<Identifier>::fromVar(const var& v)
    {
        if (v.toString().isEmpty())
            return {};

        return { v.toString() };
    }

    var VariantConverter<Identifier>::toVar(Identifier id)
    {
        return { id.toString() };
    }

    //==================================================================================================================
    const Array<var> VariantConverter<MouseCursor::StandardCursorType>::options = {
        "inherit",
        "none",
        "default",
        "wait",
        "text",
        "crosshair",
        "copy",
        "pointer",
        "grab",
        "left-right",
        "up-down",
        "move",
        "up",
        "down",
        "left",
        "right",
        "up-left",
        "up-right",
        "down-left",
        "down-right"
    };

    MouseCursor::StandardCursorType VariantConverter<MouseCursor::StandardCursorType>::fromVar(const var& v)
    {
        jassert(options.contains(v));
        return static_cast<MouseCursor::StandardCursorType>(options.indexOf(v));
    }

    var VariantConverter<MouseCursor::StandardCursorType>::toVar(MouseCursor::StandardCursorType cursor)
    {
        const auto index = static_cast<int>(cursor);

        jassert(options.size() >= index);
        return options[index];
    }

    //==================================================================================================================
    StringArray VariantConverter<StringArray>::fromVar(const var& v)
    {
        if (v.isArray())
        {
            auto& varArray = *v.getArray();
            StringArray result;

            for (const auto& value : varArray)
            {
                if (!value.isVoid())
                    result.add(value.toString());
            }

            return result;
        }

        return StringArray::fromTokens(v.toString(), " ", {});
    }

    var VariantConverter<StringArray>::toVar(const StringArray& array)
    {
        return array.joinIntoString(" ");
    }

    //==================================================================================================================
    Font VariantConverter<Font>::fromVar(const var& v)
    {
        return Font::fromString(v.toString());
    }

    var VariantConverter<Font>::toVar(const Font& font)
    {
        return font.toString();
    }

    //==================================================================================================================
    DocumentWindow::TitleBarButtons VariantConverter<DocumentWindow::TitleBarButtons>::fromVar(const var& v)
    {
        auto result = 0;

        const auto tokens = StringArray::fromTokens(v.toString(), false);

        if (tokens.contains("close"))
            result += DocumentWindow::closeButton;
        if (tokens.contains("minimise"))
            result += DocumentWindow::minimiseButton;
        if (tokens.contains("maximise"))
            result += DocumentWindow::maximiseButton;

        return static_cast<DocumentWindow::TitleBarButtons>(result);
    }

    var VariantConverter<DocumentWindow::TitleBarButtons>::toVar(const DocumentWindow::TitleBarButtons& flags)
    {
        StringArray tokens;

        if (flags & DocumentWindow::closeButton)
            tokens.add("close");
        if (flags & DocumentWindow::minimiseButton)
            tokens.add("minimise");
        if (flags & DocumentWindow::maximiseButton)
            tokens.add("maximise");

        return tokens.joinIntoString(" ");
    }

    //==================================================================================================================
    Image VariantConverter<Image>::fromVar(const var& v)
    {
        return Image{
            dynamic_cast<ImagePixelData*>(v.getObject())
        };
    }

    var VariantConverter<Image>::toVar(const Image& image)
    {
        return var{ image.getPixelData() };
    }

    //==================================================================================================================
    const std::unordered_map<String, RectanglePlacement::Flags> VariantConverter<RectanglePlacement>::map = {
        { "left", RectanglePlacement::xLeft },
        { "right", RectanglePlacement::xRight },
        { "centred-x", RectanglePlacement::xMid },
        { "top", RectanglePlacement::yTop },
        { "bottom", RectanglePlacement::yBottom },
        { "centred-y", RectanglePlacement::yMid },
        { "stretch", RectanglePlacement::stretchToFit },
        { "fill", RectanglePlacement::fillDestination },
        { "reduce-only", RectanglePlacement::onlyReduceInSize },
        { "increase-only", RectanglePlacement::onlyIncreaseInSize },
        { "do-not-resize", RectanglePlacement::doNotResize },
        { "centred", RectanglePlacement::centred },
    };

    RectanglePlacement VariantConverter<RectanglePlacement>::fromVar(const var& v)
    {
        const auto tokens = StringArray::fromTokens(v.toString(), false);
        return std::accumulate(std::begin(tokens),
                               std::end(tokens),
                               0,
                               [](const auto& flags, const auto& token) {
                                   jassert(map.count(token) > 0);
                                   return flags + map.at(token);
                               });
    }

    var VariantConverter<RectanglePlacement>::toVar(const RectanglePlacement& placement)
    {
        auto stringFlagPair = std::find_if(std::begin(map),
                                           std::end(map),
                                           [&placement](const auto& pair) {
                                               return placement.testFlags(pair.second);
                                           });
        jassert(stringFlagPair != std::end(map));

        return stringFlagPair->first;
    }
} // namespace juce
