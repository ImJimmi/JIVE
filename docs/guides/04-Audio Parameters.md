# Audio Parameters

JIVE provides several utilities to make it easier to connect your widgets to the audio parameters they need to control.

## Auto-Attaching Parameters

The `autoAttachParameters()` method of `jive::Interpreter` is the easiest way to attach parameters to their corresponding widgets.

The method will find any widgets in your views whose `"parameter"` or `"id"` properties match the ID of one of the parameters in your `juce::AudioProcessorValueTreeState`. Any changes to the views at runtime such as adding or removing components, changing the `"parameter"` or `"id"` properties, or hot-reloading will also update the parameter attachments.

```xml
<Editor width="400" height="250">
  <Slider parameter="gain"/>
  <Checkbox id="bypass"/>
</Editor>
```

```cpp
juce::UndoManager undoManager;
juce::AudioProcessorValueTreeState parameters{
  *this,
  &undoManager,
  "Parameters",
  createParameterLayout(),
};

jive::Interpreter interpreter;
interpreter.autoAttachParameters(parameters);
```

### Supported Items

| Type                              | Attachment                                                                                                        |
| --------------------------------- | ----------------------------------------------------------------------------------------------------------------- |
| `<Slider>`, `<Knob>`, `<Spinner>` | [`juce::SliderParameterAttachment`](https://docs.juce.com/master/classjuce_1_1SliderParameterAttachment.html)     |
| `<Button>`, `<Checkbox>`          | [`juce::ButtonParameterAttachment`](https://docs.juce.com/master/classjuce_1_1ButtonParameterAttachment.html)     |
| `<ComboBox>`                      | [`juce::ComboBoxParameterAttachment`](https://docs.juce.com/master/classjuce_1_1ComboBoxParameterAttachment.html) |
| `<Text>`                          | Displays the parameter's value, using its string-from-value function                                              |

## Attaching Manually

Widgets that don't match a parameter are left alone by `autoAttachParameters()`, so you can attach them yourself. Find the widget's `jive::GuiItem` and call `attachToParameter()`, passing the parameter and, optionally, an undo manager.

```xml
<Editor width="300" height="200">
  <Slider id="gain-slider" />
</Editor>
```

```cpp
interpreter.onLoad = [this](jive::GuiItem& editorItem) {
  if (auto* slider = jive::findItemWithID(editorItem, "gain-slider"))
    slider->attachToParameter(parameters.getParameter("gain"), &undoManager);
};
```

Passing `nullptr` as the parameter detaches the widget, so it's safe to call `getParameter()` without checking the result.

## Recommended Practices

### Keep the State With the Processor

The host can destroy and recreate your editor at any time, so the parameters, undo manager, and interpreter should all live as long as the processor, not the editor. JIVE's attachments only reference the parameters, so a new editor can simply attach to the same ones again.

### Always Attach Manually in `onLoad`

If you're attaching widgets by hand, attaching straight after `interpret()` works until you edit a view. Hot-reloading rebuilds the GUI items and their attachments go with them, leaving your widgets disconnected. `onLoad` is called after every build, including the first, so attachments made there are always up to date.
