/*
 * Copyright (c) 2016-20017 Max Cong <savagecm@qq.com>
 * this code can be found at https://github.com/maxcong001/connection_manager
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "logger/logger.hpp"
#include "connManager/connManager.h"
#include "connManager/connInterface.h"
#include "async/scheduler.h"
#include <thread>
#include <unistd.h>

int main()
{
	set_log_level(logger_iface::log_level::debug);
	// start IO service
	IOService::Scheduler &scheduler = IOService::Scheduler::instance();

	std::thread scheduler_thread([&scheduler] {
		std::cout << "start a new thread to run boost io_service!" << std::endl;
		scheduler.run();
		std::cout << "should not run here" << std::endl;
	});

	// now for connection manager
	ConnInfo info;
	info.type = 0;

	connManager<RedisConn<ConnInfo>> *tmp_comm = new connManager<RedisConn<ConnInfo>>();

	tmp_comm->add_pool();
	tmp_comm->add_pool();
	tmp_comm->add_pool();
	tmp_comm->add_conn(info);
	tmp_comm->add_conn(info);

	// disconnect 3 connection
	for (int i = 0; i < 5; i++)
	{
		auto tmp = tmp_comm->get_conn();
		if (tmp == nullptr)
		{
			__LOG(error, "no conn in the list!!");
		}
		else
		{
			__LOG(warn, "got one conection, connection status is " << tmp->get_conn_state());
			tmp->onDisconnected(1);
		}
	}
#if 0
	//	tmp_comm->del_conn(info);
	for (int i = 0; i < 2; i++)
	{
		auto tmp = tmp_comm->get_conn();
		if (tmp == nullptr)
		{
			__LOG(error, "no conn in the list!!");
		}
		else
		{
			std::cout << "got one conection, connection status is " << tmp->get_conn_state() << std::endl;
			tmp->onDisconnected(1);
		}
	}
#endif
	//	now there is one connection
	__LOG(error, "001!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	{
		auto tmp = tmp_comm->get_conn();
		if (tmp == nullptr)
		{
			__LOG(error, "no conn in the list!!");
		}
		else
		{
			std::cout << "got one conection, connection status is " << tmp->get_conn_state() << std::endl;
			//tmp->onDisconnected(1);
		}
	}

	__LOG(error, "002!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	{
		auto tmp = tmp_comm->get_conn();
		if (tmp == nullptr)
		{
			__LOG(error, "no conn in the list!!");
		}
		else
		{
			std::cout << "got one conection, connection status is " << tmp->get_conn_state() << std::endl;
			tmp->onDisconnected(1);
		}
	}

	
	__LOG(error, "003!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");

	for (int i = 0; i < 3; i++)
	{
		auto tmp = tmp_comm->get_conn();
		if (tmp == nullptr)
		{
			__LOG(error, "no conn in the list!!");
		}
		else
		{
			std::cout << "got one conection, connection status is " << tmp->get_conn_state() << std::endl;
			tmp->onDisconnected(1);
		}
	}

	__LOG(warn, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");

	auto tmp = tmp_comm->get_conn();
	if (tmp == nullptr)
	{
		__LOG(error, "no conn in the list!!");
	}
	else
	{
		std::cout << "got one conection, connection status is " << tmp->get_conn_state() << std::endl;
		tmp->onDisconnected(1);
	}

	// wait here
	scheduler_thread.join();
}
