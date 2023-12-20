[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest https://github.com/Tracktion/pluginval/releases/latest/download/pluginval_Windows.zip -OutFile pluginval.zip
Expand-Archive pluginval.zip -DestinationPath .
.\pluginval.exe --validate-in-process --strictness-level 10 --verbose --output-dir "./bin" "./build/runners/demo-runner/jive-demo-plugin_artefacts/Release/VST3/JIVE Demo.vst3"
Exit $LASTEXITCODE
