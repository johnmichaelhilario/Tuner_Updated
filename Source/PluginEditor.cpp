/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "jucetice_MeterComponent.h"
//==============================================================================




Pitchdetect_autocorrelateAudioProcessorEditor::Pitchdetect_autocorrelateAudioProcessorEditor (Pitchdetect_autocorrelateAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

   

    createTitle(&title, "TUNER");
    setLookAndFeel(&aLAF);
    addAndMakeVisible (noteNameLabel);
    noteNameLabel.setLookAndFeel(&aLAF);
    noteNameLabel.setText("--", juce::dontSendNotification);
    noteNameLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
    noteNameLabel.setJustificationType(juce::Justification::centred);
    noteNameLabel.setFont(juce::Font(65.0f, juce::Font::bold));
    noteNameLabel.setBounds(235, 60, 75, 75);
    

    flatMeter.reset(new MeterComponent(MeterComponent::MeterHorizontal, 20, 2, Colours::orange, Colours::orangered, Colours::darkorange, Colour(0xFF444444)));
    addAndMakeVisible(flatMeter.get());
    flatMeter->setName("flatMeter");
    flatMeter->setBounds(90, 50, 175, 20);


    sharpMeter.reset(new MeterComponent(MeterComponent::MeterHorizontal, 20, 2, Colours::orange, Colours::orangered, Colours::darkorange, Colour(0xFF444444)));
    addAndMakeVisible(sharpMeter.get());
    sharpMeter->setName("sharpMeter");
    sharpMeter->setBounds(280, 50, 175, 20);
    sharpMeter->setTransform(AffineTransform::rotation((float)3.14f,
                        sharpMeter->getPosition().getX() + (sharpMeter->getWidth() * 0.5f),
                        sharpMeter->getPosition().getY() + (sharpMeter->getHeight() * 0.5f)));

 
    addAndMakeVisible(power);
    power.setBounds(10, 40, 50, 50);

    power.setLookAndFeel(&pbLAF);
    power.onClick = [this] {
       //Do something with the ui
       
        if(power.getToggleState()){
            startTimer (50);
            createTitle(&title, "Tuner");

        }else {
            stopTimer();
            updateWidgetValues("--", 0.0f);
            noteNameLabel.setText("--",juce::dontSendNotification);
            createTitle(&title, "Tuner");
        }
     
    };
    
    addAndMakeVisible(&title);
    setSize (500, 150);
    
}


void Pitchdetect_autocorrelateAudioProcessorEditor::createTitle(juce::Label* label, juce::String title) {
    label->setText(title, juce::NotificationType::dontSendNotification);
    label->setColour(juce::Label::outlineColourId, juce::Colours::white);
    label->setJustificationType(juce::Justification::topLeft);
    label->setBounds(16, 16, 100, 25);
    label->setFont(25.0f);
    auto powerIsOn = power.getToggleStateValue();
    if (power.getToggleState()){
        label->setColour(juce::Label::textColourId, UI_Color1);
        return;
    }
}




Pitchdetect_autocorrelateAudioProcessorEditor::~Pitchdetect_autocorrelateAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void Pitchdetect_autocorrelateAudioProcessorEditor::paint (Graphics& g)
{
    
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    //g.setColour(juce::Colours::orange);
    //g.setFont(25.0f);
    //g.drawFittedText("TUNER", 10, 10, getWidth(), 30, juce::Justification::topLeft, 1);


    const int flat_png_size = 591;
    Image flat = ImageCache::getFromMemory(BinaryData::flat_png, flat_png_size);
    g.drawImageAt(flat, 63, 50);

    const int sharp_png_size = 535;
    Image sharp = ImageCache::getFromMemory(BinaryData::sharp_png, sharp_png_size);
    g.drawImageAt(sharp, 455, 50);


    int arrleft_png_size = 486;
    int arrright_png_size = 941;

    Image arrleft = ImageCache::getFromMemory(BinaryData::arrleft_png, arrleft_png_size);
    Image arrright = ImageCache::getFromMemory(BinaryData::arrright_png, arrright_png_size);

    if (arrowColourFlags == GREEN) {
        arrleft_png_size = 415;
        arrright_png_size = 875;

        arrleft = ImageCache::getFromMemory(BinaryData::arrleft_green_png, arrleft_png_size);

        arrright = ImageCache::getFromMemory(BinaryData::arrright_green_png, arrright_png_size);

    }
    if (arrowColourFlags == ORANGE) {
        arrleft_png_size = 463;
        arrright_png_size = 956;

        arrleft = ImageCache::getFromMemory(BinaryData::arrleft_orange_png, arrleft_png_size);

        arrright = ImageCache::getFromMemory(BinaryData::arrright_orange_png, arrright_png_size);
    }

    g.drawImageAt(arrright, 355, 83);
    g.drawImageAt(arrleft, 170, 87);

}

void Pitchdetect_autocorrelateAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    
}

double noteFromPitch(float frequency) {
    double noteNum = 12 * (log(frequency / 440) / log(2));
    noteNum = round(noteNum) + 69;
    return noteNum;
}

float frequencyFromNoteNumber(float note) {
    return 440 * pow(2, (note - 69) / 12);
}

double centsOffFromPitch(float frequency, float note) {
    float frqFromNote = frequencyFromNoteNumber(note);
    float logOfPitch = log(frequency / frqFromNote);
    return floor(1200 * logOfPitch) / log(2);
}

void Pitchdetect_autocorrelateAudioProcessorEditor::updateWidgetValues(String noteName, float pitchTune) {
    const String noteDefault = "--";
    noteNameLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
    noteNameLabel.setText(noteDefault,juce::dontSendNotification);
    sharpMeter -> setValue(0.0f);
    flatMeter -> setValue(0.0f);
    arrowColourFlags = ORANGE;
   
    if (pitchTune == 0.0f && noteName == noteDefault) {
        arrowColourFlags = GREY;
        repaint();
        return;
    }
    //TODO change colours of text to green
    if (pitchTune == 0.0f && noteName != noteDefault) {
        
        noteNameLabel.setText(noteName, juce::dontSendNotification);
        noteNameLabel.setColour(juce::Label::textColourId, juce::Colours::green);
        arrowColourFlags = GREEN;
        repaint();
        return;

    }

    if(pitchTune < 0.0f){
        flatMeter -> setValue(abs(pitchTune)/100);
    }

    if(pitchTune > 0.0f){
        sharpMeter -> setValue(pitchTune/ 100);
    }
    
    noteNameLabel.setText(noteName,juce::dontSendNotification);
    repaint();
}
void Pitchdetect_autocorrelateAudioProcessorEditor::timerCallback()
{
        Pitchdetect_autocorrelateAudioProcessor& ourProcessor = getProcessor();
        double key = ourProcessor.pitch / 2;

        if (key <= 0.0f) {
            updateWidgetValues("--",0.0f);
            return;
        }
        
        
        
        std::array<String, 12> not = { "C","C#","D","Eb","E","F","F#","G","G#","A","Bb","B"};

        int currentKey = noteFromPitch(key);
        int Note = frequencyFromNoteNumber(currentKey);
        float pitchTune = centsOffFromPitch(key, currentKey);
        String noteName = (String)not[currentKey % 12];
        
        updateWidgetValues(noteName, pitchTune);
}
