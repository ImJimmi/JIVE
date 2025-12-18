#include "jive_Shadow.h"

#include "jive_LookAndFeel.h"

#include <jive_core/geometry/jive_Geometry.h>
#include <jive_core/values/jive_Colours.h>

namespace jive
{
    Shadow::Shadow(const juce::String& shadowString)
    {
        *this = fromString(shadowString);
    }

    Shadow::Shadow(const char* shadowString)
        : Shadow{ juce::String{ shadowString } }
    {
    }

    class StackBlur
    {
    public:
        explicit StackBlur(int radius)
            : blurRadius{ radius }
        {
        }

        void setNumPixels(int newNumPixels)
        {
            numPixels = newNumPixels;
        }

        void setStride(int newStride)
        {
            stride = newStride;
        }

        void blur(juce::uint8* pixels) const
        {
            Queue queue{ blurRadius, pixels[0] };

            const auto stackSize = std::pow(blurRadius + 1.0, 2.0);
            auto stack = static_cast<double>(pixels[0]) * stackSize;

            for (auto sourceIndex = stride;
                 sourceIndex < (numPixels + blurRadius) * stride;
                 sourceIndex += stride)
            {
                const auto sourcePixel = pixels[juce::jlimit(0, (numPixels - 1) * stride, sourceIndex)];
                queue.add(sourcePixel);

                stack += queue.calculateStackDifference();

                const auto targetIndex = sourceIndex - (blurRadius * stride);

                if (targetIndex >= 0)
                {
                    const auto alpha = juce::jlimit(0.0,
                                                    static_cast<double>(std::numeric_limits<juce::uint8>::max()),
                                                    std::round(stack / stackSize));
                    const auto targetPixel = static_cast<juce::uint8>(alpha);
                    pixels[targetIndex] = targetPixel;
                }
            }
        }

    private:
        class Queue
        {
        public:
            Queue(int radius, juce::uint8 initialValue)
                : size{ static_cast<juce::uint64>(radius) * 2 + 1 }
                , buffer(static_cast<std::size_t>(size) + 1, initialValue)
                , inBuffer{ &buffer[static_cast<std::size_t>(radius) + 1], radius + 1 }
                , outBuffer{ buffer.data(), radius + 1 }
            {
            }

            void add(juce::uint8 valueToAdd)
            {
                outBuffer.write(inBuffer.front());
                inBuffer.write(valueToAdd);
            }

            double calculateStackDifference() const
            {
                return inBuffer.sum() - outBuffer.sum();
            }

            const juce::uint64 size;

        private:
            class RingBuffer
            {
            public:
                RingBuffer(juce::uint8* sourceData, int bufferSize)
                    : data{ sourceData }
                    , size{ bufferSize }
                {
                }

                void write(juce::uint8 value)
                {
                    data[writeIndex] = value;

                    if (++writeIndex >= size)
                        writeIndex = 0;
                }

                juce::uint8 front() const
                {
                    return data[writeIndex];
                }

                double sum() const
                {
                    return std::accumulate(data, data + size, 0.0);
                }

            private:
                int writeIndex{ 0 };
                juce::uint8* data;
                const int size;
            };

            std::vector<juce::uint8> buffer;

            RingBuffer inBuffer;
            RingBuffer outBuffer;
        };

        //==============================================================================
        const int blurRadius;

        int numPixels{ 0 };
        int stride{ 0 };
    };

    [[nodiscard]] static auto renderNonBlurredShadow(const juce::Rectangle<int>& bounds, const juce::Path& shape, float spread, float imageScale)
    {
        const auto imageSize = bounds
                                   .toFloat()
                                   .transformedBy(juce::AffineTransform::scale(imageScale))
                                   .toNearestInt();

        juce::Image result{
            juce::Image::SingleChannel,
            imageSize.getWidth(),
            imageSize.getHeight(),
            true,
        };

        juce::Graphics g{ result };
        g.addTransform(juce::AffineTransform::scale(imageSize.toFloat().getWidth() / bounds.toFloat().getWidth(),
                                                    imageSize.toFloat().getHeight() / bounds.toFloat().getHeight()));
        g.setColour(juce::Colours::white);
        g.fillPath(shape);
        g.strokePath(shape, juce::PathStrokeType{ spread * 2.0f });

        return result;
    }

