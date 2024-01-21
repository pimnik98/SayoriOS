#pragma once

#include "vector.hpp"
#include "stdio.hpp"
#include "stdint.hpp"
#include "log.hpp"

namespace audio {
	typedef struct MachinistDevice {
		char* name;

		uint8_t max_channels;
		bool fixed_channels;

		bool is_opened;

		bool (*available)();
		void (*open)();
		void (*close)();
		void (*write)(char* data, size_t length);
		void (*read)(char* buffer, size_t length);

		void (*set_nchannels)(uint8_t chans);

		// 0 - 100%
		void (*set_volume)(uint8_t left, uint8_t right);
		void (*set_rate)(uint32_t rate);

		friend internals::TTY& operator << (internals::TTY& out, const MachinistDevice& dev) {
			return out << "Audio<" << dev.name << "> " << (dev.available()?"online":"offline");
		}

	} MachinistDevice;

	class MachinistServer {
		public:
			MachinistServer();

			MachinistDevice get_device(int index);
			MachinistDevice get_device(size_t index);
			MachinistDevice get_device(char* name);

			void open(MachinistDevice& dev);
			void write(MachinistDevice& dev, char* data, size_t length);
			void read(MachinistDevice& dev, char* buffer, size_t length);
			void close(MachinistDevice& dev);

			~MachinistServer();
		
		private:
			std::vector<MachinistDevice> devices;
	};
	
	class MachinistClient {
		public:
			MachinistClient();

			void select_device();
			void select_device(int index);
			
			void open();
			void write(void* data, size_t length);
			void read(void* buffer, size_t length);
			void close();

			void set_nchannels(uint8_t chans);
			void set_rate(uint32_t rate);

			// 0% - 100%
			void set_volume(uint8_t left, uint8_t right);

			~MachinistClient();

		private:
			MachinistServer* connection = nullptr;
			MachinistDevice device;
	};

	MachinistServer* machinist_server_get();
	void machinist_server_init();
}
