/*
  Copyright (C) 2016 Rory Walsh

  Cabbage is free software; you can redistribute it
  and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  Cabbage is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU General Public
  License along with Csound; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
  02111-1307 USA
*/

#ifndef CABBAGECONTENT_H_INCLUDED
#define CABBAGECONTENT_H_INCLUDED


#include "../JuceLibraryCode/JuceHeader.h"
#include "../CodeEditor/CabbageEditorContainer.h"
#include "../BinaryData/CabbageBinaryData.h"
#include "../GUIEditor/CabbagePropertiesPanel.h"
#include "../CabbageIds.h"
#include "CabbageToolbarFactory.h"
#include "../Audio/Graph/AudioGraph.h"
#include "../Settings/CabbageSettings.h"
#include "CabbagePluginComponent.h"
#include "CabbageGraphComponent.h"

class CabbageDocumentWindow;

class CabbageContentComponent
    : public Component,
      public Button::Listener,
      public ChangeListener,
      public Timer,
      public ComboBox::Listener
{
public:

    //==============================================================================
    CabbageContentComponent(CabbageDocumentWindow* owner, CabbageSettings* settings);
    ~CabbageContentComponent();
    //==============================================================================
    void changeListenerCallback(ChangeBroadcaster* source);
    void buttonClicked(Button* button);
    void comboBoxChanged (ComboBox *comboBoxThatHasChanged);
    void updateCodeInEditor(CabbagePluginEditor* pluginEditor, bool replaceExistingLine);
    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    void resizeAllEditorAndConsoles(int height);
    void createEditorForAudioGraphNode();
    void createAudioGraph();
    void addFileTabButton(File file);
	void arrangeFileTabButtons();
	void removeEditor();
    Image createBackground();
    //==============================================================================
    void showAudioSettings();
    void createNewProject();
    void launchSSHFileBrowser(String mode);
    void setEditMode(bool enable);
    void openFile(String filename="");
    bool closeAllDocuments (bool askUserToSave);
	void closeDocument();
    bool closeAllMainWindows();
    void showSettingsDialog();
    void saveDocument(bool saveAs=false, bool recompile=true);
    void runCsoundCode();
    void stopCsoundCode();
    void showGenericWidgetWindow();
    void hideGenericWidgetWindow(bool freeContent=false);
    void createGenericCsoundPluginWrapper();
    void initSettings();
    void updateEditorColourScheme();
    void addInstrumentsAndRegionsToCombobox();
    void setLookAndFeelColours();
	
	//==============================================================================
    String getSearchString();
    void setSearchString (const String& s);
    bool isCaseSensitiveSearch();
    void setCaseSensitiveSearch (bool b);
	void showFindPanel(bool withReplace);
	void hideFindPanel();
	void findNext(bool forward);
    //==============================================================================
    CabbagePluginEditor* getCabbagePluginEditor();
    CabbagePluginProcessor* getCabbagePluginProcessor();
    CabbageOutputConsole* getCurrentOutputConsole();
    CabbageCodeEditorComponent* getCurrentCodeEditor();
    //==============================================================================
    String getAudioDeviceSettings();
    void timerCallback();
	int getStatusbarYPos();
    ScopedPointer<CabbagePropertiesPanel> propertyPanel;
    OwnedArray<TextButton> fileTabs;
    Array<File> openFiles;
    OwnedArray<CabbageEditorContainer> editorAndConsole;
    ScopedPointer<CabbageIDELookAndFeel> lookAndFeel;
    CabbageEditorContainer* getCurrentEditorContainer();
    Toolbar toolbar;

    bool setCurrentCsdFile(File file);
	const File getCurrentCsdFile(){		return currentCsdFile;	}

private:
	bool fileNeedsSaving = false;
	String searchString = String::empty;
	bool isCaseSensitive = false;
	File tempFile;	
    CabbageDocumentWindow* owner;
    //ScopedPointer<CabbageIDELookAndFeel> lookAndFeel;
    CabbageToolbarFactory factory;
    Image bgImage;
    File currentCsdFile;
    const int statusBarHeight = 25;
    CabbageSettings* cabbageSettings;
    int currentFileIndex = 0;
    int numberOfFiles = 0;
    ScopedPointer<AudioGraph> audioGraph;
	ScopedPointer<CabbageGraphComponent> graphComponent;
    bool isGUIEnabled = false;
    String consoleMessages;
    const int toolbarThickness = 35;
	class FindPanel;
	ScopedPointer<FindPanel> findPanel;
	TooltipWindow tooltipWindow;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CabbageContentComponent)
};

