#pragma once

struct Module {
	const char* moduleName;
	const char* configuration;
	int slot;
	bool status;
};

class Controller
{
public:
	Controller();
	bool SetupModule(Module module);
	int* CheckModules();


private:
	Module* modules;

};

