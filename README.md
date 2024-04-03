ChArrFifo.hpp C++ for Arduino or ESP, etc:
			Lets you save C strings to a fifo and retrieve them again in true fifo manner (First in - first out)
      Configurable in constructor as to how many strings are stored in each entry
			Stores strings on heap and retains ownership of all heap stored items to free them again as
				strings are popped from fifo. Also frees all heap items on destruct or clear.
			Use errCause to get a C string describing last error cause
			This is designed to be used with Arduino-like coding, i.e. Arduino, ESP, etc.
			It has a rather small footprint
			My intended use is to store topic and payload of MQTT messages not sent because of MQTT or WiFi
				unavailable, and send them when MQTT host is available
      Usage code exerpt in file head
