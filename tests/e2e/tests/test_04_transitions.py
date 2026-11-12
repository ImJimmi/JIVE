import pytest
import xa11y

pytestmark = pytest.mark.parametrize("window", ["04-transitions"], indirect=True)


def test_lists_easings(window: xa11y.Locator):
    for easing in ["linear", "ease-in", "ease-out", "ease-in-out"]:
        assert window.descendant(f"static_text[value='x 1500ms {easing}']").exists()


def test_move_toggles(window: xa11y.Locator):
    move = window.descendant("button[name='Move']")
    assert move.element().value == "Off"

    move.press()

    move.wait_until(lambda element: element is not None and element.value == "On")
