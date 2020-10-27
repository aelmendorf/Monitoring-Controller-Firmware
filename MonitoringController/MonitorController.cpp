#include "MonitorController.h"

void MonitorController::Init() {
	this->lastLoop = 0;
	this->lastPrint = 0;
	this->alarm = false;
	this->warning = false;
	this->hardwareMaintMode = false;
	this->softwareMaintMode = false;
	this->State = OKAY;
	//analogWriteResolution(10);
	this->SetupServer();

	for (int i = 0; i < MB_N_C; i++) {
		this->modbus.C[i] = false;
	}

	for (int i = 0; i < MB_N_R; i++) {
		this->modbus.R[i] = 0;
	}


}

void MonitorController::SetupServer() {
	IPAddress ip(172, 21, 100, 31);
	IPAddress subnet(255, 255, 0, 0);
	IPAddress dns(172, 20, 3, 5);
	IPAddress gateway(172, 21, 100, 1);
	byte macAddress[6] = { 0x60, 0x52, 0xD0, 0x06, 0x70, 0x7E };
	while (!P1.init());
#if DEBUG
	Serial.begin(115200);
	while (!Serial);
#endif
	Ethernet.init(5);   //CS pin for P1AM-ETH
	//Ethernet.begin(macAddress, ip,dns,gateway,subnet); // start the Ethernet connection
	Ethernet.begin(macAddress);
	if (Ethernet.hardwareStatus() == EthernetNoHardware) {
#if DEBUG
		Serial.println("Ethernet shield not detected");
#endif
		while (true) {
			delay(1); 
		}
	}
	if (Ethernet.linkStatus() == LinkOFF) {
#if DEBUG
		Serial.println("Ethernet cable is not connected");
#endif
	}
	/*this->server->begin();*/
#if DEBUG
	Serial.print("Ethernet Connected.  IP: "); Serial.println(Ethernet.localIP());
	this->Print();
#endif

}

void MonitorController::ReadAnalog() {
	for (int i = 0; i < AnalogInputPins; i++) {
		float read = 0;
		for (int x = 0; x < AVG; x++) {
			float inputCounts = P1.readAnalog(AnalogInputSlot, i+1);
			float value = CurrentMax * ((float)inputCounts / Bit13Reg);
			read += value;
		}
		this->AnalogValues[i] = (read / AVG);
		delay(1);

	}
}

void MonitorController::ReadDigital() {
	for (int i = 0; i < DigitalInputPins; i++) {
		this->DigitalInputs[i]=P1.readDiscrete(DiscreteInputSlot, i+1);
	}

	if (this->DigitalInputs[KeySwitchIndex]) {
		this->hardwareMaintMode = true;
	}
	else {
		this->hardwareMaintMode = false;
	}
}

void MonitorController::UpdateModbus() {

	for (int i = 0; i < AnalogInputPins; i++) {
		this->modbus.R[i] = (int)(this->AnalogValues[i] * 1000);
	}

	for (int i = 0; i < DigitalInputPins; i++) {
		this->modbus.C[i] = this->DigitalInputs[i];
	}

	if (!modbus.C[CoilComIndex]) {
		for (int i = 0; i < DigitalOutputPins; i++) {
			this->modbus.R[i + AnalogInputPins] = this->ModbusOutputValues[i];
		}
	}
	this->modbus.R[StateIndex] = this->State;
}

void MonitorController::CheckCom() {
	if (this->modbus.C[CoilComIndex]) {
		this->softwareMaintMode = this->modbus.C[SoftMaintModeIndex];
		this->alarm = this->modbus.C[AlarmIndex];
		this->warning = this->modbus.C[WarningIndex];
		for (int i = 0; i < SoftwareOuputPins; i++) {
			this->ModbusOutputValues[i] = modbus.R[i + InputRegIndex];
		}
		this->modbus.C[CoilComIndex] = false;
	}

}

