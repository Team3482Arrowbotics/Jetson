#include <cstdio>
#include <iostream>
#include <ntcore.h>
#include "networktables/NetworkTable.h"

using namespace std;

//NetworkTable *Vision;

int main(){
    NetworkTable *Vision;
    Vision->SetClientMode();
    NetworkTable::SetIPAddress("roboRIO-3482-frc.local");

    auto table = Vision->GetTable("camera1");
    while(1){
	table->PutNumber("Test", 12);
    }

    cout << "Fuck you Jared." << endl;

    return 0;
}