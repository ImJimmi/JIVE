#include "test_ComponentUtilities.h"

//======================================================================================================================
namespace test
{
    //==================================================================================================================
    int getPopupMenuNumSeparators(const juce::PopupMenu& menu)
    {
        auto numSeparators = 0;
        juce::PopupMenu::MenuItemIterator iterator{ menu };

        while (iterator.next())
        {
            if (iterator.getItem().isSeparator)
                numSeparators++;
        }

        return numSeparators;
    }

    int getPopupMenuNumHeaders(const juce::PopupMenu& menu)
    {
        auto numHeaders = 0;
        juce::PopupMenu::MenuItemIterator iterator{ menu };

        while (iterator.next())
        {
            if (iterator.getItem().isSectionHeader)
                numHeaders++;
        }

        return numHeaders;
    }
} // namespace test
