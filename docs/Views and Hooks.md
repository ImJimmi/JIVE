# Views and Hooks

The most basic JIVE example looks something like this:

```cpp
// Define an empty view with a size of 300x200
const juce::ValueTree view{
    "Component",
    {
        { "width", 300 },
        { "height", 200 },
    },
};

// Construct the jive::GuiItem for the view
jive::Interpreter interpreter;
const auto guiItem = interpreter.interpret(view);
```

As we start to build up a more complex view however, we probably want to introduce more encapsulation. Encapsulation is great but it can lead to less control, or can require pointers/references to be passed around in order to retain the same level of control.

`jive::View` offers an alternative to using pure `juce::ValueTree`s to define a view allowing you to attach additional behaviour to a view without losing any control over the UI and without needing to manually pass around pointers/references.

## Basic `jive::View`

```cpp
class MyView : public jive::View
{
protected:
    juce::ValueTree initialise() final
    {
        return juce::ValueTree{
            "Component",
            {
                { "width", 300 },
                { "height", 200 },
            },
        };
    }
};

// Construct a ValueTree from our custom View type
const juce::ValueTree view = jive::makeView<MyView>();

jive::Interpreter interpreter;
const auto guiItem = interpreter.interpret(view);
```

Here, we've moved the construction of our `juce::ValueTree` to the `initialise()` method of a custom `View` type. We're then using `jive::makeView()` to construct a `juce::ValueTree` and interpreting it with a `jive::Interpreter` as always.

In this case we're using `jive::makeView()` to construct the top-level element, however we could also use it to construct a child element:

```cpp
const juce::ValueTree topLevelView{
    "Window",
    {
        { "width", 300 },
        { "height", 200 },
    },
    {
        jive::makeView<SomeCustomView>(),
        jive::makeView<CustomViewWithConstructorArgs>(123, "four", 5.6),
    },
};
```

## Hooks

The power of `jive::View` is that it lets us attach additional behaviour to our view so it's not just a pure data structure. `jive::View` provides several `protected` methods we can override in our custom view:

- `initialise()` - pure virtual, must return the initial `juce::ValueTree` that defines the view.
- `createComponent()` - used to instantiate any custom `juce::Component` type for the given `juce::ValueTree` element.
- `setup()` - called after the full hierarchy of `jive::GuiItem`s has been constructed by the interpreter, providing access the the specific `jive::GuiItem` that corresponds to this view. Indirectly provides access to the `juce::Component` that corresponds to this view through the item's `getComponent()` method.
- `getState()` - provides access to the `juce::ValueTree` constructed by `initialise()`.

### Example - Custom Component

Here's an example that uses all of these. We've defined a custom `XYPadComponent` which is then being managed by a custom `XYPadView` that's added as a child to a `juce::ValueTree`.

```cpp
// Controls two parameters on its X and Y axis by allowing the mouse to drag
// around a control point.
class XYPadComponent : public juce::Component
{
public:
    struct Listener {};

    juce::Point<int> getValue();
    void setValue(juce::Point<int> newValue);

private:
    juce::Point<int> currentValue;
};

// Defines and manages an XYPadComponent in the view.
class XYPadView
    : public jive::View
    , private XYPadComponent::Listener
{
    XYPadView()
        : padValue{ getState(), "value" }
    {
    }

protected:
    // Create the initial state of the view
    juce::ValueTree initialise() final
    {
        return juce::ValueTree{
            "XYPad",
            {
                { "width", 200 },
                { "height", 200 },
                { "value", jive::toVar(juce::Point{ 100, 100 }) },
            },
        };
    }

    // Construct our custom component for our view
    std::unique_ptr<juce::Component> createComponent(const juce::ValueTree& tree) final
    {
        if (tree.getType().toString() == "XYPad")
            return std::make_unique<XYPadComponent>();

        return nullptr;
    }

    // Finish setting up the view after the corresponding jive::GuiItem has been
    // constructed
    void setup(jive::GuiItem& item) final
    {
        if (auto* xyPad = dynamic_cast<XYPadComponent*>(item.getComponent().get()))
        {
            xyPad->addListener(this);

            padValue.onValueChange = [this, xyPad] {
                xyPad->setValue(padValue);
            };
        }
        else
        {
            jassertfalse;
        }
    }

private:
    // Update the ValueTree when the component's value changes
    void xyPadValueChanged(XYPadComponent& pad) final
    {
        padValue = pad.getValue();
    }

    jive::Property<juce::Point<int>> padValue;

};

const juce::ValueTree view{
    "Window",
    {
        { "width", 300 },
        { "height", 200 },
    },
    {
        jive::makeView<XYPadView>(),
    },
};

// Construct the hierarchy of GuiItems and Components - including our custom
// component
jive::Interpreter interpreter;
const auto topLevelItem = interpreter.interpret(view);

// We can then access the custom view we constructed with `jive::makeView()`
// from the GuiItem that wraps it
const jive::View::ReferenceCountedPointer xyPadView = topLevelItem
                                                          ->getChildren()[0]
                                                          ->getView();
```

### Example - Widget attachments in a plugin project

```cpp
class EditorView : public jive::View
{
public:
    explicit EditorView(juce::AudioProcessorValueTreeState& processorState)
        : apvts{ processorState }
    {
    }

protected:
    // Construct an Editor type with a Slider element.
    juce::ValueTree initialise() final
    {
        return juce::ValueTree{
            "Editor",
            {
                { "width", 400 },
                { "height", 250 },
            },
            {
                juce::ValueTree { "Slider" },
            },
        };
    }

    // Attach the APVTS to the slider
    void setup(jive::GuiItem& item) final
    {
        if (auto* slider = dynamic_cast<juce::Slider*>(item.getChildren()[0]->getComponent().get()))
            attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "gain", *slider);
    }

private:
    juce::AudioProcessorValueTreeState& apvts;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

jive::Interpreter interpreter;
const auto editor = interpreter.interpret(jive::makeView<EditorView>(apvts), &pluginProcessor);
```

## Conclusion

As demonstrated, we can provide additional behaviour to a view by writing a custom `View` type and using the various hooks to access all the constituents of our UI.

For more details, see [`jive_View.h`](../jive_layouts/hooks/jive_View.h).
