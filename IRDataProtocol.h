
// IRDataProtocol.h

// A library for sending and receiving infrared data using Arduino
// Based on IRLib2 by Chris Young https://github.com/cyborg5/IRLib2

#ifndef IRDataProtocol_h
#define IRDataProtocol_h

#include <Arduino.h>
#include <IRLibSendBase.h>
#include <IRLibDecodeBase.h>
#include <IRLib_P01_NEC.h>
#include <IRLib_P02_Sony.h>
#include <IRLib_P03_RC5.h>
#include <IRLib_P04_RC6.h>
#include <IRLib_P05_Panasonic_Old.h>
#include <IRLib_P06_JVC.h>
#include <IRLib_P07_NECx.h>
#include <IRLib_P08_Samsung36.h>
#include <IRLib_P09_GICable.h>
#include <IRLib_P10_DirecTV.h>
#include <IRLib_P11_RCMM.h>
#include <IRLibCombo.h>

// Define the pin number for the IR receiver
#define RECV_PIN 11

// Define the pin number for the IR transmitter
#define SEND_PIN 3

// Define the protocol number for the IR data protocol
#define DATA_PROTOCOL 99

// Define the maximum length of a data packet in bits
#define MAX_DATA_LENGTH 64

// Define the header mark and space duration in microseconds
#define HEADER_MARK 9000
#define HEADER_SPACE 4500

// Define the one mark and space duration in microseconds
#define ONE_MARK 560
#define ONE_SPACE 1690

// Define the zero mark and space duration in microseconds
#define ZERO_MARK 560
#define ZERO_SPACE 560

// Define the end mark duration in microseconds
#define END_MARK 560

// Define the minimum gap between packets in microseconds
#define MIN_GAP 25000

// Define the tolerance for timing variations in percentage
#define TOLERANCE 25

// Define the control sequence prefix in binary
#define CONTROL_PREFIX 0b11111111

// Define the control sequence length in bits
#define CONTROL_LENGTH 8

// Define the control sequence types in binary
#define CONTROL_START 0b00000001 // indicates the start of a data transmission
#define CONTROL_END 0b00000010 // indicates the end of a data transmission
#define CONTROL_ACK 0b00000011 // indicates an acknowledgement of a received packet
#define CONTROL_NACK 0b00000100 // indicates a negative acknowledgement of a received packet

// A class for sending infrared data packets using the IR data protocol
class IRsendData: public IRsendBase {
  public:
    // Constructor
    IRsendData(): IRsendBase() {}

    // Method to send a single bit
    void sendBit(bool bit) {
      mark(bit ? ONE_MARK : ZERO_MARK);
      space(bit ? ONE_SPACE : ZERO_SPACE);
    }

    // Method to send a header
    void sendHeader() {
      mark(HEADER_MARK);
      space(HEADER_SPACE);
    }

    // Method to send an end mark
    void sendEndMark() {
      mark(END_MARK);
      space(0); // turn off IR LED
    }

    // Method to send a control sequence
    void sendControl(uint8_t type) {
      sendHeader();
      uint8_t data = CONTROL_PREFIX | type; // combine prefix and type into one byte
      for (int i = 0; i < CONTROL_LENGTH; i++) { // send each bit from MSB to LSB
        sendBit(data & 0b10000000);
        data <<= 1; // left shift by one bit
      }
      sendEndMark();
    }

    // Method to send a data packet
    void sendData(uint64_t data, uint8_t nbits) {
      if (nbits > MAX_DATA_LENGTH) return; // do nothing if data is too long
      sendHeader();
      for (int i = 0; i < nbits; i++) { // send each bit from MSB to LSB
        sendBit(data & ((uint64_t) 1) << (nbits - 1 - i)); // get the i-th bit from the left
        data <<= 1; // left shift by one bit
      }
      sendEndMark();
    }
};

// A class for receiving infrared data packets using the IR data protocol
class IRrecvData: public IRdecodeBase {
  public:
    // Constructor
    IRrecvData(): IRdecodeBase() {}

