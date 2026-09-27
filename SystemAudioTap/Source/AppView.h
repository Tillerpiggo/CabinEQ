#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "Engine.h"

// CabinEQ System's window: a status bar that always says whether it's working, the plugin's
// editor, and a setup checklist that shows itself on first launch and when something's wrong.
class AppView : public juce::Component,
                private juce::ComponentListener
{
public:
    explicit AppView (Engine& engine);
    ~AppView() override;

    void showSetup (bool shouldShow);

    void paint (juce::Graphics&) override;
    void resized() override;

    static constexpr int statusBarHeight = 52;

private:
    class StatusBar;
    class SetupPanel;

    void engineChanged();
    void updateEditor();
    void componentMovedOrResized (juce::Component&, bool wasMoved, bool wasResized) override;

    Engine& engine;
    std::unique_ptr<juce::LookAndFeel_V4> lookAndFeel;
    std::unique_ptr<StatusBar> statusBar;
    std::unique_ptr<SetupPanel> setupPanel;
    std::unique_ptr<juce::AudioProcessorEditor> editor;
    juce::AudioProcessor* editorProcessor = nullptr;

    bool isLayingOut = false;
    juce::String lastAutoShownProblem;
};
