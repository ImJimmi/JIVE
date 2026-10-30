#include "jive_Shadow.h"

#include "jive_LookAndFeel.h"

#include <jive_style_sheets/jive_style_sheets.h>

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
        static constexpr auto scale = 1.0f;
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

    [[nodiscard]] static int calculateMargin(const Shadow& shadow)
    {
        return static_cast<int>(std::ceil(shadow.getBlurRadius() + shadow.getSpreadRadius()));
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

        for (const auto& ancestor : ancestors)
        {
            if (ancestor != nullptr)
                ancestor->removeComponentListener(this);
        }
    }

    void ShadowComponent::paint(juce::Graphics& g)
    {
        if (component == nullptr)
            return;

        JIVE_TRACE();

        juce::Path shape;

        if (component->getProperties().contains(ids::shadowPath()))
            shape.restoreFromString(component->getProperties()[ids::shadowPath()].toString());
        else
            shape.addRectangle(component->getLocalBounds());

        const auto margin = static_cast<float>(calculateMargin(shadow));
        shape.applyTransform(juce::AffineTransform::translation(margin, margin));

        auto offsetShape = shape;
        offsetShape.applyTransform(juce::AffineTransform::translation(-shadow.getOffset()));

        juce::Path clipRegion;
        clipRegion.addRectangle(getLocalBounds());
        clipRegion.addPath(offsetShape);
        clipRegion.setUsingNonZeroWinding(false);
        g.reduceClipRegion(clipRegion);

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

        auto bounds = component->getBounds()
                          .translated(shadow.getOffset().x, shadow.getOffset().y)
                          .expanded(calculateMargin(shadow));

        for (auto* parentComp = component->getParentComponent();
             parentComp != nullptr;
             parentComp = parentComp->getParentComponent())
        {
            if (parentComp == parent)
                break;

            bounds.translate(parentComp->getX(), parentComp->getY());
        }

        setBounds(bounds);
    }

    void ShadowComponent::updateVisibility()
    {
        setVisible(component != nullptr ? component->isVisible() : false);
    }

    // The shadow is a child of one of the attached component's ancestors, so
    // its bounds depend on the positions of all the components in between.
    // Those in-between components can move without the attached component
    // moving (e.g. when a window is resized), so they must be listened to as
    // well.
    void ShadowComponent::updateAncestors()
    {
        for (const auto& ancestor : ancestors)
        {
            if (ancestor != nullptr)
                ancestor->removeComponentListener(this);
        }

        ancestors.clear();

        if (component == nullptr)
            return;

        for (auto* ancestor = component->getParentComponent();
             ancestor != nullptr && ancestor != parent;
             ancestor = ancestor->getParentComponent())
        {
            ancestor->addComponentListener(this);
            ancestors.emplace_back(ancestor);
        }
    }

    void ShadowComponent::updateParent()
    {
        if (component == nullptr)
            return;

        auto* laf = dynamic_cast<LookAndFeel*>(&component->getLookAndFeel());

        for (auto* otherParent = component->getParentComponent();
             otherParent != nullptr && dynamic_cast<juce::TopLevelWindow*>(otherParent) == nullptr;
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
                jassert(dynamic_cast<juce::TopLevelWindow*>(parent.getComponent()) == nullptr);
                updateAncestors();
                updateBounds();
                return;
            }
        }

        if (auto* thisParent = getParentComponent())
            thisParent->removeChildComponent(this);

        updateAncestors();
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_core/testing/jive_UnitTest.h>

static jive::UnitTest shadowFollowsAncestorsTest{
    "jive",
    "jive::ShadowComponent",
    "Follows moved ancestors",
    [](auto& test) {
        juce::Component root;
        root.setComponentID("root");
        root.setSize(200, 200);

        jive::LookAndFeel lookAndFeel{ root };
        lookAndFeel.addStyles(jive::StyleSelector{ "#root" },
                              jive::Styles{}.withBackground(juce::Colours::black));

        juce::Component container;
        container.setBounds(10, 10, 100, 100);
        root.addAndMakeVisible(container);

        juce::Component target;
        target.setBounds(5, 5, 20, 20);
        container.addAndMakeVisible(target);

        jive::ShadowComponent shadow{ target };
        shadow.setShadow(jive::Shadow{}.setColour(juce::Colours::black));

        test.expect(shadow.getParentComponent() == &root);
        test.expectEquals(shadow.getBounds(), juce::Rectangle{ 15, 15, 20, 20 });

        container.setTopLeftPosition(50, 30);
        test.expectEquals(shadow.getBounds(), juce::Rectangle{ 55, 35, 20, 20 });

        container.setBounds(container.getBounds().withWidth(80));
        target.setBounds(0, 0, 30, 30);
        test.expectEquals(shadow.getBounds(), juce::Rectangle{ 50, 30, 30, 30 });
    },
};

static jive::UnitTest shadowPathsAreLocalTest{
    "jive",
    "jive::ShadowComponent",
    "Shadow paths are in the attached component's own coordinates",
    [](auto& test) {
        juce::Component root;
        root.setComponentID("root");
        root.setSize(100, 50);

        jive::LookAndFeel lookAndFeel{ root };
        lookAndFeel.addStyles(jive::StyleSelector{ "#root" },
                              jive::Styles{}.withBackground(juce::Colours::black));

        juce::Component target;
        target.setBounds(40, 5, 30, 30);
        juce::Path shape;
        shape.addRectangle(0.0f, 10.0f, 10.0f, 10.0f);
        target.getProperties().set(jive::ids::shadowPath(), shape.toString());
        root.addAndMakeVisible(target);

        jive::ShadowComponent shadow{ target };
        shadow.setShadow(jive::Shadow{}
                             .setColour(juce::Colours::red)
                             .setOffset({ 2, 2 }));

        const auto snapshot = root.createComponentSnapshot(root.getLocalBounds());
        test.expect(snapshot.getPixelAt(51, 20) == juce::Colours::red);
        test.expect(snapshot.getPixelAt(45, 20) != juce::Colours::red);
    },
};

static jive::UnitTest shadowIncludesSpreadTest{
    "jive",
    "jive::ShadowComponent",
    "Bounds include the spread radius",
    [](auto& test) {
        juce::Component root;
        root.setComponentID("root");
        root.setSize(100, 100);

        jive::LookAndFeel lookAndFeel{ root };
        lookAndFeel.addStyles(jive::StyleSelector{ "#root" },
                              jive::Styles{}.withBackground(juce::Colours::black));

        juce::Component target;
        target.setBounds(40, 40, 20, 20);
        root.addAndMakeVisible(target);

        jive::ShadowComponent shadow{ target };
        shadow.setShadow(jive::Shadow{}
                             .setColour(juce::Colours::red)
                             .setBlurRadius(2.0f)
                             .setSpreadRadius(5.0f));

        test.expectEquals(shadow.getBounds(), juce::Rectangle{ 33, 33, 34, 34 });

        const auto snapshot = root.createComponentSnapshot(root.getLocalBounds());
        test.expect(snapshot.getPixelAt(62, 50) == juce::Colours::red);
        test.expect(snapshot.getPixelAt(50, 37) == juce::Colours::red);
    },
};
#endif
