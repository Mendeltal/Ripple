#include "PluginEditor.h"
#include "Presets.h"

using namespace juce;

// ============================================================ themes
namespace rui
{
    static Theme makeIce()
    {
        Theme t;
        t.bgTop = Colour (0xff2e3456); t.bgBot = Colour (0xff1b1f38); t.glow = Colour (0xff5a68a8);
        t.card = Colour (0x0cffffff); t.cardLine = Colour (0x1affffff);
        t.text = Colour (0xffd7def0); t.textDim = Colour (0xffaab4d2); t.textFaint = Colour (0xff7a86ae);
        t.track = Colour (0xff46507d); t.knobHi = Colour (0xff6672ac); t.knobLo = Colour (0xff303860); t.knobEdge = Colour (0x5a7882b9); t.pointer = Colour (0xffebf0fc);
        t.accDelay = Colour (0xff96c8eb); t.accRev = Colour (0xff828cf5); t.accTape = Colour (0xff78d7d7); t.accMod = Colour (0xffb9a0f0);
        t.accLofi = Colour (0xffebaf96); t.accMaster = Colour (0xff96d7f0);
        t.chip = Colour (0xff3f5cc8); t.chipText = Colour (0xfff0f5ff); t.pillA = Colour (0xff78cdcd); t.pillB = Colour (0xff96c3eb); t.pillText = Colour (0xff182846);
        t.swatch = Colour (0xff6f8cf0); t.off = Colour (0xff50597f);
        return t;
    }
    static Theme makeHot()
    {
        Theme t = makeIce();
        t.bgTop = Colour (0xff3a1d26); t.bgBot = Colour (0xff1f0e14); t.glow = Colour (0xff8a3040);
        t.text = Colour (0xfff0d9dc); t.textDim = Colour (0xffd2aab0); t.textFaint = Colour (0xffa07a82);
        t.track = Colour (0xff5e3039); t.knobHi = Colour (0xff80454f); t.knobLo = Colour (0xff3a1a22); t.knobEdge = Colour (0x66c88890); t.pointer = Colour (0xfffff0ee);
        t.accDelay = Colour (0xffff9a8c); t.accRev = Colour (0xffff5a68); t.accTape = Colour (0xffffb06a); t.accMod = Colour (0xffe89ab8);
        t.accLofi = Colour (0xffffc9a0); t.accMaster = Colour (0xffff9a8a);
        t.chip = Colour (0xffc8323c); t.pillA = Colour (0xffff8a72); t.pillB = Colour (0xffff5470); t.pillText = Colour (0xff3a0e16);
        t.swatch = Colour (0xffff5050); t.off = Colour (0xff6d3a44);
        return t;
    }
    static Theme makeWarm()
    {
        Theme t = makeIce();
        t.bgTop = Colour (0xff3b281a); t.bgBot = Colour (0xff21150c); t.glow = Colour (0xff8a5a2a);
        t.text = Colour (0xfff2e2d0); t.textDim = Colour (0xffd4b898); t.textFaint = Colour (0xffa08a70);
        t.track = Colour (0xff5e4630); t.knobHi = Colour (0xff80603f); t.knobLo = Colour (0xff3c281a); t.knobEdge = Colour (0x66c8a078); t.pointer = Colour (0xfffff4e6);
        t.accDelay = Colour (0xffffc27a); t.accRev = Colour (0xffff9a52); t.accTape = Colour (0xffe8b45c); t.accMod = Colour (0xffffb89a);
        t.accLofi = Colour (0xffd8a37a); t.accMaster = Colour (0xffffc890);
        t.chip = Colour (0xffd9731f); t.pillA = Colour (0xffffb347); t.pillB = Colour (0xffff8a3d); t.pillText = Colour (0xff3a1c08);
        t.swatch = Colour (0xffff9a3c); t.off = Colour (0xff6e5238);
        return t;
    }
    static Theme makeAir()
    {
        Theme t = makeIce(); t.light = true;
        t.bgTop = Colour (0xffeceff6); t.bgBot = Colour (0xffd0d6e4); t.glow = Colour (0xffffffff);
        t.card = Colour (0x66ffffff); t.cardLine = Colour (0x40788aa8);
        t.text = Colour (0xff23283f); t.textDim = Colour (0xff4a5272); t.textFaint = Colour (0xff7b84a2);
        t.track = Colour (0xffb7bfd4); t.knobHi = Colour (0xfffafbfe); t.knobLo = Colour (0xffc4cbdc); t.knobEdge = Colour (0x66788aa8); t.pointer = Colour (0xff23283f);
        t.accDelay = Colour (0xff3f78a8); t.accRev = Colour (0xff5560c9); t.accTape = Colour (0xff2f9a9a); t.accMod = Colour (0xff7a5fc0);
        t.accLofi = Colour (0xffc0714a); t.accMaster = Colour (0xff3a6f9e);
        t.chip = Colour (0xff5568c8); t.chipText = Colours::white; t.pillA = Colour (0xffb9cdee); t.pillB = Colour (0xffdbe3f4); t.pillText = Colour (0xff23283f);
        t.swatch = Colour (0xffeef1f8); t.off = Colour (0xffaab3cc);
        return t;
    }
    const Theme& theme (int v)
    {
        static const Theme t[4] = { makeIce(), makeHot(), makeWarm(), makeAir() };
        return t[jlimit (0, 3, v)];
    }

    Font ui (float size, bool bold)
    {
        static const String name = []
        {
            const auto all = Font::findAllTypefaceNames();
            for (auto* n : { "Inter", "SF Pro Display", "Helvetica Neue", "Segoe UI" }) if (all.contains (n)) return String (n);
            return Font::getDefaultSansSerifFontName();
        }();
        return Font (FontOptions (name, size, bold ? Font::bold : Font::plain));
    }

