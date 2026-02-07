#include "FlexStressTest.h"
#include "MinimumViewBenchmark.h"

class Temp : public Benchmark
{
public:
    Temp()
        : Benchmark{
            "Temp",
            juce::RelativeTime::seconds(20.0),
        }
    {
    }

protected:
    void prepare(jive::Interpreter& interpreter) final
    {
        window = interpreter.interpret(juce::ValueTree{
            "Window",
            {
                { "width", 500 },
                { "height", 2000 },
            },
            {
                juce::ValueTree{
                    "Text",
                    {
                        {
                            "text",
                            "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Integer ut vehicula ligula. Duis dictum turpis non dignissim aliquam. Fusce non velit eu metus vehicula vehicula. Curabitur vel venenatis quam. Nam rhoncus urna in quam aliquet, nec convallis purus placerat. Etiam tristique faucibus leo vitae consequat. Aliquam a augue eget justo rhoncus bibendum. Vivamus ut sapien ex. Suspendisse semper mi eu turpis molestie, in posuere elit accumsan.\n"
                            "In a augue eget felis varius pharetra. Integer sed mi dolor. Fusce egestas mi eu elementum suscipit. Aliquam consequat iaculis nunc ac consectetur. Ut vitae augue ultrices, scelerisque purus vitae, iaculis dolor. Cras maximus tellus sit amet lacus rutrum pulvinar. Praesent dolor justo, feugiat nec diam vitae, fringilla egestas ante. Praesent ac quam laoreet lorem finibus eleifend. Fusce fermentum placerat interdum. Aliquam malesuada ipsum tellus, nec aliquam nibh euismod eget. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nullam dapibus, lacus a molestie gravida, eros sem vestibulum turpis, quis aliquam ligula erat id eros.\n"
                            "Vestibulum urna leo, tristique a erat non, euismod lacinia lacus. Nullam in libero quis urna egestas ultrices pharetra quis nisi. Aenean sed urna sed metus consectetur lobortis. Sed vitae efficitur diam, vel viverra nisi. Morbi egestas nibh at eleifend iaculis. Morbi a libero sollicitudin, finibus massa a, tristique massa. Aenean id sem sem. Donec lobortis nisi eu odio blandit, non varius nulla ultrices. Nullam tincidunt et ligula sit amet iaculis. Fusce venenatis congue nisl, vel interdum diam pellentesque sit amet. Vivamus tincidunt augue dui, sit amet fermentum nunc commodo quis. Duis vel metus vehicula, scelerisque risus vitae, fringilla nulla. Proin ut elit in urna ultrices ornare id in leo. Curabitur tortor nisi, vestibulum ut mollis vitae, lobortis eget erat. Duis consequat hendrerit tortor semper suscipit.\n"
                            "Pellentesque viverra vehicula molestie. Vivamus pulvinar, dui at porttitor pellentesque, neque velit fringilla felis, id porttitor dui est vel nibh. Suspendisse fermentum urna tellus, quis laoreet turpis vulputate sed. Nulla vulputate nunc laoreet tellus sagittis, in consequat nunc pharetra. Aliquam vitae rutrum quam. Donec scelerisque tortor est, et fermentum mi suscipit sit amet. Pellentesque accumsan maximus ante, in vestibulum elit rhoncus quis.\n"
                            "In porta ante eu pulvinar dignissim. Ut porttitor, nunc et ultricies fermentum, ante quam rutrum erat, ut sodales est mi sed magna. Fusce molestie interdum massa, ut eleifend massa molestie at. Curabitur nulla tellus, laoreet sed tellus vitae, mattis condimentum nibh. In hac habitasse platea dictumst. Pellentesque eget tincidunt neque, sit amet scelerisque quam. Cras malesuada magna ac turpis porta laoreet. Sed mollis nulla pretium maximus luctus. In lacinia lectus tortor, eu sollicitudin justo dictum a. Suspendisse sed justo fringilla, vestibulum magna in, blandit ipsum. Curabitur fermentum ornare leo vitae elementum. Vivamus volutpat, leo id luctus elementum, est diam semper tortor, rutrum mattis enim arcu imperdiet massa. Phasellus ornare aliquet quam non cursus.",
                        },
                    },
                },
            },
        });
    }

    void doIteration(jive::Interpreter&) final
    {
        const auto width = juce::Random::getSystemRandom().nextInt({ 200, 800 });
        JIVE_TRACE("width", width);
        window->state.setProperty("width", width, nullptr);
    }

private:
    std::unique_ptr<jive::GuiItem> window;
};

class BenchmarkApp : public juce::JUCEApplication
{
public:
    BenchmarkApp() = default;

    const juce::String getApplicationName() final
    {
        return "JIVE Benchmarking";
    }

    const juce::String getApplicationVersion() final
    {
        return "1.0.0";
    }

    void initialise(const juce::String&) final
    {
#if PERFETTO
        MelatoninPerfetto tracingSession;
#endif

        // MinimumViewBenchmark{}.run();
        // FlexStressTest{}.run();
        Temp{}.run();
        quit();
    }

    void shutdown() final
    {
    }

private:
};

START_JUCE_APPLICATION(BenchmarkApp)
