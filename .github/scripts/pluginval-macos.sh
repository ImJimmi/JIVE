curl -L "https://github.com/Tracktion/pluginval/releases/latest/download/pluginval_macOS.zip" -o pluginval.zip
unzip pluginval
pluginval.app/Contents/MacOS/pluginval --validate-in-process --strictness-level 10 --verbose --output-dir "./bin" "./build/runners/demo-runner/jive-demo-plugin_artefacts/Release/VST3/JIVE Demo.vst3" || exit 1
