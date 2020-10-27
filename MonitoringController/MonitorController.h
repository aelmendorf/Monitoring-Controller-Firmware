#pragma once

#include <P1AM.h>
#include <Ethernet.h>
#include <Mudbus.h>

#define DEBUG				1
#define AVG					10
#define Bit13Reg			8191
#define CurrentMax			20

#define AnalogInputPins		8
#define DigitalInputPins	16
#define DigitalOutputPins	8
#define SoftwareOuputPins	7
#define HardwareOutputPins	3

#define DiscreteInputSlot	1
#define DigitalOutputSlot	3
#define AnalogInputSlot		2

#define SystemOkayIndex		6
#define SystemWarningIndex	7
#define SystemAlarmIndex	8

#define KeySwitchIndex		15

#define HardwareMaintIndex	38
#define CoilComIndex		39
#define SoftMaintModeIndex	40
#define WarningIndex		41
#define AlarmIndex			42
#define InputRegIndex		16 //AnalogInputPins
#define StateIndex			26
#define ModbusOutputIndex	16

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
	int OutputDefaults[DigitalOutputPins] = { 0,0,0,0,0,1,0,0 };
	int OutputValues[DigitalOutputPins] = { 0 };
	int ModbusOutputValues[DigitalOutputPins] = { 0 };
	int DigitalInputs[DigitalInputPins] = { 0 };
	//EthernetServer* server=new EthernetServer(80);
	//EthernetClient client;
};

