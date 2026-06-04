#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"

/*  Headless renderer: builds the editor and paints it to a PNG using JUCE's
    software renderer, so we can eyeball the UI without a DAW or a display.    */
int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    const juce::String outPath = argc > 1 ? juce::String (argv[1]) : "ui.png";
    const int scale = argc > 2 ? juce::String (argv[2]).getIntValue() : 2;

    DecapitoneAudioProcessor proc;
    proc.prepareToPlay (48000.0, 512);

    std::unique_ptr<juce::AudioProcessorEditor> editor (proc.createEditor());
    const int w = editor->getWidth(), h = editor->getHeight();

    // Push some signal so the meter shows a level.
    proc.outputLevel.store (0.5f);

    juce::Image img (juce::Image::ARGB, w * scale, h * scale, true);
    {
        juce::Graphics g (img);
        g.addTransform (juce::AffineTransform::scale ((float) scale));
        editor->paintEntireComponent (g, true);
    }

    juce::File out (juce::File::getCurrentWorkingDirectory().getChildFile (outPath));
    out.deleteFile();
    juce::FileOutputStream fos (out);
    juce::PNGImageFormat png;
    png.writeImageToStream (img, fos);

    juce::Logger::writeToLog ("Wrote " + out.getFullPathName()
                              + "  (" + juce::String (w) + "x" + juce::String (h)
                              + " @" + juce::String (scale) + "x)");
    printf ("Wrote %s (%dx%d @%dx)\n", out.getFullPathName().toRawUTF8(), w, h, scale);
    return 0;
}
