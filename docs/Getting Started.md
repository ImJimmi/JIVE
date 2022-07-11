# Getting Started

- [Getting Started](#getting-started)
    - [Creating the page template](#creating-the-page-template)
        - [Parsing XML files (recommended)](#parsing-xml-files-recommended)
        - [Inline Construction](#inline-construction)
        - [Inline XML String](#inline-xml-string)
    - [Creating the window](#creating-the-window)
    - [Styling](#styling)
        - [Parsing JSON files (recommended)](#parsing-json-files-recommended)
        - [Inline JSON String](#inline-json-string)
    - [Testing](#testing)

Once you have JIVE integrated with your project as shown in the [README](../README.md), you can get started writing code to create your first window with JIVE!

## Creating the page template

The first thing we need to do is to create a [`juce::ValueTree`](https://docs.juce.com/master/classValueTree.html) object describing what elements we want to display. There are a few different ways to do this:

### Parsing XML files (recommended)

<details open>

_WindowPresenter.h_

```cpp
// Note how we don't need to include JIVE here!
// The presenters only depend on `juce::ValueTree`.
#include <BinaryData.h>
#include <juce_core/juce_core.h>

class WindowPresenter
{
public:
    WindowPresenter()
        : view{
            juce::ValueTree::fromXml(juce::String::createStringFromData(
                BinaryData::window_xml,
                BinaryData::window_xmlSize
            )),
        }
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

_window.xml_

```xml
<Window width="640" height="400">
    <Text text="Hello, World!"></Text>
</Window>
```

_CMakeLists.txt_

```cmake
target_sources(my_juce_project
    # ...
    path/to/WindowPresenter.h
)

juce_add_binary_data(my_juce_project
    # ...
    path/to/window.xml
)
```
</details>

### Inline Construction

<details>

_WindowPresenter.h_

```cpp
class WindowPresenter
{
public:
    WindowPresenter()
        : view{
            "Window",
            {
                { "width", 640 },
                { "height", 400 },
            },
            {
                juce::ValueTree {
                    "Text",
                    {
                        { "text", "Hello, World!" },
                    },
                },
            },
        }
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

</details>

### Inline XML String

<details>

_WindowPresenter.h_

```cpp
class WindowPresenter
{
public:
    WindowPresenter()
        : view{
            juce::ValueTree::fromXml(R"(
                <Window width="640", height="400">
                    <Text text="Hello, World!"></Text>
                </Window>
            )")
        }
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

</details>

## Creating the window

Now that we have our `juce::ValueTree` describing what we'd like to be shown, let's actually have it shown! To do this, we simply pass the value-tree to a `jive::Interpreter` which will give us back a `std::unique_ptr<jive::GuiItem>` if it parsed successfully:

_MyJuceApp.h_

```cpp
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

Now we have a window, but it doesn't look particularly exciting. Let's define some styling using JSON.

Once again, there's a few different approaches to this:

### Parsing JSON files (recommended)

<details open>

_WindowPresenter.h_

```cpp
//...
#include <jive_core/jive_core.h>

class WindowPresenter
{
public:
    WindowPresenter()
        : view{/*...*/}
    {
        view.setProperty("style",
                         jive::parseJSON(juce::String::createStringFromData(
                            BinaryData::styles_json,
                            BinaryData::styles_jsonSize
                         )),
                         nullptr);
    }

    //...
};
```

_styles.json_

```json
{
    "background": "#254E70",
    "foreground": "#AEF3E7",

    "font-family": "Helvetica",
    "font-size": 15,
}
```

</details>

### Inline JSON String

<details>

_WindowPresenter.h_

```cpp
//...
#include <jive_core/jive_core.h>

class WindowPresenter
{
public:
    WindowPresenter()
        : view{/*...*/}
    {
        view.setProperty("style",
                         jive::parseJSON(R"({
                            "background": "#254E70",
                            "foreground": "#AEF3E7",

                            "font-family": "Helvetica",
                            "font-size": 15,
                         })"),
                         nullptr);
    }

    //...
};
```

</details>

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