    static float textWidth (const Font& f, const String& s)
    {
        GlyphArrangement ga; ga.addLineOfText (f, s, 0.f, 0.f);
        return ga.getBoundingBox (0, -1, true).getWidth();
    }

    void drawTracked (Graphics& g, const String& s, float x, float y, float tr, Justification j)
    {
        const auto f = g.getCurrentFont();
        float total = -tr;
        for (auto c : s) total += textWidth (f, String::charToString (c)) + tr + (c == ' ' ? f.getHeight() * 0.18f : 0.f);
        float cx = j.testFlags (Justification::horizontallyCentred) ? x - total * 0.5f : (j.testFlags (Justification::right) ? x - total : x);
        for (auto c : s)
        {
            const float w = textWidth (f, String::charToString (c)) + (c == ' ' ? f.getHeight() * 0.18f : 0.f);
            g.drawText (String::charToString (c), Rectangle<float> (cx - 4.f, y - 10.f, w + 8.f, 20.f), Justification::centred, false);
            cx += w + tr;
        }
    }

    // ------------------------------------------------------------ Knob
    Rectangle<int> Knob::boundsFor (float cx, float cy, float radius)
    {
        const int w = (int) (2.f * (radius + 18.f)), top = (int) (radius + 14.f);
        return { (int) (cx - (float) w * 0.5f), (int) (cy - (float) top), w, top + (int) radius + 36 };
    }

    Knob::Knob (AudioProcessorValueTreeState& a, const String& id, const String& label, float radius, const Theme*& themeRef,
                std::function<Colour (const Theme&)> accent, bool big)
        : lbl (label), r (radius), cy (radius + 14.f), bigKnob (big), th (themeRef), acc (std::move (accent)), param (a.getParameter (id))
    {
        setSliderStyle (Slider::RotaryVerticalDrag);
        setTextBoxStyle (Slider::NoTextBox, true, 0, 0);
        setRotaryParameters (degreesToRadians (225.f), degreesToRadians (495.f), true);
        setMouseDragSensitivity (200);
        setDoubleClickReturnValue (true, param->convertFrom0to1 (param->getDefaultValue()));
        rebuild();
    }

    void Knob::rebuild()
    {
        const float S = 2.f, pad = r + 22.f;
        body = Image (Image::ARGB, (int) (pad * 2.f * S), (int) (pad * 2.f * S), true);
        Graphics g (body);
        g.addTransform (AffineTransform::scale (S));
        Path p; p.addEllipse (pad - r, pad - r, r * 2.f, r * 2.f);
        DropShadow (Colours::black.withAlpha (th->light ? 0.30f : 0.55f), (int) (r * 0.55f), { 0, (int) (r * 0.2f) }).drawForPath (g, p);
        if (! th->light) DropShadow (th->knobHi.withAlpha (0.20f), (int) (r * 0.25f), { -1, -2 }).drawForPath (g, p);
        ColourGradient cg (th->knobHi, pad - r * 0.45f, pad - r * 0.6f, th->knobLo, pad + r * 0.8f, pad + r * 0.9f, true);
        g.setGradientFill (cg); g.fillPath (p);
        g.setColour (th->knobEdge); g.strokePath (p, PathStrokeType (1.f));
        // inner rim light
        g.setColour (Colours::white.withAlpha (th->light ? 0.6f : 0.07f));
        g.drawEllipse (pad - r + 1.2f, pad - r + 1.2f, r * 2.f - 2.4f, r * 2.f - 2.4f, 1.f);
    }

    void Knob::paint (Graphics& g)
    {
        const float cx = (float) getWidth() * 0.5f, pad = r + 22.f;
        g.drawImage (body, Rectangle<float> (cx - pad, cy - pad, pad * 2.f, pad * 2.f));
        const Colour ac = acc (*th);
        const float ar = r + (bigKnob ? 8.f : 6.f), w = bigKnob ? 3.6f : 2.6f;
        const bool hot = isMouseOverOrDragging();
        Path track; track.addCentredArc (cx, cy, ar, ar, 0.f, degreesToRadians (-135.f), degreesToRadians (135.f), true);
        g.setColour (th->track); g.strokePath (track, PathStrokeType (w, PathStrokeType::curved, PathStrokeType::rounded));
        const float pos = (float) valueToProportionOfLength (getValue());
        const float a = degreesToRadians (-135.f + 270.f * pos);
        if (pos > 0.002f)
        {
            Path v; v.addCentredArc (cx, cy, ar, ar, 0.f, degreesToRadians (-135.f), a, true);
            g.setColour (ac.withAlpha (hot ? 0.34f : 0.2f)); g.strokePath (v, PathStrokeType (w + 4.f, PathStrokeType::curved, PathStrokeType::rounded));
            g.setColour (ac); g.strokePath (v, PathStrokeType (w, PathStrokeType::curved, PathStrokeType::rounded));
        }
        Path ptr; ptr.addRoundedRectangle (-(bigKnob ? 1.8f : 1.4f), -r * 0.80f, bigKnob ? 3.6f : 2.8f, r * 0.52f, 1.3f);
        g.setColour (th->pointer); g.fillPath (ptr, AffineTransform::rotation (a).translated (cx, cy));

        const bool showVal = isMouseButtonDown() || (hot && valueText);
        g.setFont (ui (bigKnob ? 13.f : (r > 22.f ? 11.5f : 11.f)));
        g.setColour (showVal ? ac : th->textDim);
        const String t = showVal && valueText ? valueText ((float) getValue()) : lbl;
        g.drawText (t, Rectangle<float> (0.f, cy + r + (bigKnob ? 22.f : 12.f), (float) getWidth(), 18.f), Justification::centred, false);
    }

