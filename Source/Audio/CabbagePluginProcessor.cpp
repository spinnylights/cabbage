/*
  ==============================================================================

    CabbagePluginProcessor.cpp
    Created: 24 Oct 2016 6:38:24pm
    Author:  rory

  ==============================================================================
*/

#include "CabbagePluginProcessor.h"
#include "CabbagePluginEditor.h"


char tmp_string[4096] = {0};
char channelMessage[4096] = {0};


CabbagePluginProcessor::CabbagePluginProcessor(File inputFile)
:CsoundPluginProcessor(inputFile), 
csdFile(inputFile),
cabbageWidgets("CabbageWidgetData")
{	
	
	if(inputFile.existsAsFile())
	{
		parseCsdFile();
		createParameters();
	}

}

CabbagePluginProcessor::~CabbagePluginProcessor()
{

}

//==============================================================================
void CabbagePluginProcessor::parseCsdFile()
{
	StringArray linesFromCsd;
	linesFromCsd.addLines(csdFile.loadFileAsString());
	
	for( int lineNumber = 0; lineNumber < linesFromCsd.size() ; lineNumber++ )
	{
		if(linesFromCsd[lineNumber].equalsIgnoreCase("</Cabbage>"))
			lineNumber = linesFromCsd.size()+1;
		
		const String widgetTreeIdentifier = "WidgetFromLine_"+String(lineNumber);
		ValueTree temp(widgetTreeIdentifier);
		CabbageWidget::setWidgetState(temp, linesFromCsd[lineNumber], lineNumber);
		
		if(CabbageWidget::getProperty(temp, CabbageIdentifierIds::basetype).toString()=="interactive" ||
			CabbageWidget::getProperty(temp, CabbageIdentifierIds::basetype).toString()=="layout" )
		{
			cabbageWidgets.addChild(temp, -1, 0);
		}
		
	}
	
	CabbageUtilities::writeValueTreeToFile(cabbageWidgets, "/home/rory/Desktop/test.xml");

}

//==============================================================================
void CabbagePluginProcessor::createParameters()
{
	for(int i = 0; i < cabbageWidgets.getNumChildren(); i++)
	{
		const String typeOfWidget = CabbageWidget::getStringProp(cabbageWidgets.getChild(i), CabbageIdentifierIds::type);
		CabbageControlWidgetStrings controlWidgetTypes;
		if(controlWidgetTypes.contains(typeOfWidget))
		{
			const String name = CabbageWidget::getStringProp(cabbageWidgets.getChild(i), CabbageIdentifierIds::name);
			const String channel = CabbageWidget::getStringProp(cabbageWidgets.getChild(i), CabbageIdentifierIds::channel);
			const int value = CabbageWidget::getNumProp(cabbageWidgets.getChild(i), CabbageIdentifierIds::value);
			
			if(typeOfWidget==CabbageIdentifierIds::checkbox)
				addParameter(new CabbageAudioParameter(*getCsound(), channel, name, 0, 1, value));	
		}
	}	
}

//==============================================================================
bool CabbagePluginProcessor::hasEditor() const	
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* CabbagePluginProcessor::createEditor()
{
    return new CabbagePluginEditor(*this);
}

//==============================================================================
void CabbagePluginProcessor::receiveChannelDataFromCsound()
{

	for( int i = 0; i < cabbageWidgets.getNumChildren(); i++)
	{
		channelTmp = CabbageWidget::getStringProp(cabbageWidgets.getChild(i), CabbageIdentifierIds::channel);
		valueTmp = CabbageWidget::getNumProp(cabbageWidgets.getChild(i), CabbageIdentifierIds::value);
		identChannelTmp = CabbageWidget::getStringProp(cabbageWidgets.getChild(i), CabbageIdentifierIds::identchannel);
		identChannelMessageTmp = CabbageWidget::getStringProp(cabbageWidgets.getChild(i), CabbageIdentifierIds::identchannelmessage);
		
		if(getCsound()->GetChannel(channelTmp.toUTF8())!=valueTmp)
			CabbageWidget::setNumProp(cabbageWidgets.getChild(i), CabbageIdentifierIds::value, getCsound()->GetChannel(channelTmp.toUTF8()));		
		
		if(identChannelTmp.isNotEmpty())
		{	
			getCsound()->GetStringChannel(identChannelTmp.toUTF8(), tmp_string);
			if(String(tmp_string)!=identChannelMessageTmp)
			{
				CabbageWidget::setCustomWidgetState(cabbageWidgets.getChild(i), " "+String(tmp_string));
			}	
		}
				
	}
}
