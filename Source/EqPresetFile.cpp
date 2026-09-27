/*
  ==============================================================================

    EqPresetFile.cpp

  ==============================================================================
*/

#include "EqPresetFile.h"
#include "FilterChain.h"

namespace
{
    std::optional<float> numberAfter (const juce::StringArray& tokens, const juce::String& keyword, int skip = 0)
    {
        const int index = tokens.indexOf (keyword, true);
        if (index < 0 || index + 1 + skip >= tokens.size())
            return std::nullopt;

        auto token = tokens[index + 1 + skip];
        if (! token.containsAnyOf ("0123456789"))
            return std::nullopt;
        return token.getFloatValue();
    }

    std::optional<Band::Shape> shapeForCode (const juce::String& code)
    {
        const auto upper = code.toUpperCase();
        if (upper == "PK" || upper == "PEQ" || upper == "MODAL")
            return Band::Shape::peak;
        if (upper == "LS" || upper == "LSC")
            return Band::Shape::lowShelf;
        if (upper == "HS" || upper == "HSC")
            return Band::Shape::highShelf;
        if (upper == "HP" || upper == "HPQ")
            return Band::Shape::lowCut;
        if (upper == "LP" || upper == "LPQ")
            return Band::Shape::highCut;
        return std::nullopt; // notch, all-pass and band-pass aren't supported
    }

    juce::String codeForShape (Band::Shape shape)
    {
        switch (shape)
        {
            case Band::Shape::peak:      return "PK";
            case Band::Shape::lowShelf:  return "LSC";
            case Band::Shape::highShelf: return "HSC";
            case Band::Shape::lowCut:    return "HPQ";
            case Band::Shape::highCut:   return "LPQ";
        }
        return "PK";
    }

    juce::String formatNumber (float value, int decimals)
    {
        return juce::String (value, decimals).trimCharactersAtEnd ("0").trimCharactersAtEnd (".");
    }
}

std::optional<BandProfile> EqPresetFile::parse (const juce::String& text)
{
    std::vector<Band> bands;
    float preamp = 0.0f;
    auto channel = Band::Type::both;

    for (auto line : juce::StringArray::fromLines (text))
    {
        line = line.upToFirstOccurrenceOf ("#", false, false).trim();
        if (line.isEmpty())
            continue;

        const auto key = line.upToFirstOccurrenceOf (":", false, false).trim().toLowerCase();
        const auto value = line.fromFirstOccurrenceOf (":", false, false).trim();

        if (key == "preamp")
        {
            preamp = value.getFloatValue();
        }
        else if (key == "channel")
        {
            auto channels = juce::StringArray::fromTokens (value.toUpperCase(), " ,", "");
            channel = Band::Type::both;
            if (channels.size() == 1 && channels[0] == "L")
                channel = Band::Type::left;
            else if (channels.size() == 1 && channels[0] == "R")
                channel = Band::Type::right;
        }
        else if (key.startsWith ("filter") && (int) bands.size() < FilterChain::maxBands)
        {
            auto tokens = juce::StringArray::fromTokens (value, " \t", "");
            tokens.removeEmptyStrings();
            if (tokens.size() < 3)
                continue;

            const bool isOn = tokens[0].equalsIgnoreCase ("ON");
            auto shape = shapeForCode (tokens[1]);
            auto freq = numberAfter (tokens, "Fc");
            if (! shape.has_value() || ! freq.has_value())
                continue;

            Band band;
            band.id = (int) bands.size();
            band.shape = *shape;
            band.type = channel;
            band.enabled = isOn;
            band.freq = juce::jlimit (Band::minFreq, Band::maxFreq, *freq);
            band.ampl = juce::jlimit (Band::minGain, Band::maxGain, numberAfter (tokens, "Gain").value_or (0.0f));

            const bool hasGain = band.hasGain();
            const float defaultQ = hasGain && band.shape == Band::Shape::peak ? Band::defaultQ : Band::defaultCutQ;
            if (auto q = numberAfter (tokens, "Q"))
                band.setQ (*q);
            else if (auto bandwidth = numberAfter (tokens, "BW", 1)) // "BW Oct 1.5"
                band.setBandwidth (*bandwidth);
            else
                band.setQ (defaultQ);

            bands.push_back (band);
        }
    }

    if (bands.empty())
        return std::nullopt;
    return BandProfile (std::move (bands), juce::jlimit (-30.0f, 30.0f, preamp));
}

juce::String EqPresetFile::write (const BandProfile& bandProfile)
{
    juce::String text;
    text << "Preamp: " << formatNumber (bandProfile.getVolume(), 2) << " dB\n";

    bool needsChannels = false;
    for (const auto& band : bandProfile.getBands())
        needsChannels |= band.type != Band::Type::both;

    int filterNumber = 1;
    for (auto type : { Band::Type::both, Band::Type::left, Band::Type::right })
    {
        bool wroteChannel = false;
        for (const auto& band : bandProfile.getBands())
        {
            if (band.type != type)
                continue;

            if (needsChannels && ! wroteChannel)
            {
                text << "Channel: " << (type == Band::Type::left ? "L" : type == Band::Type::right ? "R" : "all") << "\n";
                wroteChannel = true;
            }

            text << "Filter " << filterNumber++ << ": " << (band.enabled ? "ON " : "OFF ") << codeForShape (band.shape)
                 << " Fc " << formatNumber (band.freq, 1) << " Hz";
            if (band.hasGain())
                text << " Gain " << formatNumber (band.ampl, 2) << " dB";
            text << " Q " << formatNumber (band.qFactor, 3) << "\n";
        }
    }
    return text;
}
