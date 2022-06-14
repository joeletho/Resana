#pragma once

int main(int argc, char** argv)
{
	RESANA::Log::Init();

	const auto app = RESANA::CreateApplication();
	app->Run();
	delete app;
}