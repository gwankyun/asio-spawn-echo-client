//#include "pch.h"
#include "asio-spawn-echo-client.h"

session_t::session_t(io_context_t &io_context)
	: socket(io_context)
{
}

session_t::~session_t()
{
	INFO("log");
}

void session_t::go()
{
	auto self(shared_from_this());
	asio::spawn(socket.get_io_context(),
		[this, self](yield_context_t yield)
	{
		string message;
		while (std::getline(cin, message))
		{
			INFO("log", message);
			error_code_t ec;
			auto size = socket.async_write_some(asio::buffer(message), yield[ec]);

			if (ec)
			{
				INFO("log", ec.message());
			}
			else
			{
				INFO("log", "async_write_some:{0}", size);

				vector<char> buffer(1024, '\0');
				socket.async_read_some(asio::buffer(buffer), yield[ec]);
				if (ec)
				{
					ERROR("log", ec.message());
					exit(1);
				}
				else
				{
					INFO("log", "async_read_some:{0}", buffer.data());
				}
			}
		}
	});
}

int main()
{
	auto logger = spdlog::stdout_color_mt("log");
	io_context_t io_context;

	asio::spawn(io_context,
		[&io_context](yield_context_t yield)
	{
		error_code_t ec;

		auto session = make_shared<session_t>(io_context);
		auto &socket = session->socket;

		socket.async_connect(endpoint_t(address_t::from_string("127.0.0.1"), 12500), yield[ec]);
		if (ec)
		{
			ERROR("log", ec.message());
			exit(1);
		}
		else
		{
			session->go();
		}
	});

	io_context.run();
}
