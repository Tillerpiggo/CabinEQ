/*
  ==============================================================================

    CabinEqMarketplaceStatus.h
    Created: 20 Jan 2025 7:23:12pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

class CabinEqMarketplaceStatus  : public juce::OnlineUnlockStatus
{
public:
    CabinEqMarketplaceStatus() = default;

    juce::String getProductID() override
    {
        return "cabineq";
    }

    bool doesProductIDMatch (const juce::String& returnedIDFromServer) override
    {
        return getProductID() == returnedIDFromServer;
    }

    juce::RSAKey getPublicKey() override
    {
        return juce::RSAKey ("5,766b50255400a29ebbc8396c4d9a4cdebc2b09f529dbd8d102b0fb255d551101");
    }

    juce::String getLicenseFilePath()
    {
        /*
            Does this need to be freed by the caller?
        */
        return juce::String(juce::File::getSpecialLocation(juce::File::SpecialLocationType::userApplicationDataDirectory).getFullPathName()) + "/CabinEQ.license";
    }

    void saveState (const juce::String& licenseString) override {
        juce::File licenseFile(getLicenseFilePath());
        if (! licenseFile.existsAsFile())
            licenseFile.create();
        licenseFile.replaceWithText(licenseString);
    }

    juce::String getState() override {
        return juce::File(getLicenseFilePath()).loadFileAsString();
    }

    juce::String getWebsiteName() override
    {
        return "cabinaudio.com";
    }

    juce::URL getServerAuthenticationURL() override
    {
        return juce::URL ("https://api.cabinaudio.com/login_user");
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
