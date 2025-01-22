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
        return "cabinaudio.com";
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
