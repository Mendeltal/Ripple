#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>
#include <vector>

namespace rp
{
    constexpr int numDest = 18;
    inline const juce::StringArray& destNames()
    {
        static const juce::StringArray n { "Off", "Delay Time", "Delay Feedback", "Delay Tone", "Reverb Size", "Reverb Decay",
                                           "Reverb Shimmer", "Tape Wow", "Tape Drive", "Delay Mix", "Reverb Mix", "Tape Mix",
                                           "Lo-Fi Crush", "Lo-Fi Rate", "Lo-Fi Dust", "Master Mix", "Lo-Fi Warp", "All Pitch" };
        return n;
    }
    // beats (quarter notes) for the 14 sync divisions
    inline const std::array<float, 14>& divBeats()
    {
        static const std::array<float, 14> b { 0.125f, 0.16667f, 0.25f, 0.33333f, 0.375f, 0.5f, 0.66667f, 0.75f, 1.f, 1.33333f, 1.5f, 2.f, 3.f, 4.f };
        return b;
    }
    inline const juce::StringArray& divNames()
    {
        static const juce::StringArray n { "1/32", "1/16T", "1/16", "1/8T", "1/16.", "1/8", "1/4T", "1/8.", "1/4", "1/2T", "1/4.", "1/2", "1/2.", "1 bar" };
        return n;
    }
    inline float decaySeconds (float x) { return 0.3f * std::pow (40.f, x); }
    inline float freeMs (float x) { return 20.f * std::pow (100.f, x); }
    inline float lfoHz (float x) { return 0.05f * std::pow (240.f, x); }

    struct Biquad
    {
        float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;
        void lowpass (double sr, double f, double q = 0.7071);
        void highpass (double sr, double f, double q = 0.7071);
        void bandpass (double sr, double f, double q);
        void lowShelf (double sr, double f, double gainDb);
        void highShelf (double sr, double f, double gainDb);
        inline float process (float x) noexcept { float y = b0 * x + z1; z1 = b1 * x - a1 * y + z2; z2 = b2 * x - a2 * y; return y; }
        void reset() { z1 = z2 = 0; }
    private:
        void set (double B0, double B1, double B2, double A0, double A1, double A2);
    };

    // One-pole parameter smoother
    struct Sm
    {
        float cur = 0.f, tgt = 0.f, k = 0.01f;
        void setTime (double sr, float seconds) { k = 1.f - std::exp (-1.f / (float) (sr * seconds)); }
        void snap (float v) { cur = tgt = v; }
        inline float next() noexcept { cur += (tgt - cur) * k; return cur; }
    };

    // Fractional delay line with 4-point Hermite interpolation
    struct DelayLine
    {
        std::vector<float> buf; int mask = 0, wp = 0;
        void init (int maxSamples) { int n = 1; while (n < maxSamples + 8) n <<= 1; buf.assign ((size_t) n, 0.f); mask = n - 1; wp = 0; }
        inline void write (float x) noexcept { buf[(size_t) wp] = x; wp = (wp + 1) & mask; }
        inline float read (float d) const noexcept
        {
            const float rp = (float) wp - d - 1.f;
            const int i = (int) std::floor (rp); const float f = rp - (float) i;
            const float y0 = buf[(size_t) ((i - 1) & mask)], y1 = buf[(size_t) (i & mask)], y2 = buf[(size_t) ((i + 1) & mask)], y3 = buf[(size_t) ((i + 2) & mask)];
            const float c1 = 0.5f * (y2 - y0), c2 = y0 - 2.5f * y1 + 2.f * y2 - 0.5f * y3, c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
            return ((c3 * f + c2) * f + c1) * f + y1;
        }
    };

    // 8-line feedback delay network, adjustable size.
    class FDN
    {
    public:
        void prepare (double sr);
        void setSize (float s) { scale = s; }
        void setDecay (float t60);
        void setDamping (float c) { damp = c; }
        void tick (float inL, float inR, float& outL, float& outR);
    private:
        static constexpr int N = 8;
        std::array<std::vector<float>, N> line;
        std::array<int, N> len {}, wpos {};
        std::array<float, N> lp {}, gain {}, phase {}, rate {};
        float damp = 0.3f, scale = 1.f; double sampleRate = 48000;
        std::array<std::vector<float>, 4> apBuf; std::array<int, 4> apLen {}, apPos {};
        std::vector<float> pre[2]; int preLen = 0, prePos = 0;
    };

    class PitchShifter
    {
    public:
        void prepare() { buf.assign (8192, 0.f); mask = 8191; wp = 0; ph = 0.f; }
        float process (float x);
    private:
        std::vector<float> buf; int mask = 8191, wp = 0; float ph = 0.f; const float ratio = 2.f; const int win = 2048;
    };
}

class RippleProcessor : public juce::AudioProcessor
{
public:
    RippleProcessor();
    ~RippleProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Ripple"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 14.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float> inLevel { 0.f }, outLevel { 0.f };
    std::atomic<int> presetIndex { 0 };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    float raw (const char* id) const { return apvts.getRawParameterValue (id)->load(); }

    double sr = 48000;
    // tape
    rp::DelayLine tapeLine[2]; float tapePh[2][3] {}; rp::Biquad tapeLp[2], tapeBump[2]; float tapeDc[2] {};
    float wowDrift = 0.f, wowTarget = 0.f, flutNoise = 0.f, flutTarget = 0.f; int noiseCount = 0;
    // delay
    rp::DelayLine dly[2]; rp::Biquad fbLp[2], fbHp[2]; float dlTimeCur = 24000.f;
    // reverb
    rp::FDN fdn; rp::PitchShifter ps[2]; float shimL = 0.f, shimR = 0.f; rp::Biquad rvHp[2];
    // lo-fi
    rp::DelayLine warpLine[2]; float warpPh[2] {}; rp::Biquad radHp[2], radLp[2], dustHp;
    float holdL = 0.f, holdR = 0.f, holdPh = 0.f; juce::Random rng;
    // vibe voicing on wet path
    rp::Biquad vibeA[2], vibeB[2]; int lastVibe = -1; float vibeDrive = 1.f;
    // modulation
    float lfoPh[2] { 0.f, 0.25f }, follower = 0.f, drift = 0.f, driftTarget = 0.f; int driftCount = 0;
    // smoothers
    rp::Sm smLf[5]; rp::Sm smDlMix, smRvMix, smTpMix, smMaster, smDlOn, smRvOn, smTpOn, smLfOn, smBypass, smDlTime, smFb, smDrive, smDepthTape, smShim;
    float lastTone = -1.f, lastDecay = -1.f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RippleProcessor)
};
