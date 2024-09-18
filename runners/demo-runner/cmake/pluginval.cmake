function (add_pluginval_tests
          TARGET
)
    file(DOWNLOAD
         https://raw.githubusercontent.com/Tracktion/pluginval/refs/heads/develop/tests/AddPluginvalTests.cmake
         ${CMAKE_BINARY_DIR}/pluginval/AddPluginvalTests.cmake
    )
    include(${CMAKE_BINARY_DIR}/pluginval/AddPluginvalTests.cmake)
    add_pluginval_tests(${TARGET})
endfunction ()
