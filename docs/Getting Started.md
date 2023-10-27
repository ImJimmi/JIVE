# Getting Started

- [Getting Started](#getting-started)
    - [Creating the page template](#creating-the-page-template)
    - [Creating the window](#creating-the-window)
    - [Styling](#styling)
    - [Testing](#testing)

Once you have JIVE integrated with your project as shown in the [README](../README.md), you can get started writing code to create your first window with JIVE!

## Creating the page template

The first thing we need to do is to create a [`juce::ValueTree`](https://docs.juce.com/master/classValueTree.html) object describing what elements we want to display. You can do this however you like, but the recommended approach is to use free functions that return Value Trees:

```cpp
// Note how we don't need to include JIVE here!
// The presenters only depend on `juce::ValueTree`.
#include <juce_data_structures/juce_data_structures.h>

namespace views
{
    juce::ValueTree welcomeText()
    {
        return juce::ValueTree {
            "Text",
            {
                { "text", "Hello, World!" },
            },
        };
    }

    juce::ValueTree window()
    {
        return juce::ValueTree {
            "Window",
            {
                { "width", 640 },
                { "height", 400 },
            },
            {
                welcomeText(),
            },
        };
    }
} // namespace views

class WindowPresenter
{
public:
    WindowPresenter()
        : view{ views::window() }
    {
    }

    juce::ValueTree preset()
    {
        return view;
    }

private:
    juce::ValueTree view;
};
```

## Creating the window

Now that we have our `juce::ValueTree` describing what we'd like to be shown, let's actually have it shown! To do this, we simply pass the `juce::ValueTree` to a `jive::Interpreter` which will give us back a `std::unique_ptr<jive::GuiItem>` if it parsed successfully:

_MyJuceApp.h_

```cpp
#include "WindowPresenter.h"

#include <jive_layouts/jive_layouts.h>

class MyJuceApp : public juce::JUCEApplication
{
public:
    //...

    void initialise(const juce::String&) final
    {
        jive::Interpreter interpreter;
        window = interpreter.interpret(windowPresenter.present());
    }

    void shutdown() final
    {
        window = nullptr;
    }

private:
    std::unique_ptr<jive::GuiItem> window;
    WindowPresenter windowPresenter;
};

START_JUCE_APPLICATION(MyJuceApp)
```

## Styling

Now we have a window, but it doesn't look particularly exciting. Let's define some styling using `jive::Object`:

```cpp
namespace views
{
    // Our window() function from earlier...
    juce::ValueTree window()
    {
        static constexpr auto style = [] {
            return new jive::Object {
                { "background", "#254E70" },
                { "foreground", "#AEF3E7" },
                { "font-family", "Helvetica" },
                { "font-size", 15 },
            };
        };

        return juce::ValueTree {
            "Window",
            {
                { "width", 640 },
                { "height", 400 },
                { "style", style() },
            },
            {
                welcomeText(),
            },
        };
    }
}
```

## Testing

Testing UIs is always a challenge, especially in JUCE where components are typically very imperative and don't expose many useful details.

With JIVE however, testing becomes as simple as:

_WindowPresenterTests.cpp_

```cpp
#include "WindowPresenter.h"
#include <catch2/catch_test_macros.hpp>

SCENARIO("Presenting a window")
{
    GIVEN("A window presenter")
    {
        WindowPresenter presenter;

        WHEN("The presenter presents its window")
        {
            const auto window = presenter.present();

            THEN("The window has the correct size")
            {
                CHECK(window["width"] == 640);
                CHECK(window["height"] == 400);
            }

            THEN("The window is using a flex layout")
            {
                CHECK(window["display"].toString() == "flex");
            }

            THEN("The window has a single text element")
            {
                CHECK(window.getNumChildren() == 1);

                const auto text = window.getChildWithName("Text");
                REQUIRE(text.isValid());

                AND_THEN("The text element is showing the correct copy")
                {
                    CHECK(text["text"].toString() == "Hello, World!");
                }
            }
        }
    }
}
```
