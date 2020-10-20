/*
 Name:		MonitoringController.ino
 Created:	10/18/2020 2:32:00 PM
 Author:	aelmendo
*/
#include "MonitorController.h"

// the setup function runs once when you press reset or power the board
MonitorController controller;

void setup() {
	controller.Init();
}

// the loop function runs over and over again until power down or reset
void loop() {
	controller.Run();
}