    // Method to get a single bit from the raw buffer
    bool getBit() {
      if (offset >= recvGlobal.decodeLength) return false; // return false if offset is out of range
      uint16_t markDuration = recvGlobal.decodeBuffer[offset++]; // get the mark duration and increment offset
      uint16_t spaceDuration = recvGlobal.decodeBuffer[offset++]; // get the space duration and increment offset
      if (MATCH(markDuration, ONE_MARK) && MATCH(spaceDuration, ONE_SPACE)) return true; // return true if mark and space match one bit
      if (MATCH(markDuration, ZERO_MARK) && MATCH(spaceDuration, ZERO_SPACE)) return false; // return false if mark and space match zero bit
      return false; // return false otherwise
    }

    // Method to decode a header from the raw buffer
    bool decodeHeader() {
      if (!MATCH(recvGlobal.decodeBuffer[1], HEADER_MARK)) return false; // return false if first mark does not match header mark
      if (!MATCH(recvGlobal.decodeBuffer[2], HEADER_SPACE)) return false; // return false if first space does not match header space
      offset = 3; // set offset to point to the next mark
      return true; // return true if header is valid
    }

    // Method to decode a control sequence from the raw buffer
    bool decodeControl() {
      uint8_t data = 0; // initialize data to zero
      for (int i = 0; i < CONTROL_LENGTH; i++) { // get each bit from MSB to LSB
        data <<= 1; // left shift by one bit
        data |= getBit(); // set the LSB to the current bit
      }
      if ((data & CONTROL_PREFIX) != CONTROL_PREFIX) return false; // return false if prefix does not match
      value = data & ~CONTROL_PREFIX; // set value to the type of control sequence
      bits = CONTROL_LENGTH; // set bits to the length of control sequence
      protocolNum = DATA_PROTOCOL; // set protocol number to the IR data protocol
      return true; // return true if control sequence is valid
    }

    // Method to decode a data packet from the raw buffer
    bool decodeData() {
      uint64_t data = 0; // initialize data to zero
      uint8_t nbits = 0; // initialize number of bits to zero
      while (offset < recvGlobal.decodeLength) { // loop until offset reaches the end of buffer or maximum data length is reached
        data <<= 1; // left shift by one bit
        data |= getBit(); // set the LSB to the current bit
        nbits++; // increment number of bits by one
        if (nbits == MAX_DATA_LENGTH) break; // break the loop if maximum data length is reached
      }
      value = data; // set value to the decoded data
      bits = nbits; // set bits to the number of bits decoded
      protocolNum = DATA_PROTOCOL; // set protocol number to the IR data protocol
      return true; // return true if data packet is valid
    }

    // Method to decode an infrared signal from the raw buffer
    bool decode(void) {
      resetDecoder(); // reset the decoder state
      if (recvGlobal.decodeLength < 6) return false; // return false if raw buffer is too short

      if (!decodeHeader()) return false; // return false if header is invalid
      if (MATCH(recvGlobal.decodeBuffer[offset], ONE_MARK) && MATCH(recvGlobal.decodeBuffer[offset + 1], ONE_SPACE)) { // check if the first bit is one
        return decodeControl(); // return the result of decoding a control sequence
      } else {
        return decodeData(); // return the result of decoding a data packet
      }
    }
};

#endif // IRDataProtocol_h


//Source: Conversation with Bing, 4/26/2023
//(1) How to Send and Receive Data Over IR Signals with an Arduino. https://www.digikey.com/en/maker/blogs/2021/how-to-send-and-receive-data-over-ir-signals-with-an-arduino.
//(2) Sending IR Codes | Using an Infrared Library on Arduino | Adafruit .... https://learn.adafruit.com/using-an-infrared-library/sending-ir-codes.
//(3) IR Communication - SparkFun Learn. https://learn.sparkfun.com/tutorials/ir-communication/all.