//==============================================================================
class CabbageContentComponent::FindPanel  : public Component,
                                               private TextEditor::Listener,
                                               private ButtonListener
{
public:
    FindPanel(String searchString, bool isCaseSensitive, bool withReplace)
        : caseButton ("Case-sensitive"),
          findPrev ("<"),
          findNext (">"),
		  replace("Replace"),
		  replaceAll("Replace all"),
		  showReplaceControls(withReplace)
    {
		//find components...
		setSize (260, withReplace == false ? 90 : 180);
        findEditor.setColour (CaretComponent::caretColourId, Colours::black);

        addAndMakeVisible (findEditor);
        findLabel.setText ("Find:", dontSendNotification);
        findLabel.setColour (Label::textColourId, Colours::white);
        findLabel.attachToComponent (&findEditor, false);

        addAndMakeVisible (caseButton);
        caseButton.setColour (ToggleButton::textColourId, Colours::white);
        caseButton.setToggleState (isCaseSensitive, dontSendNotification);
        caseButton.addListener (this);

        findPrev.setConnectedEdges (Button::ConnectedOnRight);
        findNext.setConnectedEdges (Button::ConnectedOnLeft);
        addAndMakeVisible (findPrev);
        addAndMakeVisible (findNext);
		findNext.addListener(this);
		findPrev.addListener(this);

        setWantsKeyboardFocus (false);
        setFocusContainer (true);
        findPrev.setWantsKeyboardFocus (false);
        findNext.setWantsKeyboardFocus (false);

        findEditor.setText (searchString);
        findEditor.addListener (this);
		
		//replace components....
        replaceEditor.setColour (CaretComponent::caretColourId, Colours::black);

        addAndMakeVisible (replaceEditor);
        replaceLabel.setText ("Replace:", dontSendNotification);
        replaceLabel.setColour (Label::textColourId, Colours::white);
        replaceLabel.attachToComponent (&replaceEditor, false);

        replace.setConnectedEdges (Button::ConnectedOnRight);
        replaceAll.setConnectedEdges (Button::ConnectedOnLeft);
        addAndMakeVisible (replace);
        addAndMakeVisible (replaceAll);
		replace.addListener(this);
		replaceAll.addListener(this);
		
        setWantsKeyboardFocus (false);
        setFocusContainer (true);
        replace.setWantsKeyboardFocus (false);
        replaceAll.setWantsKeyboardFocus (false);

		
        replaceEditor.setText (replaceString);
        replaceEditor.addListener (this);
		
		
    }

    void paint (Graphics& g) override
    {
        Path outline;
        outline.addRoundedRectangle (1.0f, 1.0f, getWidth() - 2.0f, getHeight() - 2.0f, 8.0f);

        g.setColour (Colours::black.withAlpha (0.6f));
        g.fillPath (outline);
        g.setColour (Colours::white.withAlpha (0.8f));
        g.strokePath (outline, PathStrokeType (1.0f));
    }

    void resized() override
    {
        int y = 30;
        findEditor.setBounds (10, y, getWidth() - 20, 24);
        y += 30;
        caseButton.setBounds (10, y, getWidth() / 2 - 10, 22);
        findNext.setBounds (getWidth() - 40, y, 30, 22);
        findPrev.setBounds (getWidth() - 70, y, 30, 22);

		y += 50;
        replaceEditor.setBounds (10, y, getWidth() - 20, 24);
        y += 30;
        replace.setBounds (10, y, (getWidth()/2)-5, 22);
        replaceAll.setBounds (getWidth()/2+5, y, getWidth()/2-15, 22);		
		
    }

    void buttonClicked (Button* button) override
    {
		if(button->getName() == caseButton.getName())
			getOwner()->setCaseSensitiveSearch (caseButton.getToggleState());
		else if(button->getName() == findNext.getName())
			getOwner()->findNext(true);	
		else if(button->getName() == findPrev.getName())
			getOwner()->findNext(false);		
			
    }

    void textEditorTextChanged (TextEditor&) override
    {
        getOwner()->setSearchString (findEditor.getText());

        if (CabbageContentComponent* contentComp = getOwner())
		{
            contentComp->findNext(true);			
		}
    }

    void textEditorFocusLost (TextEditor&) override {}

    void textEditorReturnKeyPressed (TextEditor&) override
    {
        getOwner()->setSearchString (findEditor.getText());

        if (CabbageContentComponent* contentComp = getOwner())
		{
            contentComp->findNext(true);			
		}
    }

    void textEditorEscapeKeyPressed (TextEditor&) override
    {
        if (CabbageContentComponent* contentComp = getOwner())
            contentComp->hideFindPanel();
    }

    CabbageContentComponent* getOwner() const
    {
        return findParentComponentOfClass <CabbageContentComponent>();
    }

    TextEditor findEditor, replaceEditor;
    Label findLabel, replaceLabel;
	String replaceString;
    ToggleButton caseButton;
	bool showReplaceControls = false;
    TextButton findPrev, findNext, replace, replaceAll;
};


#endif  // CABBAGECONTENT_H_INCLUDED
