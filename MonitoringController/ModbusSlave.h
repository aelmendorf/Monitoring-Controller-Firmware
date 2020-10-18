#pragma once
#include <Ethernet.h>
#include <ArduinoRS485.h>
#include <ArduinoModbus.h>

enum RegisterType {
	Coil,
	HoldingRegister,
	DiscreteInput,
	InputRegister
};

union Data {
	bool bitData;
	uint16_t wordData;
};

struct RegisterData {
	RegisterType type;
	int address;
	bool isFloat;
	Data data;
};

struct ModbusSettings {
	int discreteInputCount, holdingRegCount, inputRegCount, coilCount;
	int slaveId;
	ModbusSettings(int numCoils, int numDiscreteInput, int numHoldingReg, int numInputReg, int slaveId);
};

class ModbusSlave
{
public:
	ModbusSlave();
	ModbusSlave(const ModbusSettings &settings);
	void SetClient(EthernetClient& client);
	bool Initialize();
	bool Start();
	void Poll();
	bool WriteRegister(const RegisterData &input);
	RegisterData ReadRegister(int addr);

private:
	bool ConfigureRegisters();
private:
	ModbusTCPServer modbusServer;
	EthernetClient* client;
	ModbusSettings settings;
};