    // ------------------------------------------------------------ TogButton
    TogButton::TogButton (AudioProcessorValueTreeState& a, const String& id, const String& text, Style s, const Theme*& themeRef,
                                std::function<Colour (const Theme&)> accent)
        : param (a.getParameter (id)), txt (text), style (s), th (themeRef), acc (std::move (accent)),
          att (*a.getParameter (id), [this] (float) { repaint(); })
    {
        setMouseCursor (MouseCursor::PointingHandCursor);
        att.sendInitialUpdate();
    }

    void TogButton::mouseDown (const MouseEvent&)
    {
        param->beginChangeGesture(); param->setValueNotifyingHost (isOn() ? 0.f : 1.f); param->endChangeGesture(); repaint();
    }

    void TogButton::paint (Graphics& g)
    {
        auto b = getLocalBounds().toFloat();
        const Colour ac = acc ? acc (*th) : th->accRev;
        const bool on = isOn();
        if (style == Style::Lamp)
        {
            const auto c = b.getCentre();
            if (on)
            {
                ColourGradient gl (ac.withAlpha (0.55f), c.x, c.y, Colours::transparentBlack, c.x + 11.f, c.y, true);
                g.setGradientFill (gl); g.fillEllipse (c.x - 11.f, c.y - 11.f, 22.f, 22.f);
            }
            g.setColour (on ? ac : th->off); g.fillEllipse (c.x - 5.f, c.y - 5.f, 10.f, 10.f);
            return;
        }
        if (style == Style::Switch)
        {
            const float h = 22.f, w = 46.f; const auto r = Rectangle<float> (0.f, 0.f, w, h).withCentre (b.getCentre());
            g.setColour (on ? ac : th->off.withAlpha (0.6f)); g.fillRoundedRectangle (r, h * 0.5f);
            g.setColour (Colours::white.withAlpha (0.95f));
            g.fillEllipse (on ? r.getRight() - h + 2.f : r.getX() + 2.f, r.getY() + 2.f, h - 4.f, h - 4.f);
            return;
        }
        // pill halves
        Path p; const float rad = b.getHeight() * 0.5f;
        p.addRoundedRectangle (b.getX(), b.getY(), b.getWidth(), b.getHeight(),  rad, rad, style == Style::PillLeft, style == Style::PillRight, style == Style::PillLeft, style == Style::PillRight);
        g.setColour (on ? ac : Colour (th->light ? 0x14000000 : 0x14ffffff)); g.fillPath (p);
        g.setColour (on ? (th->light ? Colours::white : Colour (0xff191d3a)) : th->textDim);
        g.setFont (ui (12.5f, true)); g.drawText (txt, b, Justification::centred, false);
    }

    // ------------------------------------------------------------ ChoicePill
    ChoicePill::ChoicePill (AudioProcessorValueTreeState& a, const String& id, const Theme*& themeRef, std::function<Colour (const Theme&)> accent)
        : param (a.getParameter (id)), th (themeRef), acc (std::move (accent)), att (*a.getParameter (id), [this] (float) { repaint(); })
    {
        setMouseCursor (MouseCursor::PointingHandCursor); att.sendInitialUpdate();
    }
    void ChoicePill::paint (Graphics& g)
    {
        auto b = getLocalBounds().toFloat();
        const Colour ac = acc (*th);
        g.setColour (ac.withAlpha (th->light ? 0.16f : 0.20f)); g.fillRoundedRectangle (b, b.getHeight() * 0.5f);
        g.setColour (ac.withAlpha (0.45f)); g.drawRoundedRectangle (b.reduced (0.5f), b.getHeight() * 0.5f, 0.8f);
        g.setColour (th->light ? ac.darker (0.3f) : ac.brighter (0.35f));
        g.setFont (ui (10.5f, true)); g.drawText (param->getCurrentValueAsText(), b, Justification::centred, false);
    }
    void ChoicePill::mouseDown (const MouseEvent&)
    {
        PopupMenu m; const int n = param->getNumSteps(); const int cur = (int) std::lround (param->getValue() * (float) (n - 1));
        const auto names = rp::destNames();
        for (int i = 0; i < names.size(); ++i) m.addItem (i + 1, names[i], true, i == cur);
        m.showMenuAsync (PopupMenu::Options().withTargetComponent (this).withMinimumWidth (getWidth()), [this] (int r)
        {
            if (r <= 0) return;
            param->beginChangeGesture(); param->setValueNotifyingHost (param->convertTo0to1 ((float) (r - 1))); param->endChangeGesture();
        });
    }
}

// ============================================================ value formatting
static String fmtValue (const String& id, float v)
{
    auto pct = [] (float x) { return String ((int) std::lround (x * 100.f)) + " %"; };
    if (id == "dlFb") return pct (v / 0.95f);
    if (id == "rvDecay") return String (rp::decaySeconds (v), 1) + " s";
    if (id == "l1Rate" || id == "l2Rate") return String (rp::lfoHz (v), 2) + " Hz";
    return pct (v);
}

// ============================================================ Editor
using namespace rui;

