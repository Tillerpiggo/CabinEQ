#include "AppView.h"

namespace
{
namespace Palette
{
    const juce::Colour background { 0xff0e0f12 };
    const juce::Colour bar        { 0xff15171b };
    const juce::Colour card       { 0xff1a1d22 };
    const juce::Colour raised     { 0xff23272e };
    const juce::Colour border     { 0xff2a2e36 };
    const juce::Colour text       { 0xffe8eaed };
    const juce::Colour dim        { 0xff9aa0a8 };
    const juce::Colour faint      { 0xff5f6570 };
    const juce::Colour accent     { 0xff9d8cff };
    const juce::Colour good       { 0xff5fd38d };
    const juce::Colour caution    { 0xffffb454 };
    const juce::Colour bad        { 0xffff6b6b };
}

juce::Colour colourFor (Engine::Status::Level level)
{
    switch (level)
    {
        case Engine::Status::Level::working: return Palette::good;
        case Engine::Status::Level::idle:    return Palette::good.withAlpha (0.6f);
        case Engine::Status::Level::busy:    return Palette::accent;
        case Engine::Status::Level::warning: return Palette::caution;
        case Engine::Status::Level::problem: return Palette::bad;
    }
    return Palette::dim;
}

juce::Font font (float height, bool bold = false)
{
    juce::Font f { juce::FontOptions (height) };
    return bold ? f.boldened() : f;
}

class AppLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AppLookAndFeel()
    {
        using namespace juce;
        setColour (ResizableWindow::backgroundColourId, Palette::background);
        setColour (TextButton::buttonColourId, Palette::raised);
        setColour (TextButton::buttonOnColourId, Palette::accent);
        setColour (TextButton::textColourOffId, Palette::text);
        setColour (TextButton::textColourOnId, Palette::background);
        setColour (ComboBox::backgroundColourId, Palette::raised);
        setColour (ComboBox::textColourId, Palette::text);
        setColour (ComboBox::outlineColourId, Palette::border);
        setColour (ComboBox::arrowColourId, Palette::dim);
        setColour (PopupMenu::backgroundColourId, Palette::card);
        setColour (PopupMenu::textColourId, Palette::text);
        setColour (PopupMenu::highlightedBackgroundColourId, Palette::raised);
        setColour (PopupMenu::highlightedTextColourId, Palette::text);
        setColour (PopupMenu::headerTextColourId, Palette::dim);
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour& colour, bool highlighted, bool down) override
    {
        auto c = colour;
        if (! button.isEnabled()) c = c.withMultipliedAlpha (0.4f);
        else if (down) c = c.brighter (0.15f);
        else if (highlighted) c = c.brighter (0.08f);
        g.setColour (c);
        g.fillRoundedRectangle (button.getLocalBounds().toFloat().reduced (0.5f), 6.0f);
    }

    juce::Font getTextButtonFont (juce::TextButton&, int) override { return font (13.0f); }
    juce::Font getComboBoxFont (juce::ComboBox&) override { return font (13.0f); }
    juce::Font getPopupMenuFont() override { return font (13.0f); }

    void drawComboBox (juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<float> (0, 0, (float) width, (float) height).reduced (0.5f);
        g.setColour (box.isMouseOver (true) ? Palette::raised.brighter (0.08f) : Palette::raised);
        g.fillRoundedRectangle (bounds, 6.0f);

        juce::Path chevron;
        const float cx = bounds.getRight() - 12.0f, cy = bounds.getCentreY();
        chevron.startNewSubPath (cx - 4.0f, cy - 2.0f);
        chevron.lineTo (cx, cy + 2.0f);
        chevron.lineTo (cx + 4.0f, cy - 2.0f);
        g.setColour (Palette::dim);
        g.strokePath (chevron, juce::PathStrokeType (1.4f));
    }
};

// What a button should say and do, or nothing
struct Action
{
    juce::String text;
    std::function<void()> run;
};
}

