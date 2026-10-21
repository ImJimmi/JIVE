# Your First View

Before getting started, make sure you have followed the [integration](../../README.md#integration) steps.

## JIVE Sources

Before we can write our first view, we need to tell JIVE where to look for our source files.

JIVE sources (XML and JSON files) aren't compiled or minified in any way, so it's best practice to keep them separate from your other source code. The recommended practice is to use a `public/` directory next to your `source/` directory.

### CMake

CMake users should use the `jive_sources()` function to specify where to look for JIVE sources:

```cmake
jive_sources(my-juce-project
SOURCES
  # Glob matching all files in the public/ directory
  "${PROJECT_SOURCE_DIR}/public/*"

  # Multiple globs can be provided if adding sources from other
  # places, e.g. a shared library of common components.
  # "${PROJECT_SOURCE_DIR}/modules/my-component-library/*"
)
```

Sources are always relative to the directory containing the `CMakeLists.txt` that called `jive_sources()`. E.g. in this case we would use <code>jive::SourceFile{&nbsp;"public/some.file"&nbsp;}</code>, or <code>jive::SourceFile{&nbsp;"modules/my&#8209;component&#8209;library/some&#8209;other.file"&nbsp;}</code>.

### Projucer

Projucer users will need to manually specify their JIVE source directories by creating one or more `jive::SourceRoot` objects.

```cpp
static const jive::SourceRoot projectSources{ juce::File{ "path/to/project" } };

// Multiple roots can be created if adding sources from other
// places, e.g. a shared library of common components.
// static const jive::SourceRoot componentSources{
//     juce::File{ "path/to/project" }.getChildFile("../my-component-library"),
// };
```

_You'll likely want to use a macro defined in your Projucer project, or `juce::File::getSpecialLocation()` rather than hard-coded paths._

You will need to handle the zip bundling yourself, and use the constructor of `jive::SourceRoot` that takes a pointer to a zip archive for Release builds. See [the example projects](../../examples/) for a working solution.

## `MainView.xml`

Create a file called `MainView.xml` in your project's `public/` directory. 

_You can use other names if you wish, JIVE has no restrictions on filenames so long as they can be found and parsed._

```xml
<Window width="600" height="400">
</Window>
```

**If you're building a plugin, replace `Window` with `Editor`.**

## Interpreting the View

To build our UI, we use a `jive::Interpreter` which is responsible for parsing our JIVE sources, building the hierarchy of components, and responding to any changes in the parsed `juce::ValueTree` or the original source file if hot reloading is enabled.

### Applications

```cpp
#include <jive_layouts/jive_layouts.h>

class Application final : public juce::JUCEApplication
{
public:
  // ...

  void initialise(const juce::String&) override
  {
    window = interpreter.interpret(jive::SourceFile{ "public/MainView.xml" });
  }

private:
  jive::Interpreter interpreter;
  std::unique_ptr<jive::GuiItem> window;
};
```

### Plugins

```cpp
#include <jive_layouts/jive_layouts.h>

class PluginProcessor final : public juce::AudioProcessor
{
public:
  juce::AudioProcessorEditor* createEditor() override
  {
    auto item = interpreter.interpret(jive::SourceFile{ "public/MainView.xml" },
                                      this);

    // The top-level jive::GuiItem is also a juce::AudioProcessorEditor,
    // so we can dynamic_cast to get the editor here (unless the
    // interpreter failed in which case pluginEditor will be nullptr)
    auto* pluginEditor = dynamic_cast<juce::AudioProcessorEditor*>(item.get());

    if (pluginEditor == nullptr)
      return nullptr;

    editor = item.release();
    return pluginEditor;
  }

private:
  jive::Interpreter interpreter;

    // We can't own the editor like we own the window in applications, as
    // it's owned by the JUCE base class, so we use a weak reference here
    // instead. Just remember to always check it still exists before trying
    // to access it.
  juce::WeakReference<jive::GuiItem> editor;
};
```

## First Run

Build and run your project and you should see a blank, 600x400 window appear. If you're using CMake and have hot-reloading enabled (which it is by default), you should now be able to edit `MainView.xml` and see the UI change immediately without having to rebuild.

## Adding widgets

Edit `MainView.xml` to add some child components to the window:

```xml
<Window width="600" height="400" padding="50">
  <Text>My First JIVE UI</Text>

  <Button id="counter-button">Click Me!</Button>
  <Text id="counter-label">Clicked 0 times</Text>
</Window>
```

## Event handlers

To respond to when a user interacts with a widget, we need to assign event handlers. JIVE doesn't have any scripting capabilities, so event handling is done in C++.

_From here on out the code snippets will only show the application use-case, but all the same techniques also apply to plugins._

```cpp
class Application final : public juce::JUCEApplication
{
public:
  // ...

  void initialise(const juce::String&) override
  {
    window = interpreter.interpret(jive::SourceFile{ "public/MainView.xml" });

    // After the component hierarchy has been built, we can interact with
    // the ValueTrees that control the component tree.
    // Here, we find the value-tree node representing the button we want
    // to attach the event.
    auto buttonState = jive::findElementWithID(window->state, "counter-button");

    // Make a new event handle to respond to the button's on-click
    // callback.
    onClick = jive::Event{ buttonState, "on-click" };
    onClick->onTrigger = [this] {
      // This lambda is called when the button is clicked.
      // Here, we find the ValueTree for the label, and set its "text"
      // property with the incremented counter.
      // JIVE will respond to this and update the label UI component
      // to show the new text.
      auto labelState = jive::findElementWithID(window->state, "counter-label");
      labelState["text"] = "Clicked " + ++numClicks + " times";
    };
  }

private:
  jive::Interpreter interpreter;
  std::unique_ptr<jive::GuiItem> window;
  std::optional<jive::Event> onClick;
  int numClicks = 0;
};
```

## Styling

To customise the styling of our UI, we use JIVE's style sheets API.

Create a new file in the `public/` directory called `Styles.json`:

```jsonc
{
  // Background colour of the window
  "background": "#250001",

  // Default foreground (text) colour for all elements
  "foreground": "#F3F3F3",

  // Use the "Button" selector to specify styles for all buttons
  "Button": {
    "background": "#590004",
    "border": "#A50104"
  },

  // Use an ID selector to style just the counter-label component
  "#counter-label": {
    "foreground": "#FCBA04"
  }
}
```

Then edit your view to tell it to use your new style sheet:

```xml
<Window width="600"
        height="400"
        padding="50"
        style="public/Styles.json"
>
    <!-- ... -->
</Window>
```

JIVE will automatically pick up the "style" property, find and parse the style sheet, and apply the rules to your UI.