RippleEditor::RippleEditor (RippleProcessor& p) : AudioProcessorEditor (&p), proc (p)
{
    vibeIdx = (int) std::lround (rawParam ("vibe")); th = &theme (vibeIdx);
    rebuildStatic();
    addAndMakeVisible (content);

    auto& A = proc.apvts;
    const std::function<Colour (const Theme&)> cDl = [] (const Theme& t) { return t.accDelay; }, cRv = [] (const Theme& t) { return t.accRev; },
        cTp = [] (const Theme& t) { return t.accTape; }, cMod = [] (const Theme& t) { return t.accMod; },
        cLf = [] (const Theme& t) { return t.accLofi; }, cMa = [] (const Theme& t) { return t.accMaster; },
        cMix = [] (const Theme& t) { return t.light ? t.accMaster : t.accMaster.brighter (0.35f); };

    auto addKnob = [&] (const char* id, const char* label, float cx, float cy, float r, std::function<Colour (const Theme&)> c, bool big = false) -> Knob*
    {
        auto* k = knobs.add (new Knob (A, id, label, r, th, std::move (c), big));
        k->setBounds (Knob::boundsFor (cx, cy, r));
        const String sid (id);
        k->valueText = [sid] (float v) { return fmtValue (sid, v); };
        content.addAndMakeVisible (k);
        attach.push_back (std::make_unique<SliderParameterAttachment> (*A.getParameter (id), *k));
        return k;
    };
    // custom value text for the delay time knob
    const int RY0 = 372, CY0 = 588;
    // FX cards
    struct Fx { int x0; const char* ids[4]; const char* lab[3]; std::function<Colour (const Theme&)> col; };
    Fx fx[3] = { { 30,  { "dlTime", "dlFb", "dlTone", "dlMix" },  { "Time", "Feedback", "Tone" },   cDl },
                 { 374, { "rvSize", "rvDecay", "rvShim", "rvMix" }, { "Size", "Decay", "Shimmer" }, cRv },
                 { 718, { "tpWow", "tpFlut", "tpDrive", "tpMix" },  { "Wow", "Flutter", "Drive" },  cTp } };
    Knob* mixK[3] {};
    for (int f = 0; f < 3; ++f)
    {
        for (int i = 0; i < 3; ++i) addKnob (fx[f].ids[i], fx[f].lab[i], (float) (fx[f].x0 + 52 + i * 62), (float) (RY0 + 98), 21.f, fx[f].col);
        mixK[f] = addKnob (fx[f].ids[3], "MIX", (float) (fx[f].x0 + 285), (float) (RY0 + 98), 26.f, cMix);
    }
    // delay-time knob shows the musical division when synced
    for (auto* k : knobs)
        if (k->getName().isEmpty()) {}
    knobs[0]->valueText = [this] (float v)
    {
        if (rawParam ("sync") > 0.5f) return rp::divNames()[jlimit (0, 13, (int) std::lround (v * 13.f))];
        const float ms = rp::freeMs (v); return ms >= 1000.f ? String (ms / 1000.f, 2) + " s" : String ((int) ms) + " ms";
    };
    // modulation
    struct Mod { const char* title; const char* a; const char* b; const char* la; const char* lb; const char* dest; };
    const Mod mods[4] = { { "LFO 1", "l1Rate", "l1Depth", "Rate", "Depth", "l1Dest" }, { "LFO 2", "l2Rate", "l2Depth", "Rate", "Depth", "l2Dest" },
                          { "FOLLOWER", "foAmt", "foSpeed", "Amount", "Speed", "foDest" }, { "DRIFT", "drAmt", "drRate", "Amount", "Rate", "drDest" } };
    for (int i = 0; i < 4; ++i)
    {
        const int bx = 60 + i * 160;
        addKnob (mods[i].a, mods[i].la, (float) (bx + 22), (float) (CY0 + 116), 19.f, cMod);
        addKnob (mods[i].b, mods[i].lb, (float) (bx + 82), (float) (CY0 + 116), 19.f, cMod);
        auto* pl = pills.add (new ChoicePill (A, mods[i].dest, th, cMod));
        pl->setBounds (bx, CY0 + 170, 112, 20); content.addAndMakeVisible (pl);
    }
    // lo-fi
    const char* lf[5][2] = { { "lfCrush", "Crush" }, { "lfRate", "Rate" }, { "lfDust", "Dust" }, { "lfRadio", "Radio" }, { "lfWarp", "Warp" } };
    for (int i = 0; i < 5; ++i) addKnob (lf[i][0], lf[i][1], (float) (748 + i * 58), (float) (CY0 + 112), 19.f, cLf);
    // master
    addKnob ("master", "Master Mix", 1157.f, (float) (RY0 + 150), 54.f, cMa, true);

    // lamps (power) on each card title
    struct Lamp { const char* id; int x0, y0; std::function<Colour (const Theme&)> c; };
    const Lamp lamps[5] = { { "dlOn", 30, RY0, cDl }, { "rvOn", 374, RY0, cRv }, { "tpOn", 718, RY0, cTp }, { "modOn", 30, CY0, cMod }, { "lfOn", 718, CY0, cLf } };
    for (auto& l : lamps)
    {
        auto* t = toggles.add (new TogButton (A, l.id, "", TogButton::Style::Lamp, th, l.c));
        t->setBounds (l.x0 + 12, l.y0 + 10, 26, 26); content.addAndMakeVisible (t);
    }
    // header toggles + link
    auto* sy = toggles.add (new TogButton (A, "sync", "Tempo Sync", TogButton::Style::PillLeft, th, cRv));
    sy->setBounds (950, 34, 110, 30); content.addAndMakeVisible (sy);
    auto* by = toggles.add (new TogButton (A, "bypass", "Bypass", TogButton::Style::PillRight, th, [] (const Theme&) { return Colour (0xffe8645a); }));
    by->setBounds (1060, 34, 80, 30); content.addAndMakeVisible (by);
    auto* lk = toggles.add (new TogButton (A, "link", "", TogButton::Style::Switch, th, cMa));
    lk->setBounds (1096, RY0 + 291, 62, 40); content.addAndMakeVisible (lk);

    // link-all behaviour: moving one FX mix knob by hand moves the others by the same amount
    for (int i = 0; i < 3; ++i)
    {
        mixPrev[i] = (float) mixK[i]->getValue();
        mixK[i]->onValueChange = [this, i, mixK] () mutable
        {
            const float v = (float) mixK[i]->getValue();
            if (linkGuard || ! mixK[i]->isMouseButtonDown() || rawParam ("link") < 0.5f) { if (! linkGuard) mixPrev[i] = v; return; }
            const float d = v - mixPrev[i]; mixPrev[i] = v;
            linkGuard = true;
            static const char* ids[3] = { "dlMix", "rvMix", "tpMix" };
            for (int j = 0; j < 3; ++j)
                if (j != i)
                    if (auto* pr = proc.apvts.getParameter (ids[j]))
                    {
                        const float nv = jlimit (0.f, 1.f, pr->getValue() + d);
                        pr->beginChangeGesture(); pr->setValueNotifyingHost (nv); pr->endChangeGesture(); mixPrev[j] = nv;
                    }
            linkGuard = false;
        };
    }

    vibeAtt = std::make_unique<ParameterAttachment> (*A.getParameter ("vibe"), [this] (float v)
    {
        const int idx = (int) std::lround (v);
        if (idx == vibeIdx) return;
        vibeIdx = idx; th = &theme (idx); rebuildStatic();
        for (auto* k : knobs) k->rebuild();
        content.repaint(); for (auto* k : knobs) k->repaint(); for (auto* t : toggles) t->repaint(); for (auto* pl : pills) pl->repaint();
    });

    setResizable (true, true);
    getConstrainer()->setFixedAspectRatio ((double) W / (double) H);
    setResizeLimits (640, 430, 1920, 1290);
    setSize (W, H);
    startTimerHz (30);
}