//==============================================================================
class AppView::StatusBar : public juce::Component
{
public:
    StatusBar (Engine& e, std::function<void()> onSetupClicked) : engine (e)
    {
        outputBox.setTooltip ("Where CabinEQ sends the processed audio");
        outputBox.onChange = [this]
        {
            if (updatingBox)
                return;
            const int index = outputBox.getSelectedItemIndex();
            if (index >= 0 && index < (int) boxUIDs.size())
                engine.setOutputChoice (boxUIDs[(size_t) index]);
        };
        addAndMakeVisible (outputBox);

        testButton.setTooltip ("Plays a short burst of noise and checks it went through CabinEQ");
        testButton.onClick = [this] { engine.playTestSound(); };
        addAndMakeVisible (testButton);

        setupButton.onClick = std::move (onSetupClicked);
        addAndMakeVisible (setupButton);
    }

    void setSetupShowing (bool isShowing)
    {
        setupButton.setButtonText (isShowing ? "Show EQ" : "Setup");
    }

    void refresh()
    {
        testButton.setEnabled (engine.isConnected() && engine.getTestState() != Engine::TestState::playing);
        refreshOutputBox();
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (Palette::bar);
        g.setColour (Palette::border);
        g.drawHorizontalLine (getHeight() - 1, 0.0f, (float) getWidth());

        const auto status = engine.getStatus();
        auto area = getLocalBounds().withRight (meterArea.getX() - 12).reduced (16, 0);

        // A dot that pulses gently while audio's going through
        auto dot = area.removeFromLeft (12).toFloat().withSizeKeepingCentre (10.0f, 10.0f);
        auto colour = colourFor (status.level);
        if (status.level == Engine::Status::Level::working)
        {
            const float glow = juce::jmap (juce::jlimit (-60.0f, 0.0f, engine.getOutputLevel()), -60.0f, 0.0f, 0.15f, 0.5f);
            g.setColour (colour.withAlpha (glow));
            g.fillEllipse (dot.expanded (4.0f));
        }
        g.setColour (colour);
        g.fillEllipse (dot);
        area.removeFromLeft (10);

        g.setColour (Palette::text);
        g.setFont (font (14.0f, true));
        g.drawText (status.headline, area.removeFromTop (getHeight() / 2 + 2).withTrimmedTop (6), juce::Justification::bottomLeft, true);
        g.setColour (Palette::dim);
        g.setFont (font (12.0f));
        g.drawText (status.detail, area.withTrimmedTop (1), juce::Justification::topLeft, true);

        // In and out meters: seeing both move is the quickest proof it's working
        auto meters = meterArea.toFloat();
        for (auto [label, level] : { std::pair { "IN", engine.getInputLevel() }, std::pair { "OUT", engine.getOutputLevel() } })
        {
            auto row = meters.removeFromTop (meters.getHeight() * 0.5f);
            if (juce::String (label) == "IN")
                row = row.withTrimmedBottom (2.0f);
            g.setColour (Palette::faint);
            g.setFont (font (9.5f, true));
            g.drawText (label, row.removeFromLeft (26.0f), juce::Justification::centredLeft);

            auto track = row.withSizeKeepingCentre (row.getWidth(), 5.0f);
            g.setColour (Palette::raised);
            g.fillRoundedRectangle (track, 2.5f);
            const float proportion = juce::jmap (juce::jlimit (-60.0f, 0.0f, level), -60.0f, 0.0f, 0.0f, 1.0f);
            g.setColour (level > -1.0f ? Palette::bad : Palette::good.withAlpha (0.85f));
            g.fillRoundedRectangle (track.withWidth (track.getWidth() * proportion), 2.5f);
        }
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced (12, 10);
        setupButton.setBounds (area.removeFromRight (84));
        area.removeFromRight (8);
        testButton.setBounds (area.removeFromRight (60));
        area.removeFromRight (8);
        outputBox.setBounds (area.removeFromRight (juce::jlimit (150, 260, getWidth() / 4)));
        area.removeFromRight (16);
        meterArea = area.removeFromRight (110).reduced (0, 4);
    }

private:
    void refreshOutputBox()
    {
        // Rebuild only when something changed, so an open menu doesn't get pulled out from under you
        std::vector<juce::String> uids { juce::String() };
        juce::StringArray names;
        const auto playingTo = engine.getPlayingToName();
        names.add ("Follow Mac output" + (engine.getOutputChoice().isEmpty() && playingTo.isNotEmpty() ? " (" + playingTo + ")" : juce::String()));
        for (const auto& device : engine.getDevices())
        {
            if (device.isVirtual)
                continue;
            uids.push_back (device.uid);
            names.add (device.name);
        }

        if (uids != boxUIDs || names != boxNames)
        {
            const juce::ScopedValueSetter<bool> guard (updatingBox, true);
            boxUIDs = uids;
            boxNames = names;
            outputBox.clear (juce::dontSendNotification);
            outputBox.addItemList (names, 1);
        }

        const juce::ScopedValueSetter<bool> guard (updatingBox, true);
        const auto found = std::find (boxUIDs.begin(), boxUIDs.end(), engine.getOutputChoice());
        outputBox.setSelectedItemIndex (found != boxUIDs.end() ? (int) std::distance (boxUIDs.begin(), found) : 0, juce::dontSendNotification);
    }

