#pragma once

#include <array>
#include <vector>
#include <utility>
#include <juce_core/juce_core.h>

/*  Factory presets.

    Each preset is just a name plus a list of (parameterID, rawValue) pairs.
    Applying one writes those raw values into the APVTS, notifying the host so
    automation/undo stay consistent.  Anything not listed keeps its default.   */

namespace decap
{

struct Preset
{
    const char* name;
    std::vector<std::pair<const char*, float>> values;
};

inline const std::vector<Preset>& factoryPresets()
{
    static const std::vector<Preset> presets =
    {
        { "Init",          { { "drive", 3.0f },  { "model", 0.0f }, { "tone", 0.0f },
                             { "mix", 100.0f },   { "output", 0.0f } } },

        { "Warm Glue",     { { "drive", 2.5f },  { "model", 0.0f }, { "tone", -0.1f },
                             { "thump", 1.0f },   { "mix", 100.0f } } },

        { "Console Thick", { { "drive", 4.0f },  { "model", 2.0f }, { "tone", 0.1f },
                             { "thump", 1.0f },   { "lowcut", 30.0f } } },

        { "Bright Bite",   { { "drive", 5.0f },  { "model", 1.0f }, { "tone", 0.3f },
                             { "highcut", 18000.0f } } },

        { "Tube Vocal",    { { "drive", 3.5f },  { "model", 3.0f }, { "tone", 0.1f },
                             { "lowcut", 90.0f }, { "mix", 80.0f } } },

        { "Parallel Crush",{ { "drive", 8.0f },  { "model", 4.0f }, { "punish", 1.0f },
                             { "mix", 35.0f },    { "steep", 1.0f } } },

        { "Destroy",       { { "drive", 9.0f },  { "model", 4.0f }, { "punish", 1.0f },
                             { "steep", 1.0f },   { "lowcut", 130.0f }, { "tone", 0.2f },
                             { "mix", 100.0f } } },
    };
    return presets;
}

} // namespace decap