RippleEditor::~RippleEditor() { stopTimer(); }

void RippleEditor::resized()
{
    content.setBounds (0, 0, W, H);
    content.setTransform (AffineTransform::scale ((float) getWidth() / (float) W));
}

void RippleEditor::timerCallback()
{
    const float db = 20.f * std::log10 (jmax (proc.outLevel.load(), 1.0e-5f));
    const float n = jlimit (0.f, 1.f, (db + 60.f) / 60.f);
    outShown += (n - outShown) * (n > outShown ? 0.5f : 0.15f);
    pulse += (jlimit (0.f, 1.f, proc.inLevel.load() * 2.5f) - pulse) * 0.3f;
    phase += 0.07f;
    for (int i = 0; i < 3; ++i)
    {
        static const char* ids[3] = { "dlMix", "rvMix", "tpMix" };
        bool down = false; for (auto* k : knobs) if (k->isMouseButtonDown()) down = true;
        if (! down) mixPrev[i] = rawParam (ids[i]);
    }
    content.repaint (display.expanded (0, 20));
    content.repaint (1080, 735, 160, 30);
    content.repaint (presetPill.expanded (4));
}

String RippleEditor::delayDesc() const
{
    const float t = rawParam ("dlTime");
    if (rawParam ("sync") > 0.5f) return rp::divNames()[jlimit (0, 13, (int) std::lround (t * 13.f))];
    const float ms = rp::freeMs (t); return ms >= 1000.f ? String (ms / 1000.f, 2) + " s" : String ((int) ms) + " ms";
}

// ------------------------------------------------------------ presets
void RippleEditor::applyPreset (int idx)
{
    const auto& list = rpPresets::all();
    idx = jlimit (0, (int) list.size() - 1, idx);
    const StringArray keep { "vibe", "link", "bypass" };
    for (auto* p : proc.getParameters())
        if (auto* r = dynamic_cast<RangedAudioParameter*> (p))
        {
            if (keep.contains (r->paramID)) continue;
            r->beginChangeGesture(); r->setValueNotifyingHost (r->getDefaultValue()); r->endChangeGesture();
        }
    for (auto& kv : list[(size_t) idx].v)
        if (auto* r = proc.apvts.getParameter (kv.first))
        {
            r->beginChangeGesture(); r->setValueNotifyingHost (r->convertTo0to1 (kv.second)); r->endChangeGesture();
        }
    proc.presetIndex = idx;
    content.repaint();
}
void RippleEditor::stepPreset (int d)
{
    const int n = (int) rpPresets::all().size();
    applyPreset ((proc.presetIndex.load() + d + n) % n);
}
void RippleEditor::setVibe (int v)
{
    if (auto* p = proc.apvts.getParameter ("vibe")) { p->beginChangeGesture(); p->setValueNotifyingHost (p->convertTo0to1 ((float) v)); p->endChangeGesture(); }
}
void RippleEditor::showPresetMenu()
{
    PopupMenu m; const auto& list = rpPresets::all(); int lastCat = -1; PopupMenu sub;
    for (int c = 0; c < 5; ++c)
    {
        PopupMenu s;
        for (int i = 0; i < (int) list.size(); ++i) if (list[(size_t) i].cat == c) s.addItem (i + 1, list[(size_t) i].name, true, i == proc.presetIndex.load());
        m.addSubMenu (rpPresets::catNames[c], s);
    }
    juce::ignoreUnused (lastCat, sub);
    m.showMenuAsync (PopupMenu::Options().withTargetComponent (this), [this] (int r) { if (r > 0) applyPreset (r - 1); });
}

