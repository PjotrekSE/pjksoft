#pragma once

/*    No liability accepted for safety, usefullness or anything else
			Software is used on your own risc.
			In public domain according to
							GNU GENERAL PUBLIC LICENSE Version 3


			Lets you save C strings to a fifo and retrieve them again in true fifo manner (First in - first out)
      Configurable in constructor as to how many strings are stored in each entry
			Stores strings on heap and retains ownership of all heap stored items to free them again as
				stringes are popped from fifo. Also frees all heap items on destruct or clear.
			Use errCause to get a C string describing last error cause
			This is designed to be used with Arduino-like coding, i.e. Arduino, ESP, etc.
			It has a rather small footprint
			My intended use is to store topic and payload of MQTT messages not sent because of MQTT or WiFi
				unavailable, and send them again when MQTT host is available

			Author PjotrekSE (pjotrek at solvik dot se)

			Exerpt of working .ino code:

					...
					#include <ChArrFifo.hpp>
					...
					enum { G_lbufSz = 144 };  // Always allocate in 16-byte chunks!
					enum { G_sbufSz = 32 };		//   as so does Arduino IDE compiler
					char G_lbuf[ G_lbufSz ] = {};
					char G_sbuf[ G_sbufSz ] = {};
					...
					ChArrFifo  G_msgFifo(2);  // 2 as I save both topic & payload
					...
					void publishToMqtt(const char* topic, const char* pload ...) {
							...
							if (!mqttClient.connected()){
									const char* pptr[] = {topic, pload};
									G_msgFifo.push(pptr)
							...
					}
					void onMqttConnect(bool sessionPresent) {
							...
							if (!G_msgFifo.empty()) {
									mqttSendSavedPackages;	// Publish saved msgs
							}
					}
					...
					void mqttSendSavedPackages() {
							while (!G_msgFifo.empty()) {
									const size_t bufSzs[] = {G_sbufSz, G_lbufSz};
									char*        bufs[]   = {G_sbuf ,  G_lbuf};
									if (G_msgFifo.pop(bufSzs, bufs)) {
											publishToMqtt(bufs[0], bufs[1]...);
									}
							}
					}
					...
					void setup() {
							...
					}

					void loop() {
							...
					}
*/


#define VERSION    "0.900"   // 240402 Initial version
//                           //          ...to my great astonishment,
//                           //          it worked without flaws directly after a clean compile!
//                           //          (but that took quite a lot of compilations...)

//ToDo:


#include <deque>

class ChArrFifo {
  public:
     ChArrFifo(size_t =1);              // constructor, sets number of strings in each entry
    ~ChArrFifo();                       // destructor

    const size_t size() const;          // get entry count of fifo
		const bool   empty() const;         // are we empty?
		const char*  errCause() const;			// Description for last error
    bool         push(const char**);    // push an entry to end of fifo
    bool         pop(const size_t*, char**);  // pop an entry from top of fifo
		void         clear();               // resets fifo and frees all heap items

 private:
		size_t _chArrDim = 1;               // Dimension of stored charArray(s)
		std::deque<char**> _fifo = {};	    // Fifo to hold buffer pointers
		enum { _errCauseSz = 25 };			    // Length of description buffer
		char _errCause[_errCauseSz] = {};		// Description buffer
};

// constructor of ChArrFifo,
ChArrFifo::ChArrFifo(size_t chArrDim) {
	_chArrDim = chArrDim;
	_fifo.clear();
}

// destructor
ChArrFifo::~ChArrFifo() {
    this->_clear();
}

// Clears our instance
void ChArrFifo::clear() {
		// release all buffers
		if (!_fifo.empty()) {
				for (auto ix=0;ix<_fifo.size();+ix++) {
						char** chArrArrPtr = _fifo[ix];
						for (auto iy=0;iy<_chArrDim;iy++) {
								free(chArrArrPtr[iy]);
						}
						free(chArrArrPtr);
				}
		}
		_fifo.clear();
}

// size, return string count
const size_t ChArrFifo::size() const {
	return _fifo.size();
}

// size, return string count
const bool ChArrFifo::empty() const {
	return _fifo.empty();
}

const char* ChArrFifo::errCause() const {		// Description for last error
		return _errCause;
}

bool ChArrFifo::push(const char** chArrPtrIns) {
		if (ESP.getMaxFreeBlockSize() < 10000) {
			strncpy(_errCause,PSTR("Heap limitation"), _errCauseSz);
			return false;	// Max free heap block
		}
		char** chArrArrPtr = (char**)calloc(_chArrDim, sizeof(char*));
		for (auto ix=0;ix<_chArrDim;ix++) {
				int16_t inslen = strlen(chArrPtrIns[ix]);
				char* bufPtr = (char*)calloc(inslen + 1, sizeof(char));
				if (bufPtr == nullptr) {
					strncpy(_errCause,PSTR("Malloc failure"), _errCauseSz);
					return false;
				}
				strcpy(bufPtr, chArrPtrIns[ix]);
				chArrArrPtr[ix] = bufPtr;

		}

		// Everything OK, let's do it!
		_fifo.push_back(chArrArrPtr);
		return true;
}

bool ChArrFifo::pop(const size_t* retBufSizes, char** retBufs) {  // get first in
		if (_fifo.size() == 0) {
				strncpy(_errCause,PSTR("Fifo empty"), _errCauseSz);
				return false;
		}

		char** chArrArrPtr = _fifo.front();
		for (auto ix=0;ix<_chArrDim;ix++) {
				size_t len = strlen(chArrArrPtr[ix]) + 1;
				if (len > retBufSizes[ix]) {
						strncpy(_errCause,PSTR("Buffer too small"), _errCauseSz);
						return false;
				}
		}

		// Everything OK, let's do it!
		for (auto ix=0;ix<_chArrDim;ix++) {
				strncpy(retBufs[ix], chArrArrPtr[ix], retBufSizes[ix]);
				free(chArrArrPtr[ix]);
		}
		free(chArrArrPtr);
		_fifo.pop_front();

		return true;
}
