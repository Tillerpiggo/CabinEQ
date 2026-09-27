/*
  ==============================================================================

    ProfileList.cpp
    Created: 21 Feb 2025 9:30:33pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "ProfileList.h"

//==============================================================================
class ProfileList::Row : public juce::Component,
                         private juce::TextEditor::Listener
{
public:
    Row (ProfileList& owner, const juce::String& name, bool isSelected, int numBands)
        : owner (owner), name (name), isSelected (isSelected), numBands (numBands)
    {
        setRepaintsOnMouseActivity (true);
    }

    const juce::String& getName() const { return name; }

    void update (bool nowSelected, int nowNumBands)
    {
        if (nowSelected != isSelected || nowNumBands != numBands)
        {
            isSelected = nowSelected;
            numBands = nowNumBands;
            repaint();
        }
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced (8.0f, 2.0f);
        const bool hot = isMouseOver (true);

        if (isSelected || hot)
        {
            g.setColour (isSelected ? Theme::raised : Theme::panel);
            g.fillRoundedRectangle (bounds, Theme::cornerRadius);
        }
        if (isSelected)
        {
            g.setColour (Theme::accent);
            g.fillRoundedRectangle (bounds.withWidth (3.0f).reduced (0.0f, 8.0f), 1.5f);
        }

        if (editor != nullptr)
            return;

        auto text = bounds.reduced (14.0f, 0.0f);
        auto more = text.removeFromRight (hot ? 24.0f : 0.0f);

        g.setColour (isSelected ? Theme::text : Theme::textDim);
        g.setFont (Theme::font (13.5f, isSelected));
        g.drawText (name, text, juce::Justification::centredLeft, true);

        if (hot)
            Icons::draw (g, Icons::ellipsis(), more.withSizeKeepingCentre (18.0f, 18.0f), Theme::textDim, 1.4f);
        else if (numBands > 0)
        {
            g.setColour (Theme::textFaint);
            g.setFont (Theme::font (11.0f));
            g.drawText (juce::String (numBands), text, juce::Justification::centredRight);
        }
    }

    void mouseDown (const juce::MouseEvent& event) override
    {
        if (editor != nullptr)
            return;

        const bool onMoreButton = event.x > getWidth() - 40;
        if (event.mods.isPopupMenu() || onMoreButton)
        {
            owner.showRowMenu (name, *this);
            return;
        }
        owner.processor.selectProfile (name);
    }

    void mouseDoubleClick (const juce::MouseEvent& event) override
    {
        if (! event.mods.isPopupMenu() && event.x <= getWidth() - 40)
            startRenaming();
    }

    void startRenaming()
    {
        editor = std::make_unique<juce::TextEditor>();
        editor->setFont (Theme::font (13.5f));
        editor->setText (name, false);
        editor->setIndents (8, 0);
        editor->setJustification (juce::Justification::centredLeft);
        editor->addListener (this);
        addAndMakeVisible (*editor);
        editor->setBounds (getLocalBounds().reduced (12, 5));
        editor->grabKeyboardFocus();
        editor->selectAll();
        repaint();
    }

private:
    void finishRenaming (bool apply)
    {
        if (editor == nullptr)
            return;

        const auto newName = editor->getText();
        editor->removeListener (this);
        editor->setVisible (false);

        // The editor's the one calling, so delete it later
        juce::MessageManager::callAsync ([old = editor.release()] { delete old; });

        if (apply && newName.trim().isNotEmpty() && newName.trim() != name)
        {
            owner.processor.getUndoManager().beginNewTransaction ("Rename profile");
            owner.processor.getProfiles().renameProfile (name, newName);
        }
        repaint();
    }

    void textEditorReturnKeyPressed (juce::TextEditor&) override { finishRenaming (true); }
    void textEditorEscapeKeyPressed (juce::TextEditor&) override { finishRenaming (false); }
    void textEditorFocusLost (juce::TextEditor&) override { finishRenaming (true); }

    ProfileList& owner;
    juce::String name;
    bool isSelected;
    int numBands;
    std::unique_ptr<juce::TextEditor> editor;
};

//==============================================================================
class ProfileList::AddButton : public juce::Button
{
public:
    AddButton() : juce::Button ("New profile")
    {
        setTooltip ("New profile, or import one");
    }

    void paintButton (juce::Graphics& g, bool isHighlighted, bool isDown) override
    {
        auto bounds = getLocalBounds().toFloat();
        if (isHighlighted || isDown)
        {
            g.setColour (isDown ? Theme::raisedHover : Theme::raised);
            g.fillRoundedRectangle (bounds, Theme::cornerRadius);
        }
        Icons::draw (g, Icons::plus(), bounds.withSizeKeepingCentre (18.0f, 18.0f), isHighlighted ? Theme::text : Theme::textDim, 1.6f);
    }
};

//==============================================================================
ProfileList::ProfileList (CabinEqAudioProcessor& p)
    : processor (p)
{
    addButton = std::make_unique<AddButton>();
    addButton->onClick = [this] { showAddMenu(); };
    addAndMakeVisible (*addButton);

    viewport.setViewedComponent (&rowHolder, false);
    viewport.setScrollBarsShown (true, false);
    viewport.setScrollBarThickness (8);
    addAndMakeVisible (viewport);

    refresh();
}

ProfileList::~ProfileList() = default;

void ProfileList::refresh()
{
    auto& profiles = processor.getProfiles();
    const auto selected = profiles.getSelectedProfileName();
    const auto names = profiles.getProfileNames();
    auto numBandsIn = [&profiles] (const juce::String& name)
    {
        auto profile = profiles.getProfileNamed (name);
        return profile.has_value() ? profile->getNumBands() : 0;
    };

    // Keep the rows if the names haven't changed, so a double-click can land on the same row
    bool sameNames = (int) names.size() == rows.size();
    for (int i = 0; sameNames && i < rows.size(); ++i)
        sameNames = rows[i]->getName() == names[(size_t) i];

    if (sameNames)
    {
        for (auto* row : rows)
            row->update (row->getName() == selected, numBandsIn (row->getName()));
    }
    else
    {
        rows.clear();
        for (const auto& name : names)
            rowHolder.addAndMakeVisible (rows.add (new Row (*this, name, name == selected, numBandsIn (name))));
        resized();
    }

    if (pendingRename.isNotEmpty())
    {
        auto name = pendingRename;
        pendingRename = {};
        startRenaming (name);
    }
}

void ProfileList::paint (juce::Graphics& g)
{
    g.fillAll (Theme::sidebar);

    auto header = getLocalBounds().removeFromTop (headerHeight).reduced (22, 0);
    g.setColour (Theme::textFaint);
    g.setFont (Theme::font (11.0f, true));
    g.drawText ("PROFILES", header, juce::Justification::centredLeft);

    g.setColour (Theme::border);
    g.drawVerticalLine (getWidth() - 1, 0.0f, (float) getHeight());
}

void ProfileList::resized()
{
    auto area = getLocalBounds().withTrimmedRight (1);
    auto header = area.removeFromTop (headerHeight);
    addButton->setBounds (header.removeFromRight (44).withSizeKeepingCentre (28, 28));

    viewport.setBounds (area);
    rowHolder.setSize (viewport.getMaximumVisibleWidth(), rows.size() * rowHeight + 8);
    for (int i = 0; i < rows.size(); ++i)
        rows[i]->setBounds (0, i * rowHeight, rowHolder.getWidth(), rowHeight);
}

//==============================================================================
void ProfileList::startRenaming (const juce::String& profileName)
{
    for (auto* row : rows)
    {
        if (row->getName() == profileName)
        {
            viewport.setViewPosition (0, std::max (0, row->getY() - viewport.getHeight() / 2));
            row->startRenaming();
            return;
        }
    }
}

void ProfileList::addProfile()
{
    processor.getUndoManager().beginNewTransaction ("New profile");
    auto profile = processor.getProfiles().addProfile ("New Profile");
    processor.selectProfile (profile.getName());
    pendingRename = profile.getName(); // rename it once its row exists
}

void ProfileList::showAddMenu()
{
    juce::Component::SafePointer<ProfileList> safeThis (this);
    juce::PopupMenu menu;
    menu.addItem ("New profile", [safeThis] { if (safeThis != nullptr) safeThis->addProfile(); });
    menu.addSeparator();
    menu.addItem ("Import from file...", [safeThis] { if (safeThis != nullptr && safeThis->onImportFile) safeThis->onImportFile(); });
    menu.addItem ("Paste from clipboard", [safeThis] { if (safeThis != nullptr && safeThis->onPaste) safeThis->onPaste(); });
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (addButton.get()));
}

void ProfileList::showRowMenu (const juce::String& profileName, juce::Component& target)
{
    juce::Component::SafePointer<ProfileList> safeThis (this);
    auto& profiles = processor.getProfiles();
    const bool canDelete = profiles.getProfileNames().size() > 1;

    juce::PopupMenu menu;
    menu.addItem ("Rename", [safeThis, profileName] { if (safeThis != nullptr) safeThis->startRenaming (profileName); });
    menu.addItem ("Duplicate", [safeThis, profileName]
    {
        if (safeThis == nullptr)
            return;
        safeThis->processor.getUndoManager().beginNewTransaction ("Duplicate profile");
        auto copy = safeThis->processor.getProfiles().duplicateProfile (profileName);
        safeThis->processor.selectProfile (copy.getName());
    });
    menu.addSeparator();
    menu.addItem ("Export to file...", [safeThis, profileName] { if (safeThis != nullptr && safeThis->onExportFile) safeThis->onExportFile (profileName); });
    menu.addItem ("Copy as text", [safeThis, profileName] { if (safeThis != nullptr && safeThis->onCopy) safeThis->onCopy (profileName); });
    menu.addSeparator();
    menu.addItem (juce::PopupMenu::Item ("Delete").setEnabled (canDelete).setColour (Theme::danger).setAction ([safeThis, profileName]
    {
        if (safeThis != nullptr)
            safeThis->confirmDelete (profileName);
    }));

    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&target)
                                                  .withTargetScreenArea ({ juce::Desktop::getMousePosition(), juce::Desktop::getMousePosition() }));
}

void ProfileList::confirmDelete (const juce::String& profileName)
{
    juce::Component::SafePointer<ProfileList> safeThis (this);
    auto options = juce::MessageBoxOptions()
                       .withIconType (juce::MessageBoxIconType::NoIcon)
                       .withTitle ("Delete \"" + profileName + "\"?")
                       .withMessage ("You can get it back with Undo.")
                       .withButton ("Delete")
                       .withButton ("Cancel")
                       .withAssociatedComponent (this);

    juce::AlertWindow::showAsync (options, [safeThis, profileName] (int result)
    {
        if (safeThis == nullptr || result != 1)
            return;

        auto& profiles = safeThis->processor.getProfiles();
        auto names = profiles.getProfileNames();
        auto position = std::find (names.begin(), names.end(), profileName);
        if (position == names.end() || names.size() <= 1)
            return;

        // Select a neighbour before the profile goes
        if (profiles.getSelectedProfileName() == profileName)
            safeThis->processor.selectProfile (position + 1 != names.end() ? *(position + 1) : *(position - 1));

        safeThis->processor.getUndoManager().beginNewTransaction ("Delete profile");
        profiles.removeProfile (profileName);
    });
}