void RippleEditor::contentClick (const MouseEvent& e)
{
    const auto pt = e.getPosition();
    if (presetPill.contains (pt))
    {
        if (pt.x < presetPill.getX() + 44) stepPreset (-1);
        else if (pt.x > presetPill.getRight() - 44) stepPreset (1);
        else showPresetMenu();
    }
    else if (chipBar.contains (pt))
    {
        const int c = jlimit (0, 4, (pt.x - chipBar.getX()) / 92);
        const auto& list = rpPresets::all();
        for (int i = 0; i < (int) list.size(); ++i) if (list[(size_t) i].cat == c) { applyPreset (i); break; }
    }
    else if (vibeArea.contains (pt))
    {
        for (int i = 0; i < 4; ++i) if (std::abs (pt.x - (848 + 26 * i)) <= 13) setVibe (i);
    }
    else if (modeBar.contains (pt))
    {
        static const float modes[4][5] = { { .12f, .10f, .15f, .35f, .35f }, { 0.f, 0.f, .60f, .20f, .15f }, { .05f, .20f, .10f, .85f, 0.f }, { .65f, .55f, 0.f, 0.f, 0.f } };
        static const char* ids[5] = { "lfCrush", "lfRate", "lfDust", "lfRadio", "lfWarp" };
        const int m = jlimit (0, 3, (pt.x - modeBar.getX()) * 4 / modeBar.getWidth());
        for (int i = 0; i < 5; ++i) if (auto* p = proc.apvts.getParameter (ids[i])) { p->beginChangeGesture(); p->setValueNotifyingHost (modes[m][i]); p->endChangeGesture(); }
        if (auto* p = proc.apvts.getParameter ("lfOn")) { p->beginChangeGesture(); p->setValueNotifyingHost (1.f); p->endChangeGesture(); }
    }
    else if (menuBtn.contains (pt))
    {
        PopupMenu m; m.addItem (1, "Previous preset"); m.addItem (2, "Next preset"); m.addSeparator(); m.addItem (3, "Reset to default (Vocal Throw)");
        m.showMenuAsync (PopupMenu::Options().withTargetScreenArea (localAreaToGlobal (menuBtn)), [this] (int r)
        { if (r == 1) stepPreset (-1); else if (r == 2) stepPreset (1); else if (r == 3) applyPreset (0); });
    }
}

// ------------------------------------------------------------ static layer
void RippleEditor::rebuildStatic()
{
    const auto& t = *th; const float S = 2.f;
    staticLayer = Image (Image::ARGB, (int) (W * S), (int) (H * S), true);
    Graphics g (staticLayer);
    g.addTransform (AffineTransform::scale (S));
    g.setGradientFill (ColourGradient (t.bgTop, 0.f, 0.f, t.bgBot, 0.f, (float) H, false)); g.fillAll();
    g.setGradientFill (ColourGradient (t.glow.withAlpha (t.light ? 0.7f : 0.30f), 640.f, 40.f, Colours::transparentBlack, 640.f, 760.f, true)); g.fillRect (0, 0, W, H);
    // header
    g.setColour (t.text); g.drawEllipse (40.f, 30.f, 32.f, 32.f, 2.f); g.drawEllipse (50.f, 30.f, 32.f, 32.f, 2.f);
    g.setFont (ui (17.f, true)); drawTracked (g, "RIPPLE", 92.f, 46.f, 5.f, Justification::left);
    g.setColour (t.textFaint); g.setFont (ui (14.f)); g.drawText ("Throw Engine", 212, 36, 160, 20, Justification::left, false);
    g.setFont (ui (10.f, true)); g.setColour (t.textFaint); drawTracked (g, "VIBE", 790.f, 49.f, 2.f, Justification::left);
    // display captions
    g.setFont (ui (10.f, true)); g.setColour (t.textFaint);
    drawTracked (g, String::fromUTF8 ("ECHO TRAIL  \xc2\xb7  REVERB WASH"), 1240.f, 98.f, 2.f, Justification::right);
    drawTracked (g, "30 PRESETS", 30.f, 337.f, 2.f, Justification::left);
    g.setFont (ui (10.5f));
    const float L = std::log2 (65.f), x0 = 40.f, x1 = 1240.f;
    const struct { const char* s; float t; } ticks[] = { { "0 ms", 0 }, { "125", 125 }, { "250", 250 }, { "500", 500 }, { "1 s", 1000 }, { "2 s", 2000 }, { "4 s", 4000 }, { "8 s", 8000 } };
    for (auto& tk : ticks) { const float px = x0 + (x1 - x0) * (std::log2 (1.f + tk.t / 125.f) / L); g.drawText (tk.s, (int) px - 30, 282, 60, 16, Justification::centred, false); }

    auto card = [&] (int cx0, int cy0, int cx1, int cy1, const char* title)
    {
        Path p; p.addRoundedRectangle ((float) cx0, (float) cy0, (float) (cx1 - cx0), (float) (cy1 - cy0), 16.f);
        DropShadow (Colours::black.withAlpha (t.light ? 0.16f : 0.38f), 16, { 0, 8 }).drawForPath (g, p);
        g.setColour (t.card); g.fillPath (p);
        g.setColour (t.cardLine); g.strokePath (p, PathStrokeType (1.f));
        g.setColour (t.textDim); g.setFont (ui (11.5f, true)); drawTracked (g, title, (float) cx0 + 52.f, (float) cy0 + 23.f, 3.f, Justification::left);
    };
    const int RY0 = 372, RY1 = 572, CY0 = 588, CY1 = 800;
    card (30, RY0, 360, RY1, "DELAY"); card (374, RY0, 704, RY1, "REVERB"); card (718, RY0, 1048, RY1, "TAPE");
    card (30, CY0, 704, CY1, "MODULATION"); card (718, CY0, 1048, CY1, "LO-FI"); card (1064, RY0, 1250, CY1, "MASTER");
    g.setColour (t.light ? Colour (0x30000000) : Colour (0x1effffff));
    for (int x0c : { 30, 374, 718 }) g.drawLine ((float) x0c + 240.f, (float) RY0 + 60.f, (float) x0c + 240.f, (float) RY0 + 160.f, 1.f);
    // master lamp (always on)
    g.setColour (t.accMaster); g.fillEllipse (1064.f + 20.f, (float) RY0 + 18.f, 10.f, 10.f);
    g.setFont (ui (10.f, true)); g.setColour (t.textFaint);
    const char* sub[4] = { "LFO 1", "LFO 2", "FOLLOWER", "DRIFT" };
    for (int i = 0; i < 4; ++i) drawTracked (g, sub[i], (float) (60 + i * 160 + 52), (float) CY0 + 68.f, 2.f, Justification::centred);
        g.setFont (ui (9.5f, true)); g.setColour (t.accMaster.withAlpha (0.9f));
    {
        Path pl; pl.addRoundedRectangle (1097.f, (float) RY0 + 246.f, 120.f, 18.f, 9.f);
        g.setColour (t.accMaster.withAlpha (0.16f)); g.fillPath (pl);
        g.setColour (t.accMaster); g.fillEllipse (1109.f, (float) RY0 + 252.f, 6.f, 6.f);
        drawTracked (g, "AUTOMATE", 1160.f, (float) RY0 + 255.f, 2.f, Justification::centred);
    }
    g.setFont (ui (12.f, true)); g.setColour (t.textDim); g.drawText ("Link all", 1168, RY0 + 300, 70, 22, Justification::left, false);
    g.setFont (ui (10.f)); g.setColour (t.textFaint); g.drawText ("moves the 3 FX mixes together", 1064, RY0 + 328, 186, 14, Justification::centred, false);
    g.setFont (ui (10.f, true)); drawTracked (g, "OUT", 1088.f, (float) CY1 - 64.f, 2.f, Justification::left);
    // signature: small, quiet, same hue family as the UI
    g.setFont (Font (FontOptions (ui (14.f).getTypefaceName(), 14.f, Font::italic)));
    g.setColour (t.textFaint.withAlpha (0.9f)); g.drawText ("Tal Mendel", 30, 818, 160, 24, Justification::left, false);
    g.setFont (ui (10.f)); g.setColour (t.textFaint.withAlpha (0.7f)); g.drawText ("v1.0", 1190, 820, 60, 20, Justification::right, false);
}

