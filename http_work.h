#ifndef HTTPWORK_H_
#define HTTPWORK_H_

#ifdef __cplusplus
extern "C"{
#endif

	int socket_create(int port);
	void *socket_accept(void *argc);
	void *http_thread(void *argc);
	void save_log(char *buf);

#ifdef __cplusplus
}
#endif	

#endif
