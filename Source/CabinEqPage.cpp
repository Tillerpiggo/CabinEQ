/*
  ==============================================================================

    CabinEqPage.cpp
    Created: 27 Jul 2024 9:31:27pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CabinEqPage.h"
#include "CrossfeedControl.h"
#include "EqPresetFile.h"
#include "Format.h"

//==============================================================================
/// A square button that draws one of the Icons.
class CabinEqPage::IconButton : public juce::Button
{
public:
    IconButton (const juce::String& name, juce::Path icon, const juce::String& tooltip)
        : juce::Button (name), icon (std::move (icon))
    {
        setTooltip (tooltip);
    }

    juce::Colour onColour = Theme::accentBright;
    bool drawsBackgroundWhenOn = true;

    void paintButton (juce::Graphics& g, bool isHighlighted, bool isDown) override
    {
        auto bounds = getLocalBounds().toFloat();
        const bool on = getToggleState();

        if (isDown || isHighlighted || (on && drawsBackgroundWhenOn))
        {
            g.setColour (on && drawsBackgroundWhenOn ? onColour.withAlpha (0.16f) : (isDown ? Theme::raisedHover : Theme::raised));
            g.fillRoundedRectangle (bounds, Theme::cornerRadius);
        }

        auto colour = ! isEnabled() ? Theme::textFaint.withAlpha (0.6f)
                    : on ? onColour
                    : isHighlighted ? Theme::text : Theme::textDim;
        Icons::draw (g, icon, bounds.withSizeKeepingCentre (18.0f, 18.0f), colour, 1.7f);
    }

private:
    juce::Path icon;
};

//==============================================================================
/// The two layers, in the order the sound goes through them:  ● Bands › ● Curve
/// A layer's dot turns it on or off; its name picks it as the one the graph edits.
class CabinEqPage::LayerSwitch : public juce::Component,
                                 public juce::SettableTooltipClient
{
public:
    static constexpr int width = 150, height = 30;

    std::function<void (bool curve)> onToggle, onEdit;

    void setState (bool newBandsOn, bool newCurveOn, bool newEditingCurve)
    {
        if (on[0] == newBandsOn && on[1] == newCurveOn && editingCurve == newEditingCurve)
            return;
        on[0] = newBandsOn;
        on[1] = newCurveOn;
        editingCurve = newEditingCurve;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        g.setColour (Theme::background);
        g.fillRoundedRectangle (getLocalBounds().toFloat(), 8.0f);

        for (int layer = 0; layer < 2; ++layer)
        {
            const auto area = segment (layer);
            const bool isOn = on[layer], isEdited = isOn && editingCurve == (layer == 1);
            const bool isHovered = hovered.layer == layer;

            if (isEdited || (isHovered && ! hovered.onDot))
            {
                g.setColour (isEdited ? Theme::raised : Theme::raised.withAlpha (0.5f));
                g.fillRoundedRectangle (area, 6.0f);
            }

            // The dot: lit when the layer's playing
            const auto dot = dotArea (layer).withSizeKeepingCentre (8.0f, 8.0f);
            if (isOn)
            {
                g.setColour (Theme::accent.withAlpha (isHovered && hovered.onDot ? 0.4f : 0.22f));
                g.fillEllipse (dot.expanded (3.5f));
                g.setColour (Theme::accentBright);
                g.fillEllipse (dot);
            }
            else
            {
                g.setColour (isHovered && hovered.onDot ? Theme::text : Theme::textFaint);
                g.drawEllipse (dot.reduced (0.5f), 1.4f);
            }

            g.setColour (isEdited ? Theme::text : isOn ? Theme::textDim : Theme::textFaint);
            g.setFont (Theme::font (13.0f, isEdited));
            g.drawText (layer == 0 ? "Bands" : "Curve", area.withTrimmedLeft (23.0f), juce::Justification::centredLeft);
        }

        // The chevron between them: the bands feed the curve, when both are on
        const auto middle = getLocalBounds().toFloat().getCentre();
        juce::Path chevron;
        chevron.startNewSubPath (middle.x - 2.0f, middle.y - 4.0f);
        chevron.lineTo (middle.x + 2.0f, middle.y);
        chevron.lineTo (middle.x - 2.0f, middle.y + 4.0f);
        g.setColour (on[0] && on[1] ? Theme::accentBright.withAlpha (0.8f) : Theme::textFaint.withAlpha (0.6f));
        g.strokePath (chevron, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    void mouseMove (const juce::MouseEvent& event) override
    {
        const auto now = partAt (event.position);
        if (now.layer != hovered.layer || now.onDot != hovered.onDot)
        {
            hovered = now;
            const juce::String name (now.layer == 0 ? "bands" : "curve");
            setTooltip (now.layer < 0 ? juce::String()
                        : now.onDot ? (on[now.layer] ? "Turn the " + name + " off" : "Turn the " + name + " on")
                        : now.layer == 0 ? "Edit the bands: bells, shelves and cuts. They play first."
                                         : "Edit the curve: a smooth line through your points, on top of the bands. Tab swaps.");
            setMouseCursor (now.layer >= 0 ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::NormalCursor);
            repaint();
        }
    }

    void mouseExit (const juce::MouseEvent&) override
    {
        hovered = {};
        repaint();
    }

    void mouseUp (const juce::MouseEvent& event) override
    {
        const auto part = partAt (event.position);
        if (part.layer < 0 || ! getLocalBounds().toFloat().contains (event.position))
            return;
        if (part.onDot ? onToggle != nullptr : onEdit != nullptr)
            (part.onDot ? onToggle : onEdit) (part.layer == 1);
    }

private:
    struct Part
    {
        int layer = -1;
        bool onDot = false;
    };

    juce::Rectangle<float> segment (int layer) const
    {
        auto area = getLocalBounds().toFloat().reduced (2.0f);
        const float each = (area.getWidth() - 14.0f) * 0.5f; // 14 for the chevron
        return layer == 0 ? area.removeFromLeft (each) : area.removeFromRight (each);
    }

    juce::Rectangle<float> dotArea (int layer) const { return segment (layer).removeFromLeft (23.0f); }

    Part partAt (juce::Point<float> position) const
    {
        for (int layer = 0; layer < 2; ++layer)
            if (segment (layer).contains (position))
                return { layer, dotArea (layer).contains (position) };
        return {};
    }

    bool on[2] { true, false };
    bool editingCurve = false;
    Part hovered;
};

//==============================================================================
CabinEqPage::CabinEqPage (CabinEqAudioProcessor& p)
    : processor (p), profileList (p), graph (p), inspector (p), calibrationPanel (p),
      autoGainAttachment (p.parameters, ParamIDs::autoGain, autoGainToggle)
{
    addAndMakeVisible (profileList);
    addAndMakeVisible (graph);
    graph.setComponentID ("graph");
    addAndMakeVisible (inspector);

    profileList.onImportFile = [this] { importFile(); };
    profileList.onPaste = [this] { pasteProfile(); };
    profileList.onExportFile = [this] (const juce::String& name) { exportProfile (name); };
    profileList.onCopy = [this] (const juce::String& name) { copyProfile (name); };

    graph.onSelectionChanged = [this] { updateInspector(); updateLayerSwitch(); };
    inspector.layerBaseDb = [this] (float frequency) { return graph.layerBaseDb (frequency); };
    inspector.onEdited = [this] { graph.refresh(); };
    inspector.onDeleteClicked = [this] { graph.deleteSelectedBands(); };

    // Top bar
    undoButton = std::make_unique<IconButton> ("Undo", Icons::undo(), "Undo");
    redoButton = std::make_unique<IconButton> ("Redo", Icons::redo(), "Redo");
    calibrationButton = std::make_unique<IconButton> ("Calibration", Icons::grid(), "Calibration sounds: a grid of noise bursts to check the EQ by ear");
    crossfeedButton = std::make_unique<IconButton> ("Crossfeed", Icons::crossfeed(), "Crossfeed");
    monoButton = std::make_unique<IconButton> ("Mono", Icons::mono(), "Mono");
    settingsButton = std::make_unique<IconButton> ("Audio settings", Icons::settings(), "Audio devices");
    powerButton = std::make_unique<IconButton> ("EQ on", Icons::power(), "Turn the EQ off to compare");

    undoButton->onClick = [this] { processor.undo(); };
    redoButton->onClick = [this] { processor.redo(); };
    crossfeedButton->onClick = [this] { showCrossfeed(); };
    monoButton->onClick = [this]
    {
        auto* mono = processor.parameters.getParameter (ParamIDs::mono);
        mono->beginChangeGesture();
        mono->setValueNotifyingHost (mono->getValue() >= 0.5f ? 0.0f : 1.0f);
        mono->endChangeGesture();
        updateTopBar();
    };
    calibrationButton->onClick = [this] { setCalibrationShown (! calibrationPanel.isVisible()); };
    calibrationPanel.onCloseClicked = [this] { setCalibrationShown (false); };
    calibrationPanel.onModeChanged = [this] { graph.setCalibrationSpotsVisible (calibrationPanel.isVisible() && calibrationPanel.isShowingSpots()); };
    addChildComponent (calibrationPanel);
    settingsButton->onClick = [this] { processor.showAudioSettingsDialog(); };
    powerButton->onClick = [this]
    {
        auto* bypass = processor.parameters.getParameter (ParamIDs::bypass);
        bypass->beginChangeGesture();
        bypass->setValueNotifyingHost (bypass->getValue() >= 0.5f ? 0.0f : 1.0f);
        bypass->endChangeGesture();
        updateTopBar();
    };

    for (auto* button : { undoButton.get(), redoButton.get(), calibrationButton.get(), crossfeedButton.get(), monoButton.get(), settingsButton.get(), powerButton.get() })
        addAndMakeVisible (button);
    settingsButton->setVisible (processor.isStandalone());

    preampField.format = [] (double v) { return Format::gain (v); };
    preampField.setTooltip ("Gain before the EQ, for this profile. Turn it down if boosts make things clip.");
    preampField.onGestureStart = [this] { processor.getUndoManager().beginNewTransaction ("Change preamp"); };
    preampField.onValueChange = [this] (double v) { processor.getSelectedProfile().setVolume ((float) v); };
    addAndMakeVisible (preampField);

    // Master volume: a parameter, so hosts can automate it. It applies with the EQ on or off.
    volumeField.format = [] (double v) { return Format::gain (v); };
    volumeField.setTooltip ("Master volume, for everything. It can boost. Nothing limits it, so turn it down if the sound distorts.");
    volumeField.onGestureStart = [this] { processor.parameters.getParameter (ParamIDs::volume)->beginChangeGesture(); };
    volumeField.onGestureEnd = [this] { processor.parameters.getParameter (ParamIDs::volume)->endChangeGesture(); };
    volumeField.onValueChange = [this] (double v)
    {
        auto* parameter = processor.parameters.getParameter (ParamIDs::volume);
        parameter->setValueNotifyingHost (parameter->convertTo0to1 ((float) v));
    };
    addAndMakeVisible (volumeField);

    // The two layers: bands, then a curve on top
    layerSwitch = std::make_unique<LayerSwitch>();
    layerSwitch->onToggle = [this] (bool curve) { toggleLayer (curve); };
    layerSwitch->onEdit = [this] (bool curve) { editLayer (curve); };
    addAndMakeVisible (*layerSwitch);

    autoGainToggle.setTooltip ("Turns the output down by as much as the EQ makes music louder, so switching the EQ on and off is a fair comparison.");
    autoGainToggle.onStateChange = [this] { updateTopBar(); };
    addAndMakeVisible (autoGainToggle);

    calibrationPanel.setVisible (processor.parameters.state.getProperty ("showCalibration", false));
    calibrationButton->setToggleState (calibrationPanel.isVisible(), juce::dontSendNotification);
    graph.setCalibrationSpotsVisible (calibrationPanel.isVisible() && calibrationPanel.isShowingSpots());

    processor.stateChanged.addChangeListener (this);
    refreshAll();
    startTimerHz (10);
}

CabinEqPage::~CabinEqPage()
{
    processor.stateChanged.removeChangeListener (this);

    // It's a separate window with attachments to the parameters, so it mustn't outlive us
    delete crossfeedBox.getComponent();
}

//==============================================================================
void CabinEqPage::changeListenerCallback (juce::ChangeBroadcaster*)
{
    refreshAll();
}

void CabinEqPage::timerCallback()
{
    // Bypass and auto gain can also change from the host
    updateTopBar();
}

void CabinEqPage::refreshAll()
{
    profileList.refresh();
    graph.refresh();
    updateInspector();
    updateTopBar();
    repaint (getLocalBounds().withTrimmedLeft (Theme::sidebarWidthFor (getWidth())).removeFromTop (Theme::topBarHeight));
}

void CabinEqPage::updateInspector()
{
    const int id = graph.getFocusedBandId();
    int number = 0;
    const auto bands = processor.getSelectedBandProfile().getBands();
    for (size_t i = 0; i < bands.size(); ++i)
        if (bands[i].id == id)
            number = (int) i + 1;
    inspector.showBand (id, number, graph.getNumSelected());
}

void CabinEqPage::toggleLayer (bool curve)
{
    auto profile = processor.getSelectedProfile();
    const bool turningOn = curve ? ! profile.getCurveOn() : ! profile.getBandsOn();
    const bool otherOn = curve ? profile.getBandsOn() : profile.getCurveOn();

    processor.getUndoManager().beginNewTransaction ((turningOn ? "Turn on " : "Turn off ") + juce::String (curve ? "curve" : "bands"));
    if (curve)
        profile.setCurveOn (turningOn);
    else
        profile.setBandsOn (turningOn);

    // Something always plays: turning off the only layer hands over to the other
    if (! turningOn && ! otherOn)
    {
        if (curve)
            profile.setBandsOn (true);
        else
            profile.setCurveOn (true);
    }

    // The one you just turned on is the one you're editing
    if (turningOn)
        processor.setEditingCurve (curve);
    refreshAll();
}

void CabinEqPage::editLayer (bool curve)
{
    auto profile = processor.getSelectedProfile();
    if (! (curve ? profile.getCurveOn() : profile.getBandsOn()))
    {
        toggleLayer (curve); // editing a layer you can't hear makes no sense: on it goes
        return;
    }
    processor.setEditingCurve (curve);
    refreshAll();
}

void CabinEqPage::updateLayerSwitch()
{
    const auto profile = processor.getSelectedProfile();
    layerSwitch->setState (profile.getBandsOn(), profile.getCurveOn(), processor.isEditingCurve());
}

void CabinEqPage::updateTopBar()
{
    updateLayerSwitch();

    const bool bypassed = processor.parameters.getParameter (ParamIDs::bypass)->getValue() >= 0.5f;
    powerButton->setToggleState (! bypassed, juce::dontSendNotification);
    powerButton->setTooltip (bypassed ? "The EQ is off. Click to turn it on." : "The EQ is on. Click to turn it off and compare.");
    graph.setBypassed (bypassed);

    auto& undo = processor.getUndoManager();
    undoButton->setEnabled (undo.canUndo());
    redoButton->setEnabled (undo.canRedo());
    undoButton->setTooltip (undo.canUndo() ? "Undo " + undo.getUndoDescription().toLowerCase() : "Undo");
    redoButton->setTooltip (undo.canRedo() ? "Redo " + undo.getRedoDescription().toLowerCase() : "Redo");

    const bool crossfeedOn = processor.parameters.getRawParameterValue (ParamIDs::crossfeed)->load() >= 0.5f;
    crossfeedButton->setToggleState (crossfeedOn, juce::dontSendNotification);
    const bool monoOn = processor.parameters.getRawParameterValue (ParamIDs::mono)->load() >= 0.5f;
    monoButton->setToggleState (monoOn, juce::dontSendNotification);
    monoButton->setTooltip (monoOn ? "Mono is on: both ears get the same sound. Click for stereo."
                                   : "Mono: play left and right summed, the same in both ears");

    preampField.setValue (processor.getSelectedProfile().getVolume());
    volumeField.setValue (processor.parameters.getRawParameterValue (ParamIDs::volume)->load());

    auto autoGainText = processor.isAutoGainOn() ? "Auto gain  " + Format::gain (processor.getAutoGainDb()) : juce::String ("Auto gain");
    if (autoGainToggle.getButtonText() != autoGainText)
        autoGainToggle.setButtonText (autoGainText);
}

//==============================================================================
void CabinEqPage::paint (juce::Graphics& g)
{
    g.fillAll (Theme::background);

    auto topBar = getLocalBounds().withTrimmedLeft (Theme::sidebarWidthFor (getWidth())).removeFromTop (Theme::topBarHeight);
    g.setColour (Theme::panel);
    g.fillRect (topBar);
    g.setColour (Theme::border);
    g.drawHorizontalLine (topBar.getBottom() - 1, (float) topBar.getX(), (float) topBar.getRight());

    // The profile's name
    auto title = topBar.reduced (20, 0).withWidth (std::max (0, layerSwitch->getX() - topBar.getX() - 32));
    g.setColour (Theme::text);
    g.setFont (Theme::font (17.0f, true));
    g.drawText (processor.getProfiles().getSelectedProfileName(), title, juce::Justification::centredLeft, true);
}

void CabinEqPage::paintOverChildren (juce::Graphics& g)
{
    if (isDraggingFiles)
    {
        auto area = getLocalBounds().toFloat().reduced (8.0f);
        g.setColour (Theme::background.withAlpha (0.75f));
        g.fillRoundedRectangle (area, 10.0f);
        g.setColour (Theme::accent);
        g.drawRoundedRectangle (area, 10.0f, 2.0f);
        g.setFont (Theme::font (16.0f, true));
        g.drawText ("Drop an Equalizer APO or AutoEQ file to import it", area, juce::Justification::centred);
    }
}

void CabinEqPage::resized()
{
    auto area = getLocalBounds();
    profileList.setBounds (area.removeFromLeft (Theme::sidebarWidthFor (getWidth())));

    auto topBar = area.removeFromTop (Theme::topBarHeight).reduced (12, 0);
    auto placeRight = [&topBar] (juce::Component& component, int width, int height, int gap = 6)
    {
        component.setBounds (topBar.removeFromRight (width).withSizeKeepingCentre (width, height));
        topBar.removeFromRight (gap);
    };

    placeRight (*powerButton, 36, 36, 4);
    if (settingsButton->isVisible())
        placeRight (*settingsButton, 36, 36, 4);
    placeRight (*monoButton, 36, 36, 4);
    placeRight (*crossfeedButton, 36, 36, 4);
    placeRight (*calibrationButton, 36, 36, 14);
    placeRight (volumeField, 92, 38, 12);
    placeRight (autoGainToggle, 148, 30, 10);
    placeRight (preampField, 92, 38, 18);
    placeRight (*redoButton, 32, 32, 2);
    placeRight (*undoButton, 32, 32, 14);
    placeRight (*layerSwitch, LayerSwitch::width, LayerSwitch::height, 0);

    if (calibrationPanel.isVisible())
        calibrationPanel.setBounds (area.removeFromBottom (CalibrationPanel::preferredHeight));
    inspector.setBounds (area.removeFromBottom (Theme::inspectorHeight));
    graph.setBounds (area);
}

bool CabinEqPage::keyPressed (const juce::KeyPress& key)
{
    const auto mods = key.getModifiers();
    const bool command = mods.isCommandDown() || mods.isCtrlDown();
    const int code = key.getKeyCode();

    if (command && (code == 'Z' || code == 'z'))
    {
        if (mods.isShiftDown())
            processor.redo();
        else
            processor.undo();
        return true;
    }
    if (command && (code == 'Y' || code == 'y'))
    {
        processor.redo();
        return true;
    }
    return false;
}

//==============================================================================
bool CabinEqPage::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (const auto& path : files)
        if (juce::File (path).hasFileExtension ("txt;cfg"))
            return true;
    return false;
}

void CabinEqPage::fileDragEnter (const juce::StringArray&, int, int)
{
    isDraggingFiles = true;
    repaint();
}

void CabinEqPage::fileDragExit (const juce::StringArray&)
{
    isDraggingFiles = false;
    repaint();
}

void CabinEqPage::filesDropped (const juce::StringArray& paths, int, int)
{
    isDraggingFiles = false;
    repaint();

    juce::Array<juce::File> files;
    for (const auto& path : paths)
        files.add (juce::File (path));
    importFiles (files);
}

//==============================================================================
void CabinEqPage::importFile()
{
    fileChooser = std::make_unique<juce::FileChooser> ("Import an Equalizer APO or AutoEQ file",
                                                       juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile ("Downloads"),
                                                       EqPresetFile::fileExtensions);
    juce::Component::SafePointer<CabinEqPage> safeThis (this);
    fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles
                                  | juce::FileBrowserComponent::canSelectMultipleItems,
                              [safeThis] (const juce::FileChooser& chooser)
                              {
                                  if (safeThis != nullptr)
                                      safeThis->importFiles (chooser.getResults());
                              });
}

void CabinEqPage::importFiles (const juce::Array<juce::File>& files)
{
    for (const auto& file : files)
        if (file.existsAsFile())
            importText (file.loadFileAsString(), file.getFileNameWithoutExtension()
                                                     .replace ("ParametricEQ", "").replace ("_", " ").trim());
}

void CabinEqPage::importText (const juce::String& text, const juce::String& profileName)
{
    auto bandProfile = EqPresetFile::parse (text);
    if (! bandProfile.has_value())
    {
        showMessage ("Nothing to import",
                     "That doesn't look like an Equalizer APO or AutoEQ file. They have lines like\n"
                     "\"Filter 1: ON PK Fc 1000 Hz Gain -3 dB Q 1.4\".");
        return;
    }

    processor.getUndoManager().beginNewTransaction ("Import profile");
    auto profile = processor.getProfiles().addProfile (profileName.isNotEmpty() ? profileName : "Imported", *bandProfile);
    processor.selectProfile (profile.getName());
}

void CabinEqPage::exportProfile (const juce::String& profileName)
{
    auto profile = processor.getProfiles().getProfileNamed (profileName);
    if (! profile.has_value())
        return;

    const auto text = EqPresetFile::write (profile->getBandProfile());
    auto defaultFile = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                           .getChildFile (juce::File::createLegalFileName (profileName) + ".txt");

    fileChooser = std::make_unique<juce::FileChooser> ("Export \"" + profileName + "\"", defaultFile, "*.txt");
    juce::Component::SafePointer<CabinEqPage> safeThis (this);
    fileChooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting,
                              [safeThis, text] (const juce::FileChooser& chooser)
                              {
                                  auto file = chooser.getResult();
                                  if (file == juce::File() || safeThis == nullptr)
                                      return;
                                  if (! file.replaceWithText (text))
                                      safeThis->showMessage ("Couldn't save", "CabinEQ couldn't write to " + file.getFullPathName() + ".");
                              });
}

void CabinEqPage::copyProfile (const juce::String& profileName)
{
    if (auto profile = processor.getProfiles().getProfileNamed (profileName))
        juce::SystemClipboard::copyTextToClipboard (EqPresetFile::write (profile->getBandProfile()));
}

void CabinEqPage::pasteProfile()
{
    importText (juce::SystemClipboard::getTextFromClipboard(), "Pasted");
}

void CabinEqPage::showCrossfeed()
{
    auto content = std::make_unique<CrossfeedControl> (processor.parameters);
    content->setLookAndFeel (&getLookAndFeel());
    auto& box = juce::CallOutBox::launchAsynchronously (std::move (content), crossfeedButton->getScreenBounds(), nullptr);
    box.setLookAndFeel (&getLookAndFeel());
    crossfeedBox = &box;
}

void CabinEqPage::setCalibrationShown (bool shouldShow)
{
    if (shouldShow == calibrationPanel.isVisible())
        return;

    if (! shouldShow)
        calibrationPanel.stop(); // hidden sounds would just be confusing

    calibrationPanel.setVisible (shouldShow);
    calibrationButton->setToggleState (shouldShow, juce::dontSendNotification);
    graph.setCalibrationSpotsVisible (shouldShow && calibrationPanel.isShowingSpots());
    processor.parameters.state.setProperty ("showCalibration", shouldShow, nullptr);

    // Make room, so the graph doesn't get squashed, and give it back afterwards
    if (auto* editor = findParentComponentOfClass<juce::AudioProcessorEditor>())
    {
        const int change = CalibrationPanel::preferredHeight;
        if (shouldShow && getHeight() - change < 560)
        {
            editor->setSize (editor->getWidth(), editor->getHeight() + change);
            grewForCalibration = true;
        }
        else if (! shouldShow && grewForCalibration)
        {
            editor->setSize (editor->getWidth(), editor->getHeight() - change);
            grewForCalibration = false;
        }
    }
    resized();
}

void CabinEqPage::showMessage (const juce::String& title, const juce::String& message)
{
    juce::AlertWindow::showAsync (juce::MessageBoxOptions()
                                      .withIconType (juce::MessageBoxIconType::NoIcon)
                                      .withTitle (title)
                                      .withMessage (message)
                                      .withButton ("OK")
                                      .withAssociatedComponent (this),
                                  nullptr);
}