    static void blurHorizontally(juce::Image& frameBuffer, const Shadow& shadow, float scale)
    {
        juce::Image::BitmapData bitmapData{
            frameBuffer,
            juce::Image::BitmapData::readWrite,
        };
        const auto numRows = frameBuffer.getHeight();

        StackBlur blur{ juce::roundToInt(shadow.getBlurRadius() * scale) };
        blur.setNumPixels(bitmapData.width);
        blur.setStride(bitmapData.pixelStride);

        static juce::ThreadPool threadPool;
        static constexpr auto numJobsPerThread = 5;

        for (auto i = 0; i < numRows; i += numJobsPerThread)
        {
            threadPool.addJob([&bitmapData, i, numRows, &blur]() {
                for (auto row = i;
                     row < i + numJobsPerThread && row < numRows;
                     row++)
                {
                    auto* const pixels = bitmapData.getPixelPointer(0, row);
                    blur.blur(pixels);
                }
            });
        }

        while (threadPool.getNumJobs() > 0)
        {
        }
    }

    static void blurVertically(juce::Image& frameBuffer, const Shadow& shadow, float scale)
    {
        juce::Image::BitmapData bitmapData{
            frameBuffer,
            juce::Image::BitmapData::readWrite,
        };
        const auto numColumns = frameBuffer.getWidth();

        StackBlur blur{ juce::roundToInt(shadow.getBlurRadius() * scale) };
        blur.setNumPixels(bitmapData.height);
        blur.setStride(bitmapData.lineStride);

        static juce::ThreadPool threadPool;
        static constexpr auto numJobsPerThread = 5;

        for (auto i = 0; i < numColumns; i += numJobsPerThread)
        {
            threadPool.addJob([&bitmapData, i, numColumns, &blur]() {
                for (auto column = i;
                     column < i + numJobsPerThread && column < numColumns;
                     column++)
                {
                    auto* const pixels = bitmapData.getPixelPointer(column, 0);
                    blur.blur(pixels);
                }
            });
        }

        while (threadPool.getNumJobs() > 0)
        {
        }
    }

    void Shadow::draw(juce::Graphics& g,
                      const juce::Rectangle<int>& bounds,
                      const juce::Path& shape) const
    {
        const auto scale = 0.333f;
        auto frameBuffer = renderNonBlurredShadow(bounds, shape, spreadRadius, scale);
        blurHorizontally(frameBuffer, *this, scale);
        blurVertically(frameBuffer, *this, scale);

        g.setColour(colour);
        g.drawImage(frameBuffer, bounds.toFloat(), juce::RectanglePlacement::stretchToFit, true);
    }

    Shadow& Shadow::setOffset(const juce::Point<int>& newOffset)
    {
        offset = newOffset;
        return *this;
    }

    juce::Point<int> Shadow::getOffset() const
    {
        return offset;
    }

    Shadow& Shadow::setColour(const juce::Colour& newColour)
    {
        colour = newColour;
        return *this;
    }

    juce::Colour Shadow::getColour() const
    {
        return colour;
    }

    Shadow& Shadow::setBlurRadius(float newBlurRadius)
    {
        jassert(newBlurRadius >= 0.0f);
        blurRadius = newBlurRadius;
        return *this;
    }

    float Shadow::getBlurRadius() const
    {
        return blurRadius;
    }

    Shadow& Shadow::setSpreadRadius(float newSpreadRadius)
    {
        jassert(newSpreadRadius >= 0.0f);
        spreadRadius = newSpreadRadius;
        return *this;
    }

    float Shadow::getSpreadRadius() const
    {
        return spreadRadius;
    }

    Shadow Shadow::fromString(const juce::String& shadowString)
    {
        if (shadowString.isEmpty())
            return Shadow{};

        std::vector<float> numbers;
        juce::Colour shadowColour;

        for (const auto& token : juce::StringArray::fromTokens(shadowString, " ", "()"))
        {
            if (token.containsOnly("0123456789.px"))
                numbers.push_back(token.getFloatValue());
            else
                shadowColour = parseColour(token);
        }

        jassert(numbers.size() >= 2 && numbers.size() <= 4);

        return Shadow{}
            .setOffset({
                juce::roundToInt(numbers[0]),
                juce::roundToInt(numbers[1]),
            })
            .setColour(shadowColour)
            .setBlurRadius(numbers.size() > 2 ? numbers[2] : 0.0f)
            .setSpreadRadius(numbers.size() > 3 ? numbers[3] : 0.0f);
    }