// ------------------------------------------------------------ dynamic paint
void RippleEditor::paintContent (Graphics& g)
{
    g.drawImage (staticLayer, Rectangle<float> (0.f, 0.f, (float) W, (float) H));
    drawDisplay (g);
    const auto& t = *th;

    // preset pill
    {
        auto r = presetPill.toFloat();
        Path p; p.addRoundedRectangle (r, 16.f);
        DropShadow (Colours::black.withAlpha (t.light ? 0.18f : 0.4f), 8, { 0, 5 }).drawForPath (g, p);
        g.setGradientFill (ColourGradient (t.pillA, r.getX(), 0.f, t.pillB, r.getRight(), 0.f, false)); g.fillPath (p);
        const auto& list = rpPresets::all();
        const String nm = list[(size_t) jlimit (0, (int) list.size() - 1, proc.presetIndex.load())].name;
        g.setColour (t.pillText);
        g.setFont (ui (13.f, true)); g.drawText (nm + String::fromUTF8 ("  \xc2\xb7  ") + delayDesc(), r.reduced (40.f, 0.f), Justification::centred, false);
        g.setFont (ui (20.f)); g.drawText (String::fromUTF8 ("\xe2\x80\xb9"), r.withWidth (40.f), Justification::centred, false);
        g.drawText (String::fromUTF8 ("\xe2\x80\xba"), r.withLeft (r.getRight() - 40.f), Justification::centred, false);
    }
    // category chips
    {
        auto r = chipBar.toFloat();
        g.setColour (t.light ? Colour (0x14000000) : Colour (0x46000000)); g.fillRoundedRectangle (r, 20.f);
        const int cur = rpPresets::all()[(size_t) jlimit (0, 29, proc.presetIndex.load())].cat;
        for (int i = 0; i < 5; ++i)
        {
            auto c = Rectangle<float> (r.getX() + 92.f * (float) i + 3.f, r.getY() + 3.f, 86.f, 34.f);
            if (i == cur)
            {
                Path p; p.addRoundedRectangle (c, 17.f);
                DropShadow (t.chip.withAlpha (0.55f), 8, { 0, 4 }).drawForPath (g, p);
                g.setColour (t.chip); g.fillPath (p);
            }
            g.setColour (i == cur ? t.chipText : t.textDim); g.setFont (ui (13.f, true));
            g.drawText (rpPresets::catNames[i], c, Justification::centred, false);
        }
    }
    // vibe swatches
    {
        const Colour sw[4] = { theme (0).swatch, theme (1).swatch, theme (2).swatch, theme (3).swatch };
        const char* nm[4] = { "Ice", "Hot", "Warm", "Air" }; juce::ignoreUnused (nm);
        for (int i = 0; i < 4; ++i)
        {
            const float cx = 848.f + 26.f * (float) i, cy = 49.f;
            ColourGradient gr (sw[i].brighter (0.35f), cx - 3.f, cy - 4.f, sw[i].darker (0.25f), cx + 6.f, cy + 7.f, true);
            g.setGradientFill (gr); g.fillEllipse (cx - 8.f, cy - 8.f, 16.f, 16.f);
            g.setColour (Colours::black.withAlpha (0.25f)); g.drawEllipse (cx - 8.f, cy - 8.f, 16.f, 16.f, 1.f);
            if (i == vibeIdx) { g.setColour (t.text); g.drawEllipse (cx - 11.5f, cy - 11.5f, 23.f, 23.f, 1.6f); }
        }
    }
    // lo-fi mode strip
    {
        auto r = modeBar.toFloat(); const Colour ac = t.accLofi;
        g.setColour (ac.withAlpha (0.16f)); g.fillRoundedRectangle (r, 10.f);
        g.setColour (ac.withAlpha (0.4f)); g.drawRoundedRectangle (r.reduced (0.5f), 10.f, 0.8f);
        g.setFont (ui (10.f, true)); g.setColour (t.light ? ac.darker (0.3f) : ac.brighter (0.4f));
        const char* nm[4] = { "Cassette", "Vinyl", "Radio", "Bit" };
        for (int i = 0; i < 4; ++i) g.drawText (nm[i], Rectangle<float> (r.getX() + r.getWidth() * 0.25f * (float) i, r.getY(), r.getWidth() * 0.25f, r.getHeight()), Justification::centred, false);
        g.setColour (ac.withAlpha (0.25f));
        for (int i = 1; i < 4; ++i) g.drawLine (r.getX() + r.getWidth() * 0.25f * (float) i, r.getY() + 5.f, r.getX() + r.getWidth() * 0.25f * (float) i, r.getBottom() - 5.f, 1.f);
    }
    // menu dots
    g.setColour (t.textDim); for (int i = -1; i <= 1; ++i) g.fillEllipse (1181.f + 8.f * (float) i, 47.f, 4.f, 4.f);
    // output meter
    {
        Rectangle<float> r (1088.f, 746.f, 138.f, 10.f);
        g.setColour (t.light ? Colour (0x22000000) : Colour (0xff0f1228)); g.fillRoundedRectangle (r, 5.f);
        g.setColour (t.accMaster); g.fillRoundedRectangle (r.withWidth (jmax (6.f, r.getWidth() * outShown)), 5.f);
        const float db = 20.f * std::log10 (jmax (proc.outLevel.load(), 1.0e-5f));
        g.setFont (ui (11.f, true)); g.setColour (t.textDim);
        g.drawText (db < -59.f ? String ("-inf") : String (db, 1) + " dB", 1130, 728, 96, 16, Justification::right, false);
    }
}