    Engine& engine;
    juce::ComboBox outputBox;
    juce::TextButton testButton { "Test" }, setupButton { "Setup" };
    std::vector<juce::String> boxUIDs;
    juce::StringArray boxNames;
    bool updatingBox = false;
    juce::Rectangle<int> meterArea;
};

//==============================================================================
class AppView::SetupPanel : public juce::Component
{
public:
    SetupPanel (Engine& e, std::function<void()> onDone) : engine (e)
    {
        doneButton.setColour (juce::TextButton::buttonColourId, Palette::accent);
        doneButton.setColour (juce::TextButton::textColourOffId, Palette::background);
        doneButton.onClick = std::move (onDone);
        addAndMakeVisible (doneButton);

        for (auto& row : rows)
        {
            row.button.onClick = [&row] { if (row.action.run) row.action.run(); };
            addChildComponent (row.button);
        }
    }

    void refresh()
    {
        using Permission = SystemAudioTap::Permission;
        auto& [plugin, permission, output, others, test] = rows;

        // 1. The plugin
        if (engine.getPlugin() != nullptr && engine.isUsingBundledPlugin())
        {
            // Built in. Offer to put it where DAWs look, too
            if (engine.isPluginInstalledForDAWs())
                plugin.set (State::done, "CabinEQ", "Built in, and installed for your DAWs too.", {});
            else
                plugin.set (State::done, "CabinEQ", "Built in. To use CabinEQ in a DAW (Logic, Ableton...) as well, add it to your plug-ins.",
                            { "Add to DAWs", [this] { engine.installPluginForDAWs(); refresh(); } });
        }
        else if (auto* p = engine.getPlugin())
            plugin.set (State::done, "CabinEQ", "Loaded from " + engine.getPluginFile().getFullPathName().replace (juce::File::getSpecialLocation (juce::File::userHomeDirectory).getFullPathName(), "~")
                        + juce::String (p->getName() != "CabinEQ" ? " (" + p->getName() + ")" : ""),
                        { "Change...", [this] { choosePlugin(); } });
        else
            plugin.set (State::problem, "CabinEQ isn't installed",
                        engine.getPluginError() + ". Build and install it with update.sh in the CabinEQ folder, or choose the plugin yourself.",
                        { "Choose...", [this] { choosePlugin(); } });

        // 2. Permission to hear other apps
        const auto granted = engine.getPermission();
        if (engine.isAskingForPermission())
            permission.set (State::waiting, "Permission to hear your Mac's audio", "Answer the macOS prompt: choose Allow.", {});
        else if (granted == Permission::granted)
            permission.set (State::done, "Permission to hear your Mac's audio", "Allowed.", {});
        else if (granted == Permission::denied)
            permission.set (State::problem, "Permission to hear your Mac's audio",
                            "It's turned off. In System Settings, turn on CabinEQ System under System Audio Recording. It'll notice straight away.",
                            { "Open Settings", [] { AudioDevices::openAudioCapturePrivacySettings(); } });
        else if (engine.hasAskedForPermission())
            permission.set (State::caution, "Permission to hear your Mac's audio",
                            "macOS didn't say whether it's allowed. The test below will tell.", {});
        else
            permission.set (State::todo, "Permission to hear your Mac's audio",
                            "CabinEQ System needs it to EQ other apps. macOS will ask you once.",
                            { "Continue", [this] { engine.askForPermission(); } });

        // 3. Where it plays to
        const auto mac = engine.getMacOutput();
        const auto playingTo = engine.getPlayingToName();
        if (! engine.isConnected())
            output.set (granted == Permission::granted ? State::problem : State::waiting, "Output",
                        engine.getConnectError().isNotEmpty() ? engine.getConnectError() : "Waits for the permission above.", {});
        else if (mac.has_value() && mac->isVirtual)
            output.set (State::caution, "Output: " + playingTo,
                        "Your Mac's sound output is " + juce::String (mac->name) + ", a virtual device nobody can hear, so CabinEQ plays to "
                            + playingTo + " instead. But the volume keys still change " + juce::String (mac->name) + ".",
                        { "Fix", [this] { engine.makeMacOutput (engine.getPlayingToUID()); } });
        else if (mac.has_value() && juce::String (mac->uid) != engine.getPlayingToUID())
            output.set (State::done, "Output: " + playingTo,
                        "You picked it, so the volume keys (which control " + juce::String (mac->name) + ") won't change it.",
                        { "Use Mac output", [this] { engine.setOutputChoice ({}); } });
        else
            output.set (State::done, "Output: " + playingTo,
                        "It follows your Mac's output, so switching to headphones in the menu bar works as usual.", {});

        // 4. Other apps that would EQ it again
        const auto& conflicts = engine.getConflictingApps();
        if (conflicts.empty())
            others.set (State::done, "No other audio processors running", "Nothing else is changing your system audio.", {});
        else
            others.set (State::caution, juce::String (conflicts.front().name) + " is running",
                        "It processes system audio too, so you'd hear both at once.",
                        { "Quit it", [this, pid = conflicts.front().pid] { engine.quitConflictingApp (pid); } });

        // 5. Proof
        switch (engine.getTestState())
        {
            case Engine::TestState::notRun:
                test.set (State::todo, "Check it's working", "Plays a short burst of quiet noise from another app and checks it came through.",
                          { "Play test", [this] { engine.playTestSound(); } });
                break;
            case Engine::TestState::playing:
                test.set (State::waiting, "Check it's working", engine.getTestResult(), {});
                break;
            case Engine::TestState::passed:
                test.set (State::done, "It's working", engine.getTestResult(), { "Again", [this] { engine.playTestSound(); } });
                break;
            case Engine::TestState::failed:
                test.set (State::problem, "The test didn't come through", engine.getTestResult(), { "Again", [this] { engine.playTestSound(); } });
                break;
        }
        test.button.setEnabled (engine.isConnected());

        for (auto& row : rows)
        {
            row.button.setButtonText (row.action.text);
            row.button.setVisible (row.action.text.isNotEmpty());
        }
        resized();
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (Palette::background.withAlpha (0.82f));

        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.fillRoundedRectangle (card.toFloat().translated (0, 4), 12.0f);
        g.setColour (Palette::card);
        g.fillRoundedRectangle (card.toFloat(), 12.0f);
        g.setColour (Palette::border);
        g.drawRoundedRectangle (card.toFloat().reduced (0.5f), 12.0f, 1.0f);

        auto header = card.reduced (28, 0).withTrimmedTop (24).removeFromTop (48);
        g.setColour (Palette::text);
        g.setFont (font (19.0f, true));
        g.drawText ("Set up CabinEQ System", header.removeFromTop (26), juce::Justification::centredLeft);
        g.setColour (Palette::dim);
        g.setFont (font (13.0f));
        g.drawText ("EQ everything your Mac plays. No drivers or audio routing needed.", header, juce::Justification::centredLeft);

        for (auto& row : rows)
        {
            auto area = row.area;
            auto iconArea = area.removeFromLeft (34).removeFromTop (26).toFloat();
            drawStateIcon (g, row.state, iconArea.withSizeKeepingCentre (20.0f, 20.0f));

            auto textArea = area.withTrimmedRight (row.button.isVisible() ? row.button.getWidth() + 16 : 0);
            g.setColour (Palette::text);
            g.setFont (font (14.0f, true));
            g.drawText (row.title, textArea.removeFromTop (22), juce::Justification::centredLeft, true);
            g.setColour (Palette::dim);
            g.setFont (font (12.5f));
            g.drawFittedText (row.detail, textArea.withTrimmedTop (2), juce::Justification::topLeft, 3, 1.0f);
        }
    }