    ShadowComponent::ShadowComponent(juce::Component& componentToAttachTo)
        : component{ &componentToAttachTo }
    {
        updateParent();
        updateVisibility();

        setInterceptsMouseClicks(false, false);
        setPaintingIsUnclipped(true);

        componentToAttachTo.addComponentListener(this);
    }

    ShadowComponent::~ShadowComponent()
    {
        if (component != nullptr)
            component->removeComponentListener(this);
    }

    void ShadowComponent::paint(juce::Graphics& g)
    {
        if (component == nullptr)
            return;

        juce::Path shape;

        if (component->getProperties().contains("jive::shadow-path"))
        {
            shape.restoreFromString(component->getProperties()["jive::shadow-path"].toString());
            shape.applyTransform(juce::AffineTransform::translation(component->getPosition()));
        }
        else
        {
            const auto boundsString = component
                                          ->getProperties()
                                          .getWithDefault("jive::shadow-bounds",
                                                          component->getBounds().toString())
                                          .toString();
            shape = getShape(borderRadius,
                             juce::Rectangle<int>::fromString(boundsString)
                                 .toFloat());
        }

        const juce::Point blurRadiusOffset{
            static_cast<int>(std::ceil(shadow.getBlurRadius())) * 2,
            static_cast<int>(std::ceil(shadow.getBlurRadius())) * 2,
        };
        shape.applyTransform(juce::AffineTransform::translation(shadow.getOffset() - component->getPosition() + blurRadiusOffset));
        shadow.draw(g, getLocalBounds(), shape);
    }

    void ShadowComponent::setShadow(const Shadow& newShadow)
    {
        if (newShadow == shadow)
            return;

        shadow = newShadow;
        updateBounds();
        repaint();
    }

    void ShadowComponent::componentMovedOrResized(juce::Component&, bool, bool)
    {
        updateBounds();
    }

    void ShadowComponent::componentVisibilityChanged(juce::Component&)
    {
        updateVisibility();
    }

    void ShadowComponent::componentParentHierarchyChanged(juce::Component&)
    {
        updateParent();
    }

    void ShadowComponent::updateBounds()
    {
        if (component == nullptr || parent == nullptr)
            return;

        const juce::Point blurRadiusOffset{
            static_cast<int>(std::ceil(shadow.getBlurRadius())) * 2,
            static_cast<int>(std::ceil(shadow.getBlurRadius())) * 2,
        };

        auto bounds = juce::Rectangle<int>::fromString(component
                                                           ->getProperties()
                                                           .getWithDefault("jive::shadow-bounds",
                                                                           component->getBounds().toString())
                                                           .toString());

        auto position = component->getPosition();

        for (auto* parentComp = component->getParentComponent();
             parentComp != nullptr;
             parentComp = parentComp->getParentComponent())
        {
            if (parentComp == parent)
                break;

            position += parentComp->getPosition();
        }

        setBounds(bounds
                      .expanded(blurRadiusOffset.x, blurRadiusOffset.y)
                      .withPosition(position - blurRadiusOffset + shadow.getOffset()));
    }

    void ShadowComponent::updateVisibility()
    {
        setVisible(component != nullptr ? component->isVisible() : false);
    }

    void ShadowComponent::updateParent()
    {
        if (component == nullptr)
            return;

        auto* laf = dynamic_cast<LookAndFeel*>(&component->getLookAndFeel());

        for (auto* otherParent = component->getParentComponent();
             otherParent != nullptr;
             otherParent = otherParent->getParentComponent())
        {
            juce::FillType background{ juce::Colours::black };

            if (laf != nullptr)
            {
                background = getBackgroundFill(*otherParent,
                                               laf->findMostApplicableStyles(*otherParent),
                                               juce::Colours::transparentBlack);
            }

            if (!background.isInvisible())
            {
                parent = otherParent;
                parent->addChildComponent(this, 0);
                updateBounds();
                return;
            }
        }

        if (auto* thisParent = getParentComponent())
            thisParent->removeChildComponent(this);
    }
} // namespace jive
