#include <WinSock2.h>
#include "Utility.h"
#include "../LibCore/InfoRecorder.h"

using namespace cg;
using namespace cg::core;


CommandServer::CommandServer(): func_count_(0), size_limit_(Max_Buf_Size) {
	infoRecorder->logTrace("[CommandServer]: constructor, size limit:%d\n", size_limit_);
	//reserve 2 bytes for func_count
	get_cur_ptr(2);

	cache_mgr_ = new Cache();
	set_cache_filter();

	obj_id = -1;

	cr_ = new CommandRecorder();
	config_ = new ServerConfig("game_server.conf");
	config_->load_config();
}

CommandServer::CommandServer(int size_limit): func_count_(0), size_limit_(size_limit) {
	//reserve 2 bytes for func_count

	infoRecorder->logTrace("[CommandServer]: constructor with limited size:%d\n", size_limit);
	get_cur_ptr(2);

	cache_mgr_ = new Cache();
	set_cache_filter();

	obj_id = -1;

	cr_ = new CommandRecorder();
	config_ = new ServerConfig("game_server.conf");
	config_->load_config();
}

CommandServer::~CommandServer() {
	close_socket(connect_socket);
	close_socket(listen_socket);
	clean_up();

	if(cache_mgr_){
		delete cache_mgr_;
		cache_mgr_ = NULL;
	}

	if(cr_){
		delete cr_;
		cr_ = NULL;
	}

	if(config_){
		delete config_;
		config_ = NULL;
	}


}

Buffer* CommandServer::dump_buffer() {
	Buffer* buffer = new Buffer(*this);
	buffer->get_cur_ptr(2);
	return buffer;
}

#ifdef MULTI_CLIENTS
// init the command server with exist socket
void CommandServer::init(SOCKET s){
	infoRecorder->logTrace("[CommandServer]: init the server with socket:%p\n", s);
	this->set_connect_socket(s);
}

#endif

void CommandServer::init() {
	if(!init_socket()) return;

	listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	const char ch_opt = 1;
	int err = setsockopt(listen_socket, IPPROTO_TCP, TCP_NODELAY, &ch_opt, sizeof(char));
	if(err == -1) {
		return;
	}

	SOCKADDR_IN addr_in;
	memset(&addr_in, 0, sizeof(SOCKADDR_IN));
	addr_in.sin_family = AF_INET;
	addr_in.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addr_in.sin_port = htons(config_->command_port_);

	bind(listen_socket, (SOCKADDR*)&addr_in, sizeof(SOCKADDR));
	listen(listen_socket, 5);
}

void CommandServer::start_up() {
	init_socket();
}

void CommandServer::shut_down() {

	infoRecorder->logTrace("[CommandServer]: shut down\n");
	cr_->print_info();

	//send the terminate command
	begin_command(MaxSizeUntilNow_Opcode, 0);
	end_command(1);
}

void CommandServer::accept_client() {
	Accept();
}

void CommandServer::begin_command(int op_code, int obj_id) {
	infoRecorder->logTrace("[CommandServer]: begin command obj id:%d.\n", obj_id);
	infoRecorder->logTrace("[CommandServer]: 2 begin command obj id:%d, this: %p.\n", obj_id, this);
	this->op_code = op_code;
	this->obj_id = obj_id;

	//record current ptr for canceling the command
	infoRecorder->logTrace("[CommandServer]: before get_cur_ptr.\n");
	sv_ptr = get_cur_ptr();

	infoRecorder->logTrace("[CommandServer]: after get_cur_ptr.\n");
#ifdef Enable_Command_Validate
	cm_len_ptr = get_cur_ptr(4);
#endif

	infoRecorder->logTrace("[CommandServer]: before write.\n");
	if(obj_id == 0) {
		write_uchar((op_code << 1) | 0);
	}
	else {
		write_uchar((op_code << 1) | 1);
		write_ushort(obj_id);
	}

#ifdef USE_CACHE
	if(cache_filter[op_code]) {
		infoRecorder->logTrace("[CommandServer]: the cached command: %d.\n", op_code);
		rid_pos = get_cur_ptr(2);
		memset(rid_pos, 0, 2);
	}
	else{

	}
#endif

	func_count_++;
}

void CommandServer::end_command(int force_flush) {
	infoRecorder->logTrace("[CommandServer]: end command.\n");
#ifdef USE_CACHE
	//append or replace cache info
	int hit_id = -1, rep_id = -1;
	if(cache_filter[op_code]) {
		cache_mgr_->get_cache_index(sv_ptr, get_command_length(), hit_id, rep_id);

		if(hit_id != -1) {
#ifdef Enable_Command_Validate
			if(obj_id)
				go_back(sv_ptr + 7);
			else
				go_back(sv_ptr + 5);
#else
			if(obj_id)
				go_back(sv_ptr + 3);
			else
				go_back(sv_ptr + 1);
#endif
			write_ushort((hit_id << 1) | Cache_Use);
			cr_->cache_hit(op_code);
			infoRecorder->logTrace("[CommandServer]: op code %d hit cache.\n", op_code);
		}
		else {
			*( (unsigned short*)rid_pos ) = (rep_id << 1) | Cache_Set;
			cr_->cache_miss(op_code);
			infoRecorder->logTrace("[CommandServer]: op code %d miss cache.\n", op_code);
		}
	}


#endif

#ifdef Enable_Command_Validate
	int cm_len = get_cur_ptr() - sv_ptr;
	memcpy(cm_len_ptr, &cm_len, sizeof(int));
#endif

	if(!force_flush) {
		cr_->add_record(op_code, get_command_length());
	}

	if(force_flush || get_size() >= size_limit_) {
		int len = flush();
		if(len <= 0) {
			infoRecorder->logTrace("CommandServer::end_command(), len <= 0\n");
		}
	}
}

void CommandServer::cancel_command() {
	go_back(sv_ptr);
	func_count_--;
}

int CommandServer::get_command_length() {
	return (get_cur_ptr() - sv_ptr);
}

int CommandServer::flush() {
	infoRecorder->logTrace("[CommandServer]: flush.\n");
	set_count_part(func_count_);
	int len = send_packet(this);

	cr_->add_lengh(len);

	//reset the buffer
	func_count_ = 0;
	clear();
	get_cur_ptr(2);

	return len;
}

void CommandServer::write_vec(int op_code, float* vec, int size) {

#ifdef USE_CACHE

	int hit_id = -1, rep_id = -1;

	cache_mgr_->get_cache_index((char*)vec, size, hit_id, rep_id);
	if(hit_id != -1) {
		write_ushort((hit_id << 1) | Cache_Use);
		cr_->cache_hit(op_code);
	}
	else {
		write_ushort((rep_id << 1) | Cache_Set);
		write_byte_arr((char*)vec, size);
		cr_->cache_miss(op_code);
	}

#else

	write_byte_arr((char *)vec, size);

#endif
}


