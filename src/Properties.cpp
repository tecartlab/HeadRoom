#include "Properties.h"


//--------------------------------------------------------------
void Properties::load(string propertyFileName){
    
    filepath = propertyFileName;
	//-----------
	//the string is printed at the top of the app
	//to give the user some feedback
    ofLog(OF_LOG_NOTICE, "loading '" + filepath + "'....");

	//we load our settings file
	//initially we do this from the data/ folder
	//but when we save we are saving to the documents folder which is the only place we have write access to
	//thats why we check if the file exists in the documents folder. 
	
	if( XML.loadFile(filepath) ){
        ofLog(OF_LOG_NOTICE, "'" + filepath + "' loaded from data folder!");
	}else{
        ofLog(OF_LOG_ERROR, "unable to load '"+ filepath +"' check data/ folder");
	}
	
	//read the colors from XML
	//if the settings file doesn't exist we assigns default values (170, 190, 240)
	red		= XML.getValue("BACKGROUND:COLOR:RED", 170);
	green	= XML.getValue("BACKGROUND:COLOR:GREEN", 190);
	blue	= XML.getValue("BACKGROUND:COLOR:BLUE", 240);

	/*
		"BACKGROUND:COLOR:RED" referes to a structure like this:

		<BACKGROUND>
			<COLOR>
				<RED>101.103516</RED>
			</COLOR>
		</BACKGROUND>
	*/

	//we initalize some of our variables
	lastTagNumber	= 0;
	pointCount		= 0;
	lineCount		= 0;

}


//--------------------------------------------------------------
void Properties::save(){
	//update the colors to the XML structure when the mouse is released
	XML.setValue("BACKGROUND:COLOR:RED", red);
	XML.setValue("BACKGROUND:COLOR:GREEN", green);
	XML.setValue("BACKGROUND:COLOR:BLUE", blue);

	XML.saveFile( filepath );
    ofLog(OF_LOG_NOTICE, "'"+ filepath +"' saved to data folder");
}
