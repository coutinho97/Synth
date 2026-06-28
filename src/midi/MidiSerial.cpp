
// MidiSerialInput.cpp
#include "MidiSerial.h"
#include <iostream>


/* DEBUG */
#include "../Logger/logger.h"

#define DEBUG_ENABLE 		1

#if DEBUG_ENABLE
	#define DEBUG(fmt, ...)   	Logger::Format("[midi] " fmt, ##__VA_ARGS__)
#else
	#define DEBUG(fmt, ...)
#endif


MidiSerial::MidiSerial() = default;

MidiSerial::~MidiSerial() 
{
     Close();
}

bool MidiSerial::Open(const std::string& portName, int baud) 
{
     std::string fullPort = "\\\\.\\" + portName;

     hSerial = CreateFileA(fullPort.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

     if (hSerial == INVALID_HANDLE_VALUE) 
     {
          DEBUG("Erro ao abirir %s: %ld", portName.c_str(), GetLastError());
          return false;
     }

     DCB dcbSerialParams = {0};
     dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

     if (!GetCommState(hSerial, &dcbSerialParams)) 
     {
          CloseHandle(hSerial);
          hSerial = INVALID_HANDLE_VALUE;
          return false;
     }

     dcbSerialParams.BaudRate = baud;
     dcbSerialParams.ByteSize = 8;
     dcbSerialParams.StopBits = ONESTOPBIT;
     dcbSerialParams.Parity   = NOPARITY;

     if (!SetCommState(hSerial, &dcbSerialParams)) 
     {
          CloseHandle(hSerial);
          hSerial = INVALID_HANDLE_VALUE;
          return false;
     }

     COMMTIMEOUTS timeouts = {0};
     timeouts.ReadIntervalTimeout         = 50;
     timeouts.ReadTotalTimeoutConstant    = 50;
     timeouts.ReadTotalTimeoutMultiplier  = 10;
     timeouts.WriteTotalTimeoutConstant   = 50;
     timeouts.WriteTotalTimeoutMultiplier = 10;

     SetCommTimeouts(hSerial, &timeouts);

     DEBUG("Porta MIDI seria aberta: %s, baud: %d", portName.c_str(), baud);

     return true;
}

void MidiSerial::Close() 
{
     StopReading();

     if (hSerial != INVALID_HANDLE_VALUE) 
     {
          CloseHandle(hSerial);
          hSerial = INVALID_HANDLE_VALUE;
     }
}

void MidiSerial::StartReading() 
{
     if (running) return;
     running = true;
     readerThread = std::thread(&MidiSerial::ReadingThread, this);
}

void MidiSerial::StopReading() 
{
     running = false;

     if (readerThread.joinable()) 
     {
          readerThread.join();
     }
}

void MidiSerial::ReadingThread() 
{
     uint8_t byte;
     DWORD bytesRead;

     while (running) 
     {
          if (ReadFile(hSerial, &byte, 1, &bytesRead, nullptr) && bytesRead == 1) 
          {
               buffer.push_back(byte);

               // Tentamos parsear mensagens completas
               while (buffer.size() >= 3) 
               {
                    uint8_t status = buffer[0];

                    // Control Change
                    if ((status & 0xF0) == 0xB0) 
                    {
                         int channel = status & 0x0F;
                         int cc      = buffer[1] & 0x7F;
                         int value   = buffer[2] & 0x7F;

                         if (onControlChange) 
                         {
                              onControlChange(channel, cc, value);
                         }

                         buffer.erase(buffer.begin(), buffer.begin() + 3);
                    }
                    else 
                    {
                         // Ignora bytes que não começam com status CC
                         // (podes adicionar mais tipos no futuro)
                         buffer.erase(buffer.begin());
                    }
               }
          }
          else 
          {
               // pequeno delay para não consumir 100% CPU
               Sleep(1);
          }
     }
}


