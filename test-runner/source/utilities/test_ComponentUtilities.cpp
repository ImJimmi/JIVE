#include "test_ComponentUtilities.h"

//======================================================================================================================
namespace test
{
    //==================================================================================================================
    std::unique_ptr<juce::Component> createDummyComponent()
    {
        return std::make_unique<juce::Component>();
    }
} // namespace test
