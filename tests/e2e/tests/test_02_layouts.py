import pytest
import xa11y

pytestmark = pytest.mark.parametrize("window", ["02-layouts"], indirect=True)


def is_on(element: xa11y.Element | None) -> bool:
    return element is not None and element.checked == "on"


def test_starts_on_flex(window: xa11y.Locator):
    assert window.descendant("radio_button[name='Flex']").element().checked == "on"
    assert window.descendant("static_text[value^='All containers use the Flex layout']").exists()


def test_grid_tab_shows_grid_controls(window: xa11y.Locator):
    grid = window.descendant("radio_button[name='Grid']")

    grid.press()

    grid.wait_until(is_on)
    window.descendant("static_text[value^=\"Set 'display' to grid\"]").wait_attached()
    assert window.descendant("combo_box[value='60px 60px 60px']").exists()


def test_absolute_tab_selects(window: xa11y.Locator):
    absolute = window.descendant("radio_button[name='Absolute']")

    absolute.press()

    absolute.wait_until(is_on)
    assert window.descendant("radio_button[name='Flex']").element().checked == "off"
