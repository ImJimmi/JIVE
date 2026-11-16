import pytest
import xa11y

pytestmark = pytest.mark.parametrize("window", ["05-widgets"], indirect=True)


def has_value(value: str):
    return lambda element: element is not None and element.value == value


def test_button_counts_clicks(window: xa11y.Locator):
    window.descendant("button[name='Click me']").press()

    window.descendant("static_text[value^='Clicked']").wait_until(has_value("Clicked 1 time"))


def test_check_box_toggles(window: xa11y.Locator):
    check_box = window.descendant("check_box[name='Light']")

    check_box.toggle()

    check_box.wait_until(lambda element: element is not None and element.checked == "on")


def test_slider_drives_progress_bar(window: xa11y.Locator):
    window.descendant("slider").first().increment()

    window.descendant("progress_bar").wait_until(has_value("0.61"))


def test_text_editor_updates_greeting(window: xa11y.Locator):
    # JUCE's TextEditor exposes no writable accessibility value, so type real keystrokes.
    window.activate()
    text_box = window.descendant("text_field, text_area").first()
    xa11y.input_sim().click(text_box.element())
    text_box.wait_focused()
    xa11y.input_sim().type_text("Ada")

    window.descendant("static_text[value^='Hello']").wait_until(has_value("Hello, Ada"))


def test_custom_tab_shows_custom_widgets(window: xa11y.Locator):
    window.descendant("radio_button[name='Custom']").press()

    window.descendant("static_text[value^='When a widget needs to look different']").wait_attached()
