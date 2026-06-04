#pragma once

#include <vector>
#include <utility>
#include <initializer_list>
#include <juce_core/juce_core.h>

/*  Factory presets.

    Each preset is a name + a list of (parameterID, rawValue) pairs, plus the
    category it belongs to (used to group the editor's preset menu into
    submenus).  Applying a preset writes its raw values into the APVTS; anything
    not listed keeps its default.

    model index: 0=Tape 1=EMI 2=Neve 3=Triode 4=Pentode.
    (Style "C - Capture" is user-loaded, so no factory preset selects it.)
*/

namespace decap
{

struct Preset
{
    const char* name;
    std::vector<std::pair<const char*, float>> values;
    const char* category = nullptr;   // filled in by the builder below
};

inline const std::vector<Preset>& factoryPresets()
{
    static const std::vector<Preset> presets = []
    {
        std::vector<Preset> all;
        auto add = [&all] (const char* cat, std::initializer_list<Preset> items)
        {
            for (auto it : items) { it.category = cat; all.push_back (std::move (it)); }
        };

        add ("General",
        {
            { "Init",            { { "drive", 3.0f }, { "model", 0.0f }, { "tone", 0.0f },
                                   { "mix", 100.0f }, { "output", 0.0f } } },   // neutral starting point
        });

        add ("Drums & Percussion",
        {
            { "Concrete Kick",   { { "drive", 4.2f }, { "model", 2.0f }, { "thump", 1.0f },
                                   { "lowcut", 32.0f }, { "tone", -0.1f }, { "highcut", 12000.0f } } },   // tight Neve low-end thump
            { "Snare Switchblade",{ { "drive", 5.5f }, { "model", 1.0f }, { "tone", 0.3f },
                                   { "lowcut", 180.0f }, { "highcut", 14000.0f }, { "output", -2.5f } } },   // EMI crack and bite
            { "Bus Mortar",      { { "drive", 3.0f }, { "model", 0.0f }, { "tone", 0.0f },
                                   { "lowcut", 28.0f }, { "thump", 1.0f }, { "mix", 100.0f } } },   // tape glue for the drum bus
            { "Parallel Carnage",{ { "drive", 8.5f }, { "model", 4.0f }, { "punish", 1.0f },
                                   { "tone", 0.1f }, { "lowcut", 40.0f }, { "mix", 35.0f }, { "output", -5.0f } } },   // parallel pentode smash
            { "Pulverizer",      { { "drive", 9.5f }, { "model", 4.0f }, { "punish", 1.0f },
                                   { "tone", 0.2f }, { "lowcut", 60.0f }, { "steep", 1.0f }, { "mix", 28.0f }, { "output", -6.0f } } },   // extreme parallel crush
            { "Loft Overheads",  { { "drive", 3.4f }, { "model", 1.0f }, { "tone", 0.25f },
                                   { "lowcut", 120.0f }, { "highcut", 16000.0f }, { "mix", 70.0f } } },   // airy EMI grit on overheads
            { "Sub Detonator",   { { "drive", 6.0f }, { "model", 3.0f }, { "thump", 1.0f },
                                   { "tone", 0.15f }, { "lowcut", 28.0f }, { "output", -3.0f } } },   // triode harmonics to make 808s cut
            { "Hat Softener",    { { "drive", 2.2f }, { "model", 0.0f }, { "tone", -0.2f },
                                   { "lowcut", 300.0f }, { "highcut", 13000.0f }, { "mix", 80.0f } } },   // tame brittle hi-hats
            { "Cassette Crush",  { { "drive", 5.0f }, { "model", 0.0f }, { "tone", -0.45f },
                                   { "lowcut", 90.0f }, { "highcut", 8500.0f }, { "mix", 85.0f } } },   // lo-fi tape-degraded loop
            { "Trap Smackdown",  { { "drive", 6.5f }, { "model", 4.0f }, { "punish", 1.0f },
                                   { "tone", 0.35f }, { "lowcut", 50.0f }, { "highcut", 15000.0f }, { "output", -4.0f } } },   // grit on trap loops and claps
            { "Garage Slam",     { { "drive", 5.8f }, { "model", 2.0f }, { "thump", 1.0f },
                                   { "tone", 0.05f }, { "lowcut", 35.0f }, { "mix", 100.0f }, { "output", -3.0f } } },   // thick rock drum bus attitude
            { "Vintage Stomp",   { { "drive", 4.6f }, { "model", 3.0f }, { "tone", -0.05f },
                                   { "lowcut", 45.0f }, { "highcut", 11000.0f }, { "mix", 60.0f } } },   // smooth tube warmth for retro kits
        });

        add ("Bass & Low-End",
        {
            { "DI Phantom",      { { "drive", 2.5f }, { "model", 2.0f }, { "tone", -0.1f },
                                   { "lowcut", 25.0f }, { "thump", 1.0f }, { "mix", 100.0f } } },   // warm clean DI bass
            { "Finger Velvet",   { { "drive", 3.5f }, { "model", 3.0f }, { "tone", 0.0f },
                                   { "lowcut", 30.0f }, { "highcut", 12000.0f }, { "mix", 100.0f } } },   // smooth tube finger warmth
            { "Pick & Shovel",   { { "drive", 4.5f }, { "model", 1.0f }, { "tone", 0.35f },
                                   { "lowcut", 40.0f }, { "highcut", 14000.0f }, { "mix", 100.0f } } },   // EMI bite for pick attack
            { "Motown Magic",    { { "drive", 4.0f }, { "model", 2.0f }, { "tone", -0.15f },
                                   { "lowcut", 30.0f }, { "highcut", 9000.0f }, { "thump", 1.0f }, { "mix", 100.0f } } },   // round vintage Jamerson thump
            { "Dub Chamber",     { { "drive", 3.0f }, { "model", 0.0f }, { "tone", -0.5f },
                                   { "lowcut", 22.0f }, { "highcut", 6000.0f }, { "thump", 1.0f }, { "mix", 100.0f } } },   // dark reggae/dub fundamental
            { "808 Inferno",     { { "drive", 7.0f }, { "model", 4.0f }, { "tone", 0.2f },
                                   { "lowcut", 22.0f }, { "highcut", 11000.0f }, { "thump", 1.0f }, { "punish", 1.0f }, { "mix", 100.0f }, { "output", -4.0f } } },   // aggressive 808 grit
            { "Sub Translator",  { { "drive", 4.0f }, { "model", 3.0f }, { "tone", 0.1f },
                                   { "lowcut", 20.0f }, { "thump", 1.0f }, { "mix", 45.0f } } },   // parallel sub harmonics for small speakers
            { "Growl Engine",    { { "drive", 6.5f }, { "model", 4.0f }, { "tone", 0.25f },
                                   { "lowcut", 45.0f }, { "highcut", 8000.0f }, { "punish", 1.0f }, { "mix", 100.0f }, { "output", -3.0f } } },   // grindy bass growl
            { "Parallel Forge",  { { "drive", 6.0f }, { "model", 2.0f }, { "tone", 0.0f },
                                   { "lowcut", 20.0f }, { "thump", 1.0f }, { "mix", 40.0f }, { "output", -2.0f } } },   // parallel drive, clean sub intact
            { "Synth Crusher",   { { "drive", 8.0f }, { "model", 4.0f }, { "tone", 0.15f },
                                   { "lowcut", 35.0f }, { "highcut", 9000.0f }, { "punish", 1.0f }, { "steep", 1.0f }, { "mix", 100.0f }, { "output", -5.0f } } },   // crushed buzzy synth bass
            { "Fuzz Leviathan",  { { "drive", 9.5f }, { "model", 4.0f }, { "tone", 0.1f },
                                   { "lowcut", 50.0f }, { "highcut", 7000.0f }, { "punish", 1.0f }, { "steep", 1.0f }, { "mix", 100.0f }, { "output", -7.0f } } },   // extreme fuzz bass wall
            { "Creamy Pocket",   { { "drive", 3.0f }, { "model", 2.0f }, { "tone", -0.05f },
                                   { "lowcut", 28.0f }, { "highcut", 13000.0f }, { "thump", 1.0f }, { "mix", 100.0f } } },   // creamy Neve mix-pocket glue
        });

        add ("Guitars, Keys & Synths",
        {
            { "Crunch Rhythm",   { { "drive", 6.0f }, { "model", 4.0f }, { "tone", 0.1f }, { "lowcut", 110.0f },
                                   { "highcut", 7000.0f }, { "punish", 1.0f }, { "output", -3.0f } } },   // tight amp-style crunch
            { "Lead Igniter",    { { "drive", 7.5f }, { "model", 3.0f }, { "tone", 0.25f }, { "lowcut", 120.0f },
                                   { "highcut", 9000.0f }, { "output", -4.0f } } },   // singing lead with sustain
            { "Velvet Fuzz",     { { "drive", 9.5f }, { "model", 4.0f }, { "tone", -0.1f }, { "lowcut", 90.0f },
                                   { "highcut", 5500.0f }, { "punish", 1.0f }, { "output", -6.0f } } },   // thick saturated fuzz lead
            { "Amp In A Box",    { { "drive", 6.5f }, { "model", 4.0f }, { "tone", 0.0f }, { "lowcut", 100.0f },
                                   { "highcut", 6000.0f }, { "punish", 1.0f }, { "output", -3.5f } } },   // DI reamp full amp grit
            { "Acoustic Shimmer",{ { "drive", 2.0f }, { "model", 1.0f }, { "tone", 0.35f }, { "lowcut", 80.0f },
                                   { "highcut", 18000.0f }, { "mix", 45.0f } } },   // airy parallel acoustic sparkle
            { "Surf Spaghetti",  { { "drive", 3.5f }, { "model", 0.0f }, { "tone", 0.15f }, { "lowcut", 90.0f },
                                   { "highcut", 9000.0f }, { "output", -1.0f } } },   // twangy vintage surf tone
            { "Lo-Fi Sting",     { { "drive", 5.0f }, { "model", 0.0f }, { "tone", -0.4f }, { "lowcut", 160.0f },
                                   { "highcut", 4000.0f }, { "thump", 1.0f }, { "output", -2.0f } } },   // gritty dark lo-fi guitar
            { "Rhodes Warmth",   { { "drive", 3.0f }, { "model", 3.0f }, { "tone", 0.1f }, { "lowcut", 60.0f },
                                   { "highcut", 14000.0f }, { "mix", 60.0f } } },   // creamy tube Rhodes warmth
            { "Vintage Keys",    { { "drive", 2.5f }, { "model", 0.0f }, { "tone", -0.05f }, { "lowcut", 50.0f },
                                   { "highcut", 12000.0f }, { "mix", 55.0f } } },   // soft tape-warmed electric piano
            { "Organ Grit",      { { "drive", 7.0f }, { "model", 2.0f }, { "tone", 0.05f }, { "lowcut", 70.0f },
                                   { "highcut", 8000.0f }, { "thump", 1.0f }, { "output", -4.0f } } },   // driven Leslie-style organ growl
            { "Synth Lead Edge", { { "drive", 6.0f }, { "model", 1.0f }, { "tone", 0.3f }, { "lowcut", 100.0f },
                                   { "highcut", 12000.0f }, { "output", -3.0f } } },   // bright cutting synth lead
            { "Pad Thickener",   { { "drive", 3.5f }, { "model", 2.0f }, { "tone", -0.1f }, { "lowcut", 40.0f },
                                   { "highcut", 16000.0f }, { "mix", 40.0f } } },   // creamy parallel pad thickening
        });

        add ("Vocals",
        {
            { "Silky Lead",      { { "drive", 2.5f }, { "model", 3.0f }, { "tone", 0.15f }, { "lowcut", 100.0f },
                                   { "highcut", 18000.0f }, { "mix", 70.0f } } },   // smooth tube vocal warmth
            { "Vintage Tube Croon",{ { "drive", 3.2f }, { "model", 3.0f }, { "tone", -0.05f }, { "lowcut", 90.0f },
                                   { "thump", 1.0f }, { "mix", 75.0f }, { "output", -1.0f } } },   // fat classic valve vocal
            { "Neve Velvet",     { { "drive", 2.8f }, { "model", 2.0f }, { "tone", 0.0f }, { "lowcut", 95.0f },
                                   { "mix", 65.0f } } },   // thick creamy console body
            { "Pop Air Sheen",   { { "drive", 1.8f }, { "model", 1.0f }, { "tone", 0.55f }, { "lowcut", 120.0f },
                                   { "highcut", 20000.0f }, { "mix", 60.0f } } },   // bright EMI top-end shimmer
            { "Breathy Intimacy",{ { "drive", 1.4f }, { "model", 0.0f }, { "tone", 0.25f }, { "lowcut", 110.0f },
                                   { "mix", 45.0f } } },   // soft close-mic whisper glue
            { "Backing Vocal Glue",{ { "drive", 3.0f }, { "model", 0.0f }, { "tone", 0.1f }, { "lowcut", 130.0f },
                                   { "highcut", 16000.0f }, { "mix", 50.0f } } },   // cohesive stacked harmony bed
            { "Rap Vocal Grit",  { { "drive", 5.5f }, { "model", 2.0f }, { "tone", 0.2f }, { "lowcut", 100.0f },
                                   { "punish", 1.0f }, { "mix", 55.0f }, { "output", -2.0f } } },   // upfront hip-hop edge
            { "Aggressive Scream",{ { "drive", 8.5f }, { "model", 4.0f }, { "tone", 0.3f }, { "lowcut", 120.0f },
                                   { "punish", 1.0f }, { "steep", 1.0f }, { "mix", 100.0f }, { "output", -4.0f } } },   // buzzy pentode rock rage
            { "Podcast Warmth",  { { "drive", 2.0f }, { "model", 3.0f }, { "tone", 0.05f }, { "lowcut", 80.0f },
                                   { "highcut", 15000.0f }, { "mix", 80.0f } } },   // smooth spoken-word body
            { "Telephone FX",    { { "drive", 6.5f }, { "model", 4.0f }, { "tone", 0.4f }, { "lowcut", 500.0f },
                                   { "highcut", 3000.0f }, { "punish", 1.0f }, { "mix", 100.0f }, { "output", -3.0f } } },   // narrowband phone caller
            { "Megaphone Blast", { { "drive", 7.5f }, { "model", 4.0f }, { "tone", 0.5f }, { "lowcut", 400.0f },
                                   { "highcut", 3500.0f }, { "punish", 1.0f }, { "steep", 1.0f }, { "mix", 100.0f }, { "output", -4.0f } } },   // distorted bullhorn shout
            { "Lo-Fi Crushed",   { { "drive", 6.0f }, { "model", 1.0f }, { "tone", -0.3f }, { "lowcut", 300.0f },
                                   { "highcut", 4000.0f }, { "mix", 100.0f }, { "output", -2.0f } } },   // gritty vintage radio voice
        });

        add ("Mix Bus / Master Glue",
        {
            { "Bus Glue",        { { "drive", 1.8f }, { "model", 0.0f }, { "tone", 0.0f },
                                   { "mix", 100.0f }, { "output", 0.0f } } },   // gentle 2-bus tape warmth
            { "Tape Master",     { { "drive", 2.2f }, { "model", 0.0f }, { "tone", -0.1f },
                                   { "highcut", 19000.0f }, { "thump", 1.0f }, { "mix", 100.0f }, { "output", -0.5f } } },   // warm tape master with weight
            { "Console Glue",    { { "drive", 2.4f }, { "model", 2.0f }, { "tone", 0.05f },
                                   { "highcut", 18000.0f }, { "mix", 100.0f }, { "output", -0.5f } } },   // thick Neve console glue
            { "Analog Sheen",    { { "drive", 1.6f }, { "model", 1.0f }, { "tone", 0.25f },
                                   { "highcut", 20000.0f }, { "mix", 90.0f }, { "output", 0.0f } } },   // bright EMI air and sparkle
            { "Mastering Harmonics",{ { "drive", 1.2f }, { "model", 3.0f }, { "tone", 0.1f },
                                   { "highcut", 20000.0f }, { "mix", 70.0f }, { "output", 0.0f } } },   // subtle triode mastering harmonics
            { "Sub-Mix Glue",    { { "drive", 2.6f }, { "model", 2.0f }, { "tone", 0.0f },
                                   { "lowcut", 30.0f }, { "mix", 80.0f }, { "output", -1.0f } } },   // parallel glue for stem sub-mixes
        });

        add ("Lo-Fi & Special FX",
        {
            { "Vinyl Warmth",    { { "drive", 4.0f }, { "model", 0.0f }, { "tone", -0.3f },
                                   { "lowcut", 40.0f }, { "highcut", 12000.0f }, { "mix", 85.0f }, { "output", -1.5f } } },   // warm dusty vinyl vibe
            { "Dirty Cassette",  { { "drive", 5.5f }, { "model", 0.0f }, { "tone", -0.4f },
                                   { "lowcut", 120.0f }, { "highcut", 7000.0f }, { "mix", 100.0f }, { "output", -2.0f } } },   // gnarly worn-out cassette
            { "Vintage Radio",   { { "drive", 6.0f }, { "model", 1.0f }, { "tone", -0.2f },
                                   { "lowcut", 350.0f }, { "highcut", 4500.0f }, { "mix", 100.0f }, { "output", -2.0f } } },   // mid-focused transistor radio
            { "AM Broadcast",    { { "drive", 6.5f }, { "model", 3.0f }, { "tone", -0.3f },
                                   { "lowcut", 450.0f }, { "highcut", 3200.0f }, { "steep", 1.0f }, { "mix", 100.0f }, { "output", -2.5f } } },   // narrow-band AM squawk
            { "Broken Speaker",  { { "drive", 7.5f }, { "model", 4.0f }, { "tone", -0.5f },
                                   { "lowcut", 300.0f }, { "highcut", 3000.0f }, { "mix", 100.0f }, { "output", -3.5f } } },   // crackly blown cone speaker
            { "Tape Dropout",    { { "drive", 5.0f }, { "model", 0.0f }, { "tone", -0.55f },
                                   { "lowcut", 90.0f }, { "highcut", 5500.0f }, { "mix", 95.0f }, { "output", -2.0f } } },   // wobbly degraded tape texture
            { "Blown Amp",       { { "drive", 8.5f }, { "model", 4.0f }, { "tone", -0.15f },
                                   { "lowcut", 150.0f }, { "highcut", 6000.0f }, { "punish", 1.0f }, { "mix", 100.0f }, { "output", -5.0f } } },   // overdriven blown amp grind
            { "Obliterate",      { { "drive", 10.0f }, { "model", 4.0f }, { "tone", 0.2f },
                                   { "lowcut", 200.0f }, { "highcut", 5000.0f }, { "punish", 1.0f }, { "steep", 1.0f }, { "mix", 100.0f }, { "output", -7.0f } } },   // full pentode destroy
        });

        return all;
    }();
    return presets;
}

} // namespace decap
