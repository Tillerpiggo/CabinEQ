/*
  ==============================================================================

    CabinEQPage.h
    Created: 27 Jul 2024 9:31:27pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "EQNode.h"
#include "CabinEQGraph.h"

class TutorialMarketplaceStatus   : public juce::OnlineUnlockStatus
{
public:
    TutorialMarketplaceStatus() = default;

    juce::String getProductID() override
    {
        return "TestApp";
    }

    bool doesProductIDMatch (const juce::String& returnedIDFromServer) override
    {
        return getProductID() == returnedIDFromServer;
    }

    juce::RSAKey getPublicKey() override
    {
        return juce::RSAKey ("INSERT_PUBLIC_KEY_HERE");
    }

    void saveState (const juce::String&) override {}
    juce::String getState() override { return {}; }

    juce::String getWebsiteName() override
    {
        return "juce.com";
    }

    juce::URL getServerAuthenticationURL() override
    {
        return juce::URL ("https://localhost:8443/auth.php");
    }

    juce::String readReplyFromWebserver (const juce::String& email, const juce::String& password) override
    {
        juce::URL url (getServerAuthenticationURL()
                    .withParameter ("product", getProductID())
                    .withParameter ("email", email)
                    .withParameter ("pw", password)
                    .withParameter ("os", juce::SystemStats::getOperatingSystemName())
                    .withParameter ("mach", getLocalMachineIDs()[0]));

        DBG ("Trying to unlock via URL: " << url.toString (true));

        {
            juce::ScopedLock lock (streamCreationLock);
            stream.reset (new juce::WebInputStream (url, true));
        }

        if (stream->connect (nullptr))
        {
            auto* thread = juce::Thread::getCurrentThread();

            if (thread->threadShouldExit() || stream->isError())
                return {};

            auto contentLength = stream->getTotalLength();
            auto downloaded    = 0;

            const size_t bufferSize = 0x8000;
            juce::HeapBlock<char> buffer (bufferSize);

            while (! (stream->isExhausted() || stream->isError() || thread->threadShouldExit()))
            {
                auto max = juce::jmin ((int) bufferSize, contentLength < 0 ? std::numeric_limits<int>::max()
                                                                     : static_cast<int> (contentLength - downloaded));

                auto actualBytesRead = stream->read (buffer.get() + downloaded, max - downloaded);

                if (actualBytesRead < 0 || thread->threadShouldExit() || stream->isError())
                    break;

                downloaded += actualBytesRead;

                if (downloaded == contentLength)
                    break;
            }

            if (thread->threadShouldExit() || stream->isError() || (contentLength > 0 && downloaded < contentLength))
                return {};

            return { juce::CharPointer_UTF8 (buffer.get()) };
        }

        return {};
    }

    void userCancelled() override
    {
        juce::ScopedLock lock (streamCreationLock);

        if (stream != nullptr)
            stream->cancel();
    }

private:
    juce::CriticalSection streamCreationLock;
    std::unique_ptr<juce::WebInputStream> stream;
};

class TutorialUnlockForm    : public juce::OnlineUnlockForm
{
public:
    TutorialUnlockForm (TutorialMarketplaceStatus& status)
        : OnlineUnlockForm (status, "Please provide your email and password.")
    {}

    void dismiss() override
    {
        setVisible (false);
    }
};

class CabinEQPage   : public juce::Component,
                      public juce::Slider::Listener,
                      public juce::ComboBox::Listener,
                      public juce::TextEditor::Listener,
                      public juce::Button::Listener,
                      public CabinEQGraph::Listener,
                      public StartupMVPAudioProcessor::Listener,
                      public juce::Timer
{
public:
    CabinEQPage (StartupMVPAudioProcessor& p);
    ~CabinEQPage() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    // CabinEQGraphListener methods
    int addCurvePt (float freq, float ampl, CabinEQGraph* sender) override;
    void updateCurvePt (int id, float freq, float ampl, CabinEQGraph* sender) override;
    void removeCurvePt (int id, CabinEQGraph* sender) override;
    void startPlayingValueAt (float freq, float ampl) override;
    void playValueAt (float freq, float ampl) override;
    void testValueAt (float freq) override;
    void stopPlaying() override;
    void stopTesting() override;
    float getCurrPlayingFreq() override;
    float getCurrTestingFreq() override;
    void userStoppedDoingShit() override;
    
    void sliderValueChanged (juce::Slider *slider) override;
    void textEditorReturnKeyPressed (juce::TextEditor& textEditor) override;
    void textEditorEscapeKeyPressed (juce::TextEditor& textEditor) override;
    void textEditorFocusLost (juce::TextEditor& textEditor) override;
    void comboBoxChanged (juce::ComboBox *comboBoxThatHasChanged) override;
    void inputAttemptWhenModal() override;
    
    void buttonClicked (juce::Button *button) override;
    
    void didLoadData() override;
    
    void timerCallback() override;
    
protected:
    void flagFilterChanged();
    void toggleBypass();
    void applyFilterIfProcessing();
    void loadDropdownOptions();
    void dismissAlertWindow();
    
    void showForm();
    void unlockApp();
    
    StartupMVPAudioProcessor& processor;
    juce::String profileId;
    juce::TextButton bypassButton { "ON" };
    juce::TextButton unlockButton { "UNLOCK" };
    juce::TextButton duplicateButton { "COPY" };
    bool isBypassed = true;
    bool hasFilterChanged = true;
    bool creatingDuplicate = false;
    
    CabinEQGraph cabinEQGraph;
    juce::ComboBox dropdownProfiles;
    juce::Slider referenceSlider;
    std::unique_ptr<juce::AlertWindow> alertWindow;
    
    const juce::Colour backgroundColor = juce::Colour::fromRGB (0.4, 0.4, 0.4);
    const juce::String textEditorName = "ProfileEditor";
    
    int lastSelectedId = 1;
    
    TutorialMarketplaceStatus marketplaceStatus;
    TutorialUnlockForm unlockForm;
 
    bool isUnlocked = false;
};
