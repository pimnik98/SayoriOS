#pragma once

class stream {
	public:
		stream() {};

		virtual void open() = 0;
		virtual int  read(void* buffer, int count) = 0;
		virtual int  write(void* buffer, int count) = 0;
		virtual void close() = 0;
		
		~stream() {}
};
