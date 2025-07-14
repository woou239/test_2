#pragma once

#include <cstdint>

#define IOCP_API __declspec(dllexport)

extern "C" {
	IOCP_API bool init(uint32_t num_of_boards, char channel_map_path[], char save_path[], uint32_t save_file_time, uint32_t sample_rate);
	IOCP_API void uninitialize();
	IOCP_API bool get_data(uint32_t* buffer, uint32_t num);
	IOCP_API void begin_save(bool flag);
	IOCP_API bool send_msg(int mode);
	IOCP_API uint32_t get_selfcheck_state_for_qt();
}