void MonitorController::CheckState() {
	if (this->softwareMaintMode || this->hardwareMaintMode) {
		if (this->hardwareMaintMode && this->softwareMaintMode) {
			this->softwareMaintMode = false;
			this->modbus.C[HardwareMaintIndex] = true;
			this->modbus.C[SoftMaintModeIndex] = false;
		}
		else if (this->hardwareMaintMode && !this->softwareMaintMode) {
			this->modbus.C[HardwareMaintIndex] = true;
		}
		else if (!this->hardwareMaintMode && this->softwareMaintMode) {
			this->modbus.C[HardwareMaintIndex] = false;
		}
		this->State = MAINTENCE;
	}
	else {
		this->modbus.C[HardwareMaintIndex] = false;
		if (this->alarm || this->warning) {
			if (this->alarm) {
				this->State = ALARM;
			}
			else if (this->warning) {
				this->State = WARNING;
			}
		}
		else {
			this->State = OKAY;
		}
	}
	this->SetLights();
}

void MonitorController::SetLights() {
	switch (this->State) {
		case OKAY: {
			this->DisplaySystemOkay();
			break;
		}
		case WARNING: {
			this->DisplaySystemWarning();
			break;
		}
		case ALARM: {
			this->DisplaySystemAlarm();
			break;
		}
		case MAINTENCE: {
			this->DisplayMaintenance();
			break;
		}
	}
}



void MonitorController::Run() {
	modbus.Run();
	if (millis() >= (LoopTime + this->lastLoop)) {
		this->ReadDigital();
		this->ReadAnalog();
		this->CheckCom();
		this->CheckState();
		this->UpdateModbus();
		//this->BroadcastWeb();
		this->lastLoop += LoopTime;
	}
#if DEBUG
	if (millis() >= (PrintTime + this->lastPrint)) {
		this->Print();
		this->lastPrint += PrintTime;
	}
#endif // DEBUG
}

void MonitorController::DisplaySystemOkay() {
	this->ModbusOutputValues[SystemOkayIndex] = LOW;
	this->ModbusOutputValues[SystemWarningIndex] = HIGH;
	this->ModbusOutputValues[SystemAlarmIndex] = HIGH;
	P1.writeDiscrete(1, 3, SystemOkayIndex);
	P1.writeDiscrete(0, 3, SystemWarningIndex);
	P1.writeDiscrete(0, 3, SystemAlarmIndex);
	//digitalWrite(SystemOkayIndex, LOW);
	//digitalWrite(SystemWarningIndex, HIGH);
	//digitalWrite(SystemAlarmIndex, HIGH);
}

void MonitorController::DisplaySystemWarning() {
	this->ModbusOutputValues[SystemOkayIndex] = HIGH;
	this->ModbusOutputValues[SystemWarningIndex] = LOW;
	this->ModbusOutputValues[SystemAlarmIndex] = HIGH;
	P1.writeDiscrete(0, DigitalOutputSlot, SystemOkayIndex);
	P1.writeDiscrete(1, DigitalOutputSlot, SystemWarningIndex);
	P1.writeDiscrete(0, DigitalOutputSlot, SystemAlarmIndex);
	//digitalWrite(SystemOkayIndex, HIGH);
	//digitalWrite(SystemWarningIndex, LOW);
	//digitalWrite(SystemAlarmIndex, HIGH);
}

void MonitorController::DisplaySystemAlarm() {
	this->ModbusOutputValues[SystemOkayIndex] = HIGH;
	this->ModbusOutputValues[SystemWarningIndex] = HIGH;
	this->ModbusOutputValues[SystemAlarmIndex] = LOW;
	P1.writeDiscrete(0, DigitalOutputSlot, SystemOkayIndex);
	P1.writeDiscrete(0, DigitalOutputSlot, SystemWarningIndex);
	P1.writeDiscrete(1, DigitalOutputSlot, SystemAlarmIndex);
	//digitalWrite(SystemOkayIndex, HIGH);
	//digitalWrite(SystemWarningIndex, HIGH);
	//digitalWrite(SystemAlarmIndex, LOW);
}

