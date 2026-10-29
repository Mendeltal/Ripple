#pragma once
#include <cmath>
#include <utility>
#include <vector>

namespace rpPresets
{
    inline float ms (float v)  { return std::log (v / 20.f) / std::log (100.f); }        // free delay time (ms) -> 0..1
    inline float dec (float s) { return std::log (s / 0.3f) / std::log (40.f); }         // reverb decay (s)     -> 0..1
    inline float div (int i)   { return (float) i / 13.f; }                              // sync division index  -> 0..1
    inline float hz (float f)  { return std::log (f / 0.05f) / std::log (240.f); }       // LFO rate (Hz)        -> 0..1

    // division indices: 0 1/32  1 1/16T  2 1/16  3 1/8T  4 1/16.  5 1/8  6 1/4T  7 1/8.  8 1/4  9 1/2T  10 1/4.  11 1/2  12 1/2.  13 1 bar
    enum Cat { Vocal = 0, Guitar, Drums, Keys, Backing };
    inline const char* catNames[5] = { "Vocal", "Guitar", "Drums", "Keys", "Backing" };

    struct Preset { const char* name; int cat; std::vector<std::pair<const char*, float>> v; };

    inline const std::vector<Preset>& all()
    {
        static const std::vector<Preset> p = {
            // ---------------- VOCAL
            { "Vocal Throw", Vocal, { {"dlTime", div (10)}, {"dlFb", .40f}, {"dlTone", .55f}, {"dlMix", .45f}, {"rvSize", .55f}, {"rvDecay", dec (2.4f)}, {"rvMix", .25f} } },
            { "Dotted Slap", Vocal, { {"dlTime", div (5)}, {"dlFb", .18f}, {"dlTone", .60f}, {"dlMix", .35f}, {"tpDrive", .15f}, {"tpMix", .5f}, {"rvSize", .3f}, {"rvDecay", dec (1.0f)}, {"rvMix", .15f} } },
            { "Space Tail", Vocal, { {"dlTime", div (8)}, {"dlFb", .50f}, {"dlTone", .40f}, {"dlMix", .35f}, {"rvSize", .8f}, {"rvDecay", dec (6.f)}, {"rvShim", .2f}, {"rvMix", .5f} } },
            { "Ping-Pong Hook", Vocal, { {"dlTime", div (7)}, {"dlFb", .55f}, {"dlTone", .5f}, {"dlMix", .5f}, {"tpWow", .15f}, {"tpMix", .4f}, {"rvSize", .4f}, {"rvDecay", dec (1.6f)}, {"rvMix", .15f} } },
            { "Tape Echo Lead", Vocal, { {"sync", 0.f}, {"dlTime", ms (320.f)}, {"dlFb", .45f}, {"dlTone", .35f}, {"dlMix", .4f}, {"tpDrive", .35f}, {"tpWow", .25f}, {"tpFlut", .2f}, {"tpMix", .8f}, {"rvSize", .4f}, {"rvDecay", dec (1.5f)}, {"rvMix", .15f} } },
            { "Telephone Throw", Vocal, { {"dlTime", div (8)}, {"dlFb", .40f}, {"dlTone", .5f}, {"dlMix", .5f}, {"lfRadio", .85f}, {"lfCrush", .10f}, {"rvDecay", dec (1.5f)}, {"rvMix", .2f} } },
            { "Ethereal Vox", Vocal, { {"dlTime", div (10)}, {"dlFb", .45f}, {"dlMix", .3f}, {"rvSize", .9f}, {"rvDecay", dec (9.f)}, {"rvShim", .55f}, {"rvMix", .6f}, {"l2Depth", .3f}, {"l2Rate", hz (.12f)}, {"l2Dest", 4.f} } },
            { "Wash Riser", Vocal, { {"dlTime", div (11)}, {"dlFb", .6f}, {"dlTone", .45f}, {"dlMix", .4f}, {"rvSize", .85f}, {"rvDecay", dec (12.f)}, {"rvShim", .35f}, {"rvMix", .7f}, {"l1Rate", hz (.1f)}, {"l1Depth", .4f}, {"l1Dest", 15.f} } },
            // ---------------- GUITAR
            { "Surf Spring", Guitar, { {"sync", 0.f}, {"dlTime", ms (90.f)}, {"dlFb", .25f}, {"dlMix", .3f}, {"rvSize", .35f}, {"rvDecay", dec (1.6f)}, {"rvMix", .45f}, {"tpWow", .1f}, {"tpDrive", .2f}, {"tpMix", .6f} } },
            { "Ambient Swell", Guitar, { {"dlTime", div (10)}, {"dlFb", .55f}, {"dlTone", .45f}, {"dlMix", .5f}, {"rvSize", .85f}, {"rvDecay", dec (7.f)}, {"rvShim", .45f}, {"rvMix", .5f}, {"l2Depth", .35f}, {"l2Rate", hz (.15f)}, {"l2Dest", 4.f} } },
            { "Slapback Tape", Guitar, { {"sync", 0.f}, {"dlTime", ms (110.f)}, {"dlFb", .12f}, {"dlMix", .5f}, {"tpDrive", .4f}, {"tpWow", .1f}, {"tpMix", .9f}, {"rvMix", 0.f} } },
            { "Dub Echo", Guitar, { {"dlTime", div (8)}, {"dlFb", .72f}, {"dlTone", .30f}, {"dlMix", .55f}, {"tpDrive", .3f}, {"tpFlut", .25f}, {"tpMix", .6f}, {"rvSize", .4f}, {"rvDecay", dec (2.f)}, {"rvMix", .2f}, {"l1Rate", hz (.25f)}, {"l1Depth", .3f}, {"l1Dest", 3.f} } },
            { "Shimmer Pad Guitar", Guitar, { {"dlTime", div (11)}, {"dlFb", .4f}, {"dlMix", .3f}, {"rvSize", .9f}, {"rvDecay", dec (10.f)}, {"rvShim", .8f}, {"rvMix", .65f} } },
            { "Lo-Fi Chorus Guitar", Guitar, { {"dlTime", div (6)}, {"dlFb", .25f}, {"dlMix", .2f}, {"tpWow", .45f}, {"tpFlut", .35f}, {"tpDrive", .2f}, {"tpMix", .55f}, {"lfCrush", .10f}, {"lfRate", .15f}, {"lfDust", .2f}, {"lfRadio", .25f}, {"rvSize", .3f}, {"rvDecay", dec (1.2f)}, {"rvMix", .15f} } },
            // ---------------- DRUMS
            { "Room Tape Snare", Drums, { {"sync", 0.f}, {"dlTime", ms (60.f)}, {"dlFb", .1f}, {"dlMix", .15f}, {"rvSize", .3f}, {"rvDecay", dec (1.2f)}, {"rvMix", .4f}, {"tpDrive", .5f}, {"tpMix", .7f} } },
            { "Echo Fill Throw", Drums, { {"dlTime", div (5)}, {"dlFb", .6f}, {"dlMix", .6f}, {"rvDecay", dec (1.5f)}, {"rvMix", .2f} } },
            { "Crushed Break", Drums, { {"dlTime", div (8)}, {"dlFb", .3f}, {"dlMix", .25f}, {"tpDrive", .6f}, {"tpMix", .8f}, {"lfCrush", .55f}, {"lfRate", .45f}, {"lfDust", .2f}, {"lfRadio", .2f}, {"rvMix", .1f} } },
            { "Dub Snare Bomb", Drums, { {"dlTime", div (8)}, {"dlFb", .7f}, {"dlTone", .35f}, {"dlMix", .45f}, {"rvSize", .8f}, {"rvDecay", dec (5.f)}, {"rvMix", .55f} } },
            { "Gated Splash", Drums, { {"dlTime", div (3)}, {"dlFb", .2f}, {"dlMix", .2f}, {"rvSize", .55f}, {"rvDecay", dec (1.0f)}, {"rvMix", .3f}, {"foAmt", .6f}, {"foSpeed", .2f}, {"foDest", 10.f} } },
            // ---------------- KEYS
            { "Dreamy Keys", Keys, { {"dlTime", div (10)}, {"dlFb", .5f}, {"dlTone", .45f}, {"dlMix", .35f}, {"rvSize", .8f}, {"rvDecay", dec (6.f)}, {"rvShim", .35f}, {"rvMix", .45f}, {"tpWow", .25f}, {"tpMix", .4f} } },
            { "Vintage Chorus Keys", Keys, { {"dlMix", 0.f}, {"tpWow", .5f}, {"tpFlut", .4f}, {"tpDrive", .15f}, {"tpMix", .6f}, {"rvSize", .4f}, {"rvDecay", dec (2.f)}, {"rvMix", .25f} } },
            { "Tape Pad Bloom", Keys, { {"dlMix", .2f}, {"rvSize", .95f}, {"rvDecay", dec (11.f)}, {"rvShim", .6f}, {"rvMix", .65f}, {"tpWow", .3f}, {"tpDrive", .25f}, {"tpMix", .5f}, {"drAmt", .4f}, {"drRate", .3f} } },
            { "Lo-Fi Piano", Keys, { {"dlMix", .15f}, {"rvSize", .4f}, {"rvDecay", dec (1.8f)}, {"rvMix", .3f}, {"tpWow", .2f}, {"tpMix", .5f}, {"lfCrush", .2f}, {"lfRate", .25f}, {"lfDust", .45f}, {"lfRadio", .4f}, {"lfWarp", .35f} } },
            { "Pluck Echoes", Keys, { {"dlTime", div (7)}, {"dlFb", .45f}, {"dlTone", .55f}, {"dlMix", .45f}, {"rvSize", .5f}, {"rvDecay", dec (2.5f)}, {"rvMix", .25f}, {"l1Rate", hz (.5f)}, {"l1Depth", .15f}, {"l1Dest", 1.f} } },
            // ---------------- BACKING VOCALS
            { "Wide Backing Wash", Backing, { {"dlTime", div (10)}, {"dlFb", .35f}, {"dlTone", .4f}, {"dlMix", .25f}, {"rvSize", .9f}, {"rvDecay", dec (5.f)}, {"rvMix", .6f}, {"tpWow", .15f}, {"tpMix", .3f} } },
            { "Choir Space", Backing, { {"dlMix", .15f}, {"rvSize", 1.f}, {"rvDecay", dec (12.f)}, {"rvShim", .5f}, {"rvMix", .7f} } },
            { "Pitched Halo", Backing, { {"dlTime", div (8)}, {"dlFb", .3f}, {"dlMix", .2f}, {"rvSize", .8f}, {"rvDecay", dec (8.f)}, {"rvShim", .9f}, {"rvMix", .6f} } },
            { "Vintage Doubler", Backing, { {"sync", 0.f}, {"dlTime", ms (28.f)}, {"dlFb", 0.f}, {"dlMix", .45f}, {"tpWow", .35f}, {"tpFlut", .5f}, {"tpDrive", .15f}, {"tpMix", .8f}, {"rvDecay", dec (1.5f)}, {"rvMix", .2f} } },
            { "Radio Backing", Backing, { {"dlTime", div (5)}, {"dlFb", .3f}, {"dlMix", .2f}, {"rvDecay", dec (3.f)}, {"rvMix", .35f}, {"lfRadio", .7f}, {"lfCrush", .15f}, {"lfDust", .2f} } },
            { "Evolving Backing", Backing, { {"dlMix", .15f}, {"rvSize", .85f}, {"rvDecay", dec (7.f)}, {"rvShim", .3f}, {"rvMix", .5f}, {"l1Rate", hz (.1f)}, {"l1Depth", .5f}, {"l1Dest", 4.f}, {"l2Rate", hz (.15f)}, {"l2Depth", .4f}, {"l2Dest", 7.f}, {"tpWow", .2f}, {"tpMix", .4f}, {"drAmt", .5f} } },
        };
        return p;
    }
}
