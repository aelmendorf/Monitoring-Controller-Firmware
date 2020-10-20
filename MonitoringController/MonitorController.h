#pragma once

#include <P1AM.h>
#include <Ethernet.h>
#include <Mudbus.h>

#define DEBUG				1

#define Vref				5
#define Resistor			250
#define AVG					30
#define Step				204.6f
#define AnalogMax			1023

#define AnalogInputPins		8
#define DigitalInputPins	16
#define DigitalOutputPins	10
#define SoftwareOuputPins	7
#define HardwareOutputPins	3

#define DigitalInputSlot	1
#define DigitalOutputSlot	3
#define AnalogInputSlot		2

#define SystemOkayIndex		7
#define SystemWarningIndex	8
#define SystemAlarmIndex	9

#define KeySwitchIndex		21

#define HardwareMaintIndex	38
#define CoilComIndex		39
#define SoftMaintModeIndex	40
#define WarningIndex		41
#define AlarmIndex			42
#define InputRegIndex		16 //AnalogInputPins

#define LoopTime			50
#define PrintTime			5000 
#define ResetTime			604800000



enum ControllerState { OKAY, WARNING, ALARM, MAINTENCE };

class MonitorController
{
public:
	Mudbus modbus;
	void SetupServer();
	void ReadAnalog();
	void ReadDigital();
	void UpdateModbus();
	void CheckCom();
	void CheckState();
	void SetLights();
	
	void DisplaySystemOkay();
	void DisplaySystemWarning();
	void DisplaySystemAlarm();
	void DisplayMaintenance();

	void Print();
	void BroadcastWeb();
	void Init();
	void Run();
private:

	bool hardwareMaintMode = false;
	bool softwareMaintMode = false;
	bool warning = false;
	bool alarm = false;

	ControllerState State;

	long lastPrint, lastLoop;
	float AnalogValues[AnalogInputPins] = { 0.000f,0.000f,0.000f,0.000f,0.000f,0.000f,0.000f,0.000f };
	int OutputDefaults[DigitalOutputPins] = { LOW,LOW,LOW,LOW,LOW,LOW,HIGH,LOW,HIGH,HIGH };
	int OutputValues[DigitalOutputPins] = { 0 };
	int ModbusOutputValues[DigitalOutputPins] = { 0 };
	bool DigitalInputs[DigitalInputPins] = { 0 };
	EthernetServer* server=new EthernetServer(80);
	EthernetClient client;
};

