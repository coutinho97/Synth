// MidiSerial.h
#pragma once

#include <windows.h>
#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <vector>


class MidiSerial
{
public:
     using ControlChangeCallback = std::function<void(int channel, int cc_number, int value)>;

     MidiSerial();
     ~MidiSerial();

     bool Open(const std::string& portName = "COM10", int baud = 31250); // ex: "COM4"
     void Close();

     void SetControlChangeCallback(ControlChangeCallback cb) 
     {
          onControlChange = cb;
     }

     void StartReading();
     void StopReading();

private:
     void ReadingThread();

     HANDLE hSerial = INVALID_HANDLE_VALUE;
     std::thread readerThread;
     std::atomic<bool> running{false};

     ControlChangeCallback onControlChange;

     // buffer temporário para mensagens incompletas
     std::vector<uint8_t> buffer;
};