void MonitorController::DisplayMaintenance() {
	this->ModbusOutputValues[SystemOkayIndex] = LOW;
	this->ModbusOutputValues[SystemWarningIndex] = LOW;
	this->ModbusOutputValues[SystemAlarmIndex] = HIGH;
	P1.writeDiscrete(1, DigitalOutputSlot, SystemOkayIndex);
	P1.writeDiscrete(1, DigitalOutputSlot, SystemWarningIndex);
	P1.writeDiscrete(0, DigitalOutputSlot, SystemAlarmIndex);
	//digitalWrite(SystemOkayIndex, LOW);
	//digitalWrite(SystemWarningIndex, LOW);
	//digitalWrite(SystemAlarmIndex, HIGH);
}

void MonitorController::Print() {
	Serial.println("Discrete Input:");
	for (int i = 0; i < DigitalInputPins; i++) {
		Serial.print(" P"); Serial.print(i); Serial.print("=");
		Serial.print(this->DigitalInputs[i]); 
		//if (this->DigitalInputs[i]) {
		//	Serial.print(": High");
		//}
		//else {
		//	Serial.print(": Low");
		//}
	}
	Serial.println();
	Serial.println("Analog Input:");
	for (int i = 0; i < AnalogInputPins; i++) {
		Serial.print(" P"); Serial.print(i);
		Serial.print(": "); Serial.print(this->AnalogValues[i]);
	}
	Serial.println();
}

void MonitorController::BroadcastWeb() {
	//this->client = this->server->available();
	//if (client) {
	//	boolean currentLineIsBlank = true;
	//	if (client.available()) {
	//		char c = client.read();
	//		Serial.write(c);
	//		// if you've gotten to the end of the line (received a newline
	//		// character) and the line is blank, the http request has ended,
	//		// so you can send a reply
	//		while(c!='\n' && !currentLineIsBlank){
	//		}
	//		if (c == '\n' && currentLineIsBlank) {
	//			// send a standard http response header
	//			client.println("HTTP/1.1 200 OK");
	//			client.println("Content-Type: text/html");
	//			client.println("Connection: close");  // the connection will be closed after completion of the response
	//			client.println("Refresh: 5");  // refresh the page automatically every 5 sec
	//			client.println();
	//			client.println("<!DOCTYPE HTML>");
	//			client.println("<html>"); //Start our HTML here
	//			client.println("<head>");
	//			client.println("<style>");
	//			client.println("table, th, td {");
	//			client.println("border: 1px solid black;");
	//			client.println("border-collapse: collapse;");
	//			client.println("}");
	//			client.println("</style>");
	//			client.println("</head>");
	//			client.println("<body>");
	//			client.println("<h1>Facility Monitoring Web</h1>");  //Print the title
	//			client.println("<br>");

	//			client.println("<table>");
	//			client.println("<tr>");
	//			client.println("<th>Channel</th>");
	//			client.println("<th>Value</th>");
	//			client.println("</tr>");

	//			for (int i = 0; i < AnalogInputPins; i++) {		
	//				client.println("<tr>");
	//				client.print("<td>");
	//				client.print(this->AnalogValues[i]);	
	//				client.println("</td>");
	//				client.print("<td>");
	//				client.print("A"); client.print(i + 1);
	//				client.println("<td>");
	//				client.println("</tr>");
	//			}

	//			client.println("</table>");
	//			client.println("</body>");
	//			client.println("</html>");  //Our HTML ends here
	//		}
	//		if (c == '\n') {
	//			// you're starting a new line
	//			currentLineIsBlank = true;
	//		}
	//		else if (c != '\r') {
	//			// you've gotten a character on the current line
	//			currentLineIsBlank = false;
	//		}
	//	}
	//}
}