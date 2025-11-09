#include "jive_Interpolate.h"

#include <jive_core/geometry/jive_BorderRadii.h>
#include <jive_core/graphics/jive_Fill.h>

#if JIVE_UNIT_TESTS
    #include <jive_core/logging/jive_StringStreams.h>

class InterpolateTest : public juce::UnitTest
{
public:
    InterpolateTest()
        : juce::UnitTest{ "jive::interpolate()", "jive" }
    {
    }

    void runTest() final
    {
        testNumericInterpolation();
        testStringInterpolation();
        testBorderSizeInterpolation();
        testArrayInterpolation();
        testGridPxInterpolation();
        testGridTrackInfoInterpolation();
        testColourInterpolation();
    }

private:
    void testNumericInterpolation()
    {
        beginTest("int");
        expectEquals(jive::interpolate(5, 15, 0.5), 10);

        beginTest("float");
        expectEquals(jive::interpolate(0.0f, 100.0f, 0.123), 12.3f);

        beginTest("double");
        expectEquals(jive::interpolate(0.0, 3.0, 0.222), 0.666);
    }

    void testStringInterpolation()
    {
        beginTest("string / numeric");
        expectEquals<juce::String>(jive::interpolate<juce::String>("100", "200", 0.1),
                                   "110");

        beginTest("string / non-numeric");
        expectEquals<juce::String>(jive::interpolate<juce::String>("hello", "world", 0.45),
                                   "world");
    }

    void testBorderSizeInterpolation()
    {
        beginTest("border size");
        expectEquals(jive::interpolate(juce::BorderSize{ 1.0, 2.0, 3.0, 4.0 }, juce::BorderSize{ 2.0, 3.0, 4.0, 5.0 }, 0.5),
                     juce::BorderSize{ 1.5, 2.5, 3.5, 4.5 });

        beginTest("border radii");
        expectEquals(jive::interpolate(jive::BorderRadii{ 1.0, 2.0, 3.0, 4.0 }, jive::BorderRadii{ 2.0, 3.0, 4.0, 5.0 }, 0.5),
                     jive::BorderRadii{ 1.5, 2.5, 3.5, 4.5 });
    }

    void testArrayInterpolation()
    {
        beginTest("array");
        expectEquals(jive::interpolate(juce::Array{ 1, 10, 100 }, juce::Array{ 5, 50, 500, 5000 }, 0.25),
                     juce::Array{ 2, 20, 200, 5000 });
    }

    void testGridPxInterpolation()
    {
        beginTest("grid / px");
        expectEquals(jive::interpolate(juce::Grid::Px{ 100 }, juce::Grid::Px{ 200 }, 0.3).pixels,
                     juce::Grid::Px{ 130 }.pixels);
    }

    void testGridTrackInfoInterpolation()
    {
        beginTest("grid / track info / both px");
        expectEquals(jive::interpolate(juce::Grid::TrackInfo{ juce::Grid::Px{ 100 } },
                                       juce::Grid::TrackInfo{ juce::Grid::Px{ 200 } },
                                       0.3)
                         .getSize(),
                     juce::Grid::TrackInfo{ juce::Grid::Px{ 130 } }.getSize());

        beginTest("grid / track info / both fr");
        expectEquals(jive::interpolate(juce::Grid::TrackInfo{ juce::Grid::Fr{ 1 } },
                                       juce::Grid::TrackInfo{ juce::Grid::Fr{ 2 } },
                                       2.0 / 3.0)
                         .getSize(),
                     juce::Grid::TrackInfo{ juce::Grid::Fr{ 1667 } }.getSize());

        beginTest("grid / track info / different");
        expectEquals(jive::interpolate(juce::Grid::TrackInfo{ juce::Grid::Px{ 100 } },
                                       juce::Grid::TrackInfo{ juce::Grid::Fr{ 2 } },
                                       0.4174)
                         .getSize(),
                     juce::Grid::TrackInfo{ juce::Grid::Fr{ 2 } }.getSize());
    }

    void testColourInterpolation()
    {
        beginTest("colour");
        expectEquals(jive::interpolate(juce::Colours::red, juce::Colours::blue, 0.5),
                     juce::Colours::red.interpolatedWith(juce::Colours::blue, 0.5f));

        beginTest("fill");
        expect(jive::interpolate(jive::Fill{ juce::Colours::orange },
                                 jive::Fill{ juce::Colours::cornflowerblue },
                                 0.3)
               == jive::Fill{ juce::Colours::orange.interpolatedWith(juce::Colours::cornflowerblue, 0.3f) });

        expect(jive::interpolate(jive::Fill{ jive::Gradient{} },
                                 jive::Fill{ juce::Colours::aliceblue },
                                 0.7)
               == jive::Fill{ juce::Colours::aliceblue });
    }
};

static InterpolateTest interpolateTest;
#endif
