#pragma once

#include <BinaryData.h>
#include <jive_core/jive_core.h>

namespace jive_demo::icons
{
    namespace small
    {
        [[nodiscard]] inline auto back()
        {
            return jive::parseXML(binary_data::backsmall_svg,
                                  binary_data::backsmall_svgSize);
        }

        [[nodiscard]] inline auto next()
        {
            return jive::parseXML(binary_data::nextsmall_svg,
                                  binary_data::nextsmall_svgSize);
        }
    } // namespace small

    namespace large
    {
        [[nodiscard]] inline auto build()
        {
            return jive::parseXML(binary_data::buildlarge_svg,
                                  binary_data::buildlarge_svgSize);
        }

        [[nodiscard]] inline auto code()
        {
            return jive::parseXML(binary_data::codelarge_svg,
                                  binary_data::codelarge_svgSize);
        }

        [[nodiscard]] inline auto layout()
        {
            return jive::parseXML(binary_data::layoutlarge_svg,
                                  binary_data::layoutlarge_svgSize);
        }

        [[nodiscard]] inline auto motionBlur()
        {
            return jive::parseXML(binary_data::motionblurlarge_svg,
                                  binary_data::motionblurlarge_svgSize);
        }

        [[nodiscard]] inline auto palette()
        {
            return jive::parseXML(binary_data::palettelarge_svg,
                                  binary_data::palettelarge_svgSize);
        }
    } // namespace large
} // namespace jive_demo::icons
