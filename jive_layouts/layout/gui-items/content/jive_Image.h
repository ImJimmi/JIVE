#pragma once

#include <jive_layouts/layout/gui-items/jive_GuiItemDecorator.h>
#include <jive_layouts/utilities/jive_Drawable.h>

namespace jive
{
    class Image
        : public GuiItemDecorator
        , private juce::ComponentListener
        , private BoxModel::Listener
        , private juce::ValueTree::Listener
    {
    public:
        explicit Image(std::unique_ptr<GuiItem> itemToDecorate);
        ~Image() override;

        bool isContainer() const override;
        bool isContent() const override;

        [[nodiscard]] Drawable getDrawable() const;
        [[nodiscard]] ImageComponent& getImageComponent();
        [[nodiscard]] const ImageComponent& getImageComponent() const;

    protected:
        void componentMovedOrResized(juce::Component&, bool, bool) override;
        void boxModelChanged(BoxModel& boxModelThatChanged) override;
        void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& id) override;

    private:
        float calculateAspectRatio(const juce::ImageComponent& image) const;
        float calculateRequiredWidth(const juce::ImageComponent& image) const;
        float calculateRequiredWidth(const juce::Drawable& drawable) const;
        float calculateRequiredWidth() const;
        float calculateRequiredHeight(const juce::ImageComponent& image) const;
        float calculateRequiredHeight(const juce::Drawable& drawable) const;
        float calculateRequiredHeight() const;

        void updateImageComponentDrawable();

        std::unique_ptr<juce::Component> childComponent;
        bool changingChild = false;

        PassiveProperty<Drawable> source;
        PassiveProperty<juce::RectanglePlacement> placement;
        PassiveProperty<Length> width;
        PassiveProperty<Length> height;
        PassiveProperty<Length> idealWidth;
        PassiveProperty<Length> idealHeight;

        const BoxModel& boxModel;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Image)
    };
} // namespace jive