    void resized() override
    {
        card = getLocalBounds().withSizeKeepingCentre (std::min (620, getWidth() - 32), std::min (560, getHeight() - 24));
        auto area = card.reduced (28, 0).withTrimmedTop (24 + 48 + 16).withTrimmedBottom (20);
        doneButton.setBounds (area.removeFromBottom (34).removeFromRight (96));
        area.removeFromBottom (10);

        const int rowHeight = std::max (58, area.getHeight() / (int) rows.size());
        for (auto& row : rows)
        {
            row.area = area.removeFromTop (rowHeight);
            const int width = std::max (84, (int) juce::GlyphArrangement::getStringWidth (font (13.0f), row.action.text) + 28);
            row.button.setBounds (row.area.getRight() - width, row.area.getY(), width, 28);
        }
    }

private:
    enum class State { done, todo, waiting, caution, problem };

    struct Row
    {
        State state = State::todo;
        juce::String title, detail;
        Action action;
        juce::TextButton button;
        juce::Rectangle<int> area;

        void set (State s, const juce::String& t, const juce::String& d, Action a)
        {
            state = s; title = t; detail = d; action = std::move (a);
        }
    };

    static void drawStateIcon (juce::Graphics& g, State state, juce::Rectangle<float> area)
    {
        juce::Path mark;
        const auto c = area.getCentre();
        juce::Colour colour = Palette::faint;

        switch (state)
        {
            case State::done:
                colour = Palette::good;
                mark.startNewSubPath (c.x - 4.5f, c.y + 0.5f);
                mark.lineTo (c.x - 1.0f, c.y + 4.0f);
                mark.lineTo (c.x + 5.0f, c.y - 3.5f);
                break;
            case State::caution:
            case State::problem:
                colour = state == State::caution ? Palette::caution : Palette::bad;
                mark.startNewSubPath (c.x, c.y - 5.0f);
                mark.lineTo (c.x, c.y + 1.0f);
                mark.addEllipse (c.x - 0.9f, c.y + 3.6f, 1.8f, 1.8f);
                break;
            case State::waiting:
                colour = Palette::accent;
                mark.addCentredArc (c.x, c.y, 4.5f, 4.5f, 0.0f, 0.0f, juce::MathConstants<float>::pi * 1.5f, true);
                break;
            case State::todo:
                colour = Palette::dim;
                mark.addEllipse (c.x - 1.5f, c.y - 1.5f, 3.0f, 3.0f);
                break;
        }

        g.setColour (colour.withAlpha (0.18f));
        g.fillEllipse (area);
        g.setColour (colour);
        g.strokePath (mark, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    void choosePlugin()
    {
        chooser = std::make_unique<juce::FileChooser> ("Choose CabinEQ (or another plugin)",
                                                       engine.getPluginFile().getParentDirectory(), "*.vst3;*.component");
        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles
                                  | juce::FileBrowserComponent::canSelectDirectories,
                              [this, safe = juce::Component::SafePointer<SetupPanel> (this)] (const juce::FileChooser& c)
                              {
                                  if (safe != nullptr && c.getResult() != juce::File())
                                      engine.setPluginFile (c.getResult());
                              });
    }

    Engine& engine;
    std::array<Row, 5> rows;
    juce::TextButton doneButton { "Done" };
    juce::Rectangle<int> card;
    std::unique_ptr<juce::FileChooser> chooser;
};

//==============================================================================
AppView::AppView (Engine& e) : engine (e)
{
    lookAndFeel = std::make_unique<AppLookAndFeel>();
    setLookAndFeel (lookAndFeel.get());

    statusBar = std::make_unique<StatusBar> (engine, [this] { showSetup (! setupPanel->isVisible()); });
    addAndMakeVisible (*statusBar);

    setupPanel = std::make_unique<SetupPanel> (engine, [this]
    {
        engine.setHasSeenSetup();
        showSetup (false);
    });
    addChildComponent (*setupPanel);

    updateEditor();
    setSize (editor != nullptr ? editor->getWidth() : 900, (editor != nullptr ? editor->getHeight() : 560) + statusBarHeight);

    engine.onChange = [this] { engineChanged(); };

    // First launch, or something needs fixing: start with the checklist
    const auto level = engine.getStatus().level;
    showSetup (! engine.hasSeenSetup() || level == Engine::Status::Level::problem || level == Engine::Status::Level::warning);
}

AppView::~AppView()
{
    engine.onChange = nullptr;
    if (editor != nullptr)
        editor->removeComponentListener (this);
    editor.reset();
    setLookAndFeel (nullptr);
}

void AppView::showSetup (bool shouldShow)
{
    const bool show = shouldShow || engine.getPlugin() == nullptr;
    setupPanel->setVisible (show);

    // A plugin's editor is a native view, which always draws on top of anything JUCE draws,
    // so the checklist can only be seen with the editor out of the way
    if (editor != nullptr)
        editor->setVisible (! show);
    statusBar->setSetupShowing (show && engine.getPlugin() != nullptr);

    if (show)
    {
        setupPanel->refresh();
        setupPanel->toFront (false);
    }
}

void AppView::engineChanged()
{
    updateEditor();
    statusBar->refresh();

    // Bring the checklist up by itself when something new goes wrong
    const auto status = engine.getStatus();
    if (status.level == Engine::Status::Level::problem && status.headline != lastAutoShownProblem && engine.hasSeenSetup())
    {
        lastAutoShownProblem = status.headline;
        showSetup (true);
    }
    else if (status.level != Engine::Status::Level::problem)
    {
        lastAutoShownProblem = {};
    }

    if (setupPanel->isVisible())
        setupPanel->refresh();
}

void AppView::updateEditor()
{
    auto* processor = engine.getPlugin();
    if (processor == editorProcessor)
        return;

    if (editor != nullptr)
        editor->removeComponentListener (this);
    editor.reset();
    editorProcessor = processor;

    if (processor != nullptr)
    {
        editor.reset (processor->hasEditor() ? processor->createEditorIfNeeded() : new juce::GenericAudioProcessorEditor (*processor));
        if (editor != nullptr)
        {
            addChildComponent (*editor);
            editor->setVisible (! setupPanel->isVisible());
            editor->addComponentListener (this);
            setSize (editor->getWidth(), editor->getHeight() + statusBarHeight);
        }
    }

    setupPanel->toFront (false);
    resized();
}

void AppView::componentMovedOrResized (juce::Component& component, bool, bool wasResized)
{
    // The editor resized itself (from its own corner): make the window fit it
    if (wasResized && &component == editor.get() && ! isLayingOut)
        setSize (editor->getWidth(), editor->getHeight() + statusBarHeight);
}

void AppView::paint (juce::Graphics& g)
{
    g.fillAll (Palette::background);
}

void AppView::resized()
{
    const juce::ScopedValueSetter<bool> guard (isLayingOut, true);
    auto area = getLocalBounds();
    statusBar->setBounds (area.removeFromTop (statusBarHeight));
    if (editor != nullptr)
        editor->setBounds (area);
    setupPanel->setBounds (area);
}
