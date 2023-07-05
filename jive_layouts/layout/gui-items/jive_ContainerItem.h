#pragma once

namespace jive
{
    class ContainerItem
        : public GuiItemDecorator
        , private BoxModel::Listener
    {
    public:
        explicit ContainerItem(std::unique_ptr<GuiItem> itemToDecorate);
        ~ContainerItem() override;

        void insertChild(std::unique_ptr<GuiItem> child, int index) override;

    protected:
        void boxModelInvalidated(BoxModel& boxModel) override;

        virtual juce::Rectangle<float> calculateIdealSize(juce::Rectangle<float> constraints) const = 0;

        void layoutChanged();

    private:
        Property<float> idealWidth;
        Property<float> idealHeight;

        BoxModel& boxModel;
    };
} // namespace jive
