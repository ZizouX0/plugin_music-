#pragma once

#include <array>
#include <vector>
#include <utility>
#include <juce_core/juce_core.h>

/*  Factory presets.

    Each preset is just a name plus a list of (parameterID, rawValue) pairs.
    Applying one writes those raw values into the APVTS, notifying the host so
    automation/undo stay consistent.  Anything not listed keeps its default.

    Organised by category. model index: 0=Tape 1=EMI 2=Neve 3=Triode 4=Pentode.
    (Style "C - Capture" is user-loaded, so it is never used by a factory preset.)
*/

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
        // --- Default -------------------------------------------------------
        { "Init",            { { "drive", 3.0f }, { "model", 0.0f }, { "tone", 0.0f },
                               { "mix", 100.0f }, { "output", 0.0f } } },   // neutral starting point

        // --- Drums & Percussion -------------------------------------------
        { "Concrete Kick",   { { "drive", 4.2f }, { "model", 2.0f }, { "thump", 1.0f },
                               { "lowcut", 32.0f }, { "tone", -0.1f }, { "highcut", 12000.0f } } },   // tight Neve low-end thump for kick punch
        { "Snare Switchblade",{ { "drive", 5.5f }, { "model", 1.0f }, { "tone", 0.3f },
                               { "lowcut", 180.0f }, { "highcut", 14000.0f }, { "output", -2.5f } } },   // EMI crack and bite for snare attack
        { "Bus Mortar",      { { "drive", 3.0f }, { "model", 0.0f }, { "tone", 0.0f },
                               { "lowcut", 28.0f }, { "thump", 1.0f }, { "mix", 100.0f } } },   // gentle tape glue across the whole drum bus
        { "Parallel Carnage",{ { "drive", 8.5f }, { "model", 4.0f }, { "punish", 1.0f },
                               { "tone", 0.1f }, { "lowcut", 40.0f }, { "mix", 35.0f }, { "output", -5.0f } } },   // parallel pentode smash blended under dry drums
        { "Pulverizer",      { { "drive", 9.5f }, { "model", 4.0f }, { "punish", 1.0f },
                               { "tone", 0.2f }, { "lowcut", 60.0f }, { "steep", 1.0f }, { "mix", 28.0f }, { "output", -6.0f } } },   // extreme parallel crush for explosive transients
        { "Loft Overheads",  { { "drive", 3.4f }, { "model", 1.0f }, { "tone", 0.25f },
                               { "lowcut", 120.0f }, { "highcut", 16000.0f }, { "mix", 70.0f } } },   // airy EMI grit on room and overhead mics
        { "Sub Detonator",   { { "drive", 6.0f }, { "model", 3.0f }, { "thump", 1.0f },
                               { "tone", 0.15f }, { "lowcut", 28.0f }, { "output", -3.0f } } },   // triode harmonics to make 808s cut on small speakers
        { "Hat Softener",    { { "drive", 2.2f }, { "model", 0.0f }, { "tone", -0.2f },
                               { "lowcut", 300.0f }, { "highcut", 13000.0f }, { "mix", 80.0f } } },   // tame harsh, brittle hi-hats with warm tape
        { "Cassette Crush",  { { "drive", 5.0f }, { "model", 0.0f }, { "tone", -0.45f },
                               { "lowcut", 90.0f }, { "highcut", 8500.0f }, { "mix", 85.0f } } },   // lo-fi tape-degraded drum loop vibe
        { "Trap Smackdown",  { { "drive", 6.5f }, { "model", 4.0f }, { "punish", 1.0f },
                               { "tone", 0.35f }, { "lowcut", 50.0f }, { "highcut", 15000.0f }, { "output", -4.0f } } },   // aggressive grit on trap drum loops and claps
        { "Garage Slam",     { { "drive", 5.8f }, { "model", 2.0f }, { "thump", 1.0f },
                               { "tone", 0.05f }, { "lowcut", 35.0f }, { "mix", 100.0f }, { "output", -3.0f } } },   // thick Neve drive for rock drum bus attitude
        { "Vintage Stomp",   { { "drive", 4.6f }, { "model", 3.0f }, { "tone", -0.05f },
                               { "lowcut", 45.0f }, { "highcut", 11000.0f }, { "mix", 60.0f } } },   // smooth tube warmth and round-off for retro drum kits

        // --- Bass & Low-End ------------------------------------------------
        { "DI Phantom",      { { "drive", 2.5f }, { "model", 2.0f }, { "tone", -0.1f },
                               { "lowcut", 25.0f }, { "thump", 1.0f }, { "mix", 100.0f } } },   // warm clean DI bass, Neve low-mid glue
        { "Finger Velvet",   { { "drive", 3.5f }, { "model", 3.0f }, { "tone", 0.0f },
                               { "lowcut", 30.0f }, { "highcut", 12000.0f }, { "mix", 100.0f } } },   // smooth tube finger warmth
        { "Pick & Shovel",   { { "drive", 4.5f }, { "model", 1.0f }, { "tone", 0.35f },
                               { "lowcut", 40.0f }, { "highcut", 14000.0f }, { "mix", 100.0f } } },   // EMI bright bite for pick attack
        { "Motown Magic",    { { "drive", 4.0f }, { "model", 2.0f }, { "tone", -0.15f },
                               { "lowcut", 30.0f }, { "highcut", 9000.0f }, { "thump", 1.0f }, { "mix", 100.0f } } },   // round vintage Jamerson-style thump
        { "Dub Chamber",     { { "drive", 3.0f }, { "model", 0.0f }, { "tone", -0.5f },
                               { "lowcut", 22.0f }, { "highcut", 6000.0f }, { "thump", 1.0f }, { "mix", 100.0f } } },   // dark warm reggae/dub fundamental
        { "808 Inferno",     { { "drive", 7.0f }, { "model", 4.0f }, { "tone", 0.2f },
                               { "lowcut", 22.0f }, { "highcut", 11000.0f }, { "thump", 1.0f }, { "punish", 1.0f }, { "mix", 100.0f }, { "output", -4.0f } } },   // aggressive 808 grit and harmonics
        { "Sub Translator",  { { "drive", 4.0f }, { "model", 3.0f }, { "tone", 0.1f },
                               { "lowcut", 20.0f }, { "thump", 1.0f }, { "mix", 45.0f } } },   // parallel sub harmonics for small-speaker translation
        { "Growl Engine",    { { "drive", 6.5f }, { "model", 4.0f }, { "tone", 0.25f },
                               { "lowcut", 45.0f }, { "highcut", 8000.0f }, { "punish", 1.0f }, { "mix", 100.0f }, { "output", -3.0f } } },   // grindy odd-harmonic bass growl
        { "Parallel Forge",  { { "drive", 6.0f }, { "model", 2.0f }, { "tone", 0.0f },
                               { "lowcut", 20.0f }, { "thump", 1.0f }, { "mix", 40.0f }, { "output", -2.0f } } },   // parallel drive keeping clean sub intact
        { "Synth Crusher",   { { "drive", 8.0f }, { "model", 4.0f }, { "tone", 0.15f },
                               { "lowcut", 35.0f }, { "highcut", 9000.0f }, { "punish", 1.0f }, { "steep", 1.0f }, { "mix", 100.0f }, { "output", -5.0f } } },   // crushed buzzy synth bass
        { "Fuzz Leviathan",  { { "drive", 9.5f }, { "model", 4.0f }, { "tone", 0.1f },
                               { "lowcut", 50.0f }, { "highcut", 7000.0f }, { "punish", 1.0f }, { "steep", 1.0f }, { "mix", 100.0f }, { "output", -7.0f } } },   // extreme saturated fuzz bass wall
        { "Creamy Pocket",   { { "drive", 3.0f }, { "model", 2.0f }, { "tone", -0.05f },
                               { "lowcut", 28.0f }, { "highcut", 13000.0f }, { "thump", 1.0f }, { "mix", 100.0f } } },   // creamy Neve mix-pocket bass glue

        // --- Guitars, Keys & Synths ---------------------------------------
        { "Crunch Rhythm",   { { "drive", 6.0f }, { "model", 4.0f }, { "tone", 0.1f }, { "lowcut", 110.0f },
                               { "highcut", 7000.0f }, { "punish", 1.0f }, { "output", -3.0f } } },   // tight amp-style rhythm crunch
        { "Lead Igniter",    { { "drive", 7.5f }, { "model", 3.0f }, { "tone", 0.25f }, { "lowcut", 120.0f },
                               { "highcut", 9000.0f }, { "output", -4.0f } } },   // singing lead boost with sustain
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
                               { "highcut", 12000.0f }, { "output", -3.0f } } },   // bright cutting synth lead bite
        { "Pad Thickener",   { { "drive", 3.5f }, { "model", 2.0f }, { "tone", -0.1f }, { "lowcut", 40.0f },
                               { "highcut", 16000.0f }, { "mix", 40.0f } } },   // creamy parallel pad thickening

        // --- Vocals --------------------------------------------------------
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

        // --- Mix Bus / Master Glue ----------------------------------------
        { "Bus Glue",        { { "drive", 1.8f }, { "model", 0.0f }, { "tone", 0.0f },
                               { "mix", 100.0f }, { "output", 0.0f } } },   // gentle 2-bus tape warmth
        { "Tape Master",     { { "drive", 2.2f }, { "model", 0.0f }, { "tone", -0.1f },
                               { "highcut", 19000.0f }, { "thump", 1.0f }, { "mix", 100.0f }, { "output", -0.5f } } },   // warm tape master with low-end weight
        { "Console Glue",    { { "drive", 2.4f }, { "model", 2.0f }, { "tone", 0.05f },
                               { "highcut", 18000.0f }, { "mix", 100.0f }, { "output", -0.5f } } },   // thick Neve console glue on the 2-bus
        { "Analog Sheen",    { { "drive", 1.6f }, { "model", 1.0f }, { "tone", 0.25f },
                               { "highcut", 20000.0f }, { "mix", 90.0f }, { "output", 0.0f } } },   // bright EMI air and top-end sparkle
        { "Mastering Harmonics",{ { "drive", 1.2f }, { "model", 3.0f }, { "tone", 0.1f },
                               { "highcut", 20000.0f }, { "mix", 70.0f }, { "output", 0.0f } } },   // subtle triode harmonics for mastering
        { "Sub-Mix Glue",    { { "drive", 2.6f }, { "model", 2.0f }, { "tone", 0.0f },
                               { "lowcut", 30.0f }, { "mix", 80.0f }, { "output", -1.0f } } },   // parallel console glue for stem sub-mixes

        // --- Lo-Fi & Special FX -------------------------------------------
        { "Vinyl Warmth",    { { "drive", 4.0f }, { "model", 0.0f }, { "tone", -0.3f },
                               { "lowcut", 40.0f }, { "highcut", 12000.0f }, { "mix", 85.0f }, { "output", -1.5f } } },   // warm dusty vinyl record vibe
        { "Dirty Cassette",  { { "drive", 5.5f }, { "model", 0.0f }, { "tone", -0.4f },
                               { "lowcut", 120.0f }, { "highcut", 7000.0f }, { "mix", 100.0f }, { "output", -2.0f } } },   // gnarly worn-out cassette tape
        { "Vintage Radio",   { { "drive", 6.0f }, { "model", 1.0f }, { "tone", -0.2f },
                               { "lowcut", 350.0f }, { "highcut", 4500.0f }, { "mix", 100.0f }, { "output", -2.0f } } },   // mid-focused old transistor radio
        { "AM Broadcast",    { { "drive", 6.5f }, { "model", 3.0f }, { "tone", -0.3f },
                               { "lowcut", 450.0f }, { "highcut", 3200.0f }, { "steep", 1.0f }, { "mix", 100.0f }, { "output", -2.5f } } },   // narrow-band AM radio squawk
        { "Broken Speaker",  { { "drive", 7.5f }, { "model", 4.0f }, { "tone", -0.5f },
                               { "lowcut", 300.0f }, { "highcut", 3000.0f }, { "mix", 100.0f }, { "output", -3.5f } } },   // crackly blown cone speaker
        { "Tape Dropout",    { { "drive", 5.0f }, { "model", 0.0f }, { "tone", -0.55f },
                               { "lowcut", 90.0f }, { "highcut", 5500.0f }, { "mix", 95.0f }, { "output", -2.0f } } },   // wobbly degraded tape dropout texture
        { "Blown Amp",       { { "drive", 8.5f }, { "model", 4.0f }, { "tone", -0.15f },
                               { "lowcut", 150.0f }, { "highcut", 6000.0f }, { "punish", 1.0f }, { "mix", 100.0f }, { "output", -5.0f } } },   // overdriven blown guitar amp grind
        { "Obliterate",      { { "drive", 10.0f }, { "model", 4.0f }, { "tone", 0.2f },
                               { "lowcut", 200.0f }, { "highcut", 5000.0f }, { "punish", 1.0f }, { "steep", 1.0f }, { "mix", 100.0f }, { "output", -7.0f } } },   // full pentode destroy and obliterate
    };
    return presets;
}

} // namespace decap
