#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class Length
    {
    public:
        //==============================================================================================================
        void setCorrespondingGuiItem(const GuiItem& correspondingItem);

        void setPixels(float pixels);
        float getPixels() const;
        bool isPixels() const;

        void setPercent(float percent);
        float getPercent() const;
        bool isPercent() const;

        //==============================================================================================================
        operator float() const;

    protected:
        //==============================================================================================================
        virtual float getRelativeParentLength() const = 0;

        //==============================================================================================================
        template <typename LengthType>
        static LengthType createFromPixels(float pixels)
        {
            LengthType length;
            length.setPixels(pixels);

            return length;
        }

        template <typename LengthType>
        static LengthType createFromPercent(float percent, const GuiItem& correspondingItem)
        {
            LengthType length;
            length.setCorrespondingGuiItem(correspondingItem);
            length.setPercent(percent);

            return length;
        }

        //==============================================================================================================
        const GuiItem* item = nullptr;

    private:
        //==============================================================================================================
        enum class Unit
        {
            pixels,
            percent,
        };

        //==============================================================================================================
        float magnitude{ 0.f };
        Unit unit;

        //==============================================================================================================
        JUCE_LEAK_DETECTOR(Length)
    };

    //==================================================================================================================
    class Width : public Length
    {
    public:
        //==============================================================================================================
        using Length::Length;

        //==============================================================================================================
        static Width fromPixels(float pixels);
        static Width fromPercent(float percent, const GuiItem& correspondingItem);

    protected:
        //==============================================================================================================
        float getRelativeParentLength() const final;
    };

    //==================================================================================================================
    class Height : public Length
    {
    public:
        //==============================================================================================================
        using Length::Length;

        //==============================================================================================================
        static Height fromPixels(float pixels);
        static Height fromPercent(float percent, const GuiItem& correspondingItem);

    protected:
        //==============================================================================================================
        float getRelativeParentLength() const final;
    };
} // namespace jive
