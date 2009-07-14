#ifndef PSIACTIONS_H
#define PSIACTIONS_H

enum ActivationType {
	UserAction = 0,    // action is triggered by user
	UserPassiveAction, // action is triggered by user, but should be performed in background
	IncomingStanza,    // action is triggered by incoming stanza
	FromXml            // action is triggered by restoring saved events from XML
};

#endif
