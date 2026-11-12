import os
import subprocess
from contextlib import contextmanager
import sys
from pathlib import Path

import pytest
import xa11y

REPO_ROOT = Path(__file__).resolve().parents[3]
BUILD_DIR = Path(os.environ.get("JIVE_BUILD_DIR", REPO_ROOT / "build"))
CONFIG = os.environ.get("JIVE_BUILD_CONFIG", "Debug")

EXAMPLES = {
    "01-text": "JIVE Text",
    "02-layouts": "JIVE Layouts",
    "03-style-sheets": "JIVE Style Sheets",
    "04-transitions": "JIVE Transitions",
    "05-widgets": "JIVE Widgets",
}


def executable_pattern(name: str) -> str:
    if sys.platform == "win32":
        return f"{name}.exe"

    if sys.platform.startswith("linux"):
        return name

    return f"{name}.app/Contents/MacOS/{name}"


def find_executable(example: str) -> Path:
    name = EXAMPLES[example]
    artefacts = BUILD_DIR / "examples" / example / f"jive-{example}_artefacts"

    if (artefacts / CONFIG).is_dir():
        artefacts /= CONFIG

    candidates = [
        path
        for path in artefacts.rglob(executable_pattern(name))
        if path.is_file() and os.access(path, os.X_OK)
    ]

    if not candidates:
        pytest.skip(f"{name} isn't built for {CONFIG} in {artefacts}")

    return candidates[0]


@contextmanager
def launch(example: str):
    process = subprocess.Popen([find_executable(example)])

    try:
        yield process
    finally:
        process.terminate()
        process.wait(timeout=10)


@pytest.fixture(scope="module")
def process(request: pytest.FixtureRequest):
    with launch(request.param) as process:
        yield process


@pytest.fixture(scope="module")
def window(request: pytest.FixtureRequest):
    if sys.platform.startswith("linux"):
        pytest.skip("JUCE has no accessibility support on Linux")

    example = request.param

    with launch(example) as process:
        app = xa11y.App.by_pid(process.pid, timeout=20)
        locator = app.locator(f"window[name='{EXAMPLES[example]}']")
        locator.wait_visible(timeout=20)
        locator.descendant("combo_box").first().wait_visible(timeout=20)
        yield locator