void RippleEditor::drawDisplay (Graphics& g)
{
    const auto& t = *th;
    const float x0 = 40.f, x1 = 1240.f, baseY = 268.f, W2 = x1 - x0, L = std::log2 (65.f);
    auto xOf = [&] (float sec) { return x0 + W2 * (std::log2 (1.f + sec * 1000.f / 125.f) / L); };

    const bool rvOn = rawParam ("rvOn") > 0.5f, dlOn = rawParam ("dlOn") > 0.5f;
    const float rvMix = rawParam ("rvMix"), dlMix = rawParam ("dlMix"), fb = rawParam ("dlFb");
    const float decay = rp::decaySeconds (rawParam ("rvDecay")) * (0.8f + 0.5f * rawParam ("rvSize"));
    const float amp = (rvOn ? 1.f : 0.15f) * (0.25f + 0.75f * rvMix);

    // reverb wash
    Path fill, edge; bool first = true;
    for (float px = x0; px <= x1; px += 4.f)
    {
        const float pos = (px - x0) / W2, sec = 0.125f * (std::pow (2.f, pos * L) - 1.f);
        float h = amp * std::pow (jmax (0.f, 1.f - sec / decay), 1.15f) * 185.f;
        h *= 1.f + 0.05f * std::sin (px * 0.045f + phase) + 0.03f * std::sin (px * 0.11f - phase * 1.7f);
        h += 4.f * pulse * (1.f - pos);
        const float y = baseY - jmax (1.5f, h);
        if (first) { fill.startNewSubPath (px, baseY); fill.lineTo (px, y); edge.startNewSubPath (px, y); first = false; }
        else { fill.lineTo (px, y); edge.lineTo (px, y); }
    }
    fill.lineTo (x1, baseY); fill.closeSubPath();
    g.setGradientFill (ColourGradient (t.accRev.withAlpha (t.light ? 0.42f : 0.50f), 0.f, baseY - 140.f, t.accRev.withAlpha (0.02f), 0.f, baseY + 12.f, false));
    g.fillPath (fill);
    g.setColour (t.accRev.withAlpha (0.55f)); g.strokePath (edge, PathStrokeType (1.4f, PathStrokeType::curved));
    g.setColour (t.accRev.withAlpha (0.18f)); g.strokePath (edge, PathStrokeType (6.f, PathStrokeType::curved));

    // echo taps
    const bool sync = rawParam ("sync") > 0.5f;
    const float dt = jmax (0.01f, sync ? rp::divBeats()[(size_t) jlimit (0, 13, (int) std::lround (rawParam ("dlTime") * 13.f))] * 60.f / 120.f
                                      : rp::freeMs (rawParam ("dlTime")) * 0.001f);
    const float fbn = jlimit (0.f, 0.95f, fb);
    for (int i = 0; i < 12; ++i)
    {
        const float sec = dt * (float) i; if (sec > 8.f) break;
        float a = i == 0 ? 1.f : (0.30f + 0.70f * dlMix) * 0.78f * std::pow (fbn, (float) (i - 1)) * (dlOn ? 1.f : 0.15f);
        if (a < 0.03f) break;
        if (i == 0) a = 0.80f + 0.20f * pulse;
        const float x = xOf (sec), h = jmax (10.f, 168.f * a), w = jmax (5.f, 14.f - (float) i);
        const Colour c = t.accDelay.interpolatedWith (t.accRev, jmin (1.f, (float) i / 8.f));
        const float al = (i == 0 ? 0.95f : 0.9f - 0.04f * (float) i);
        for (int k = 3; k >= 1; --k) { g.setColour (c.withAlpha (0.05f * al * (float) (4 - k))); g.fillRoundedRectangle (x - w * 0.5f - 3.f * (float) k, baseY - 8.f - h - 3.f * (float) k, w + 6.f * (float) k, h + 6.f * (float) k, (w + 6.f * (float) k) * 0.5f); }
        g.setGradientFill (ColourGradient (c.brighter (t.light ? 0.f : 0.25f).withAlpha (al), x, baseY - 8.f - h, c.withAlpha (al * 0.7f), x, baseY - 8.f, false));
        g.fillRoundedRectangle (x - w * 0.5f, baseY - 8.f - h, w, h, w * 0.5f);
    }
}
