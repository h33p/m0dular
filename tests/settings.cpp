#include <stdio.h>
#include "../utils/settings.h"
#include "../utils/shared_utils.h"
#include "../utils/freelistallocator.h"
#include "../utils/allocwraps.h"

#include <atomic>
#include <thread>
#include <chrono>

typedef std::chrono::high_resolution_clock Clock;

//Pointer proxies prevent the option variables from storing and rechecking the address of the SettingsGroup layer every time it is accessed. Should improve performance
SettingsGroup* globalSettingsPtr = new typename std::decay<decltype(*globalSettingsPtr)>::type();
pointer_proxy<globalSettingsPtr> globalSettings;

SettingsGroup groupASettings;
SettingsGroup groupBSettings;
SettingsGroup* groupSettings = &groupASettings;

SettingsGroup overrideASettings;
SettingsGroup overrideBSettings;
SettingsGroup* overrideSettings = &overrideASettings;

Option<int, "option_A"_crc32, globalSettings> option_A(1);
Option<int, "option_B"_crc32, overrideSettings, groupSettings, globalSettings> option_B(23);

static int EXIT(int ret)
{
	printf("Return: %d\n", ret);
	return ret;
}

int main()
{
	if (option_A != 1)
		return EXIT(1);

	if (option_B != 23)
		return EXIT(2);

	groupSettings = &groupBSettings;

	if (option_A != 1)
		return EXIT(3);

	if (option_B != 23) {
		printf("%d\n", option_B + 0);
		return EXIT(4);
	}

	option_B = 360;

	overrideSettings = &overrideBSettings;

	if (option_A != 1)
		return EXIT(5);

	if (option_B != 23) {
		printf("%d\n", option_B + 0);
		return EXIT(6);
	}

	groupASettings.Set<"option_B"_crc32>((int)993.2);

	if (option_B == 993)
		return EXIT(7);

	groupSettings = &groupASettings;

	if (option_B != 993)
		return EXIT(8);

	int optB = option_B;

	std::vector<unsigned char> buf;
	groupSettings->Serialize(buf);

	if (SettingsGroup(buf).SettingsGroup::Get<int, "option_B"_crc32>() != optB)
		return EXIT(9);

	{
		auto t1 = Clock::now();
		for (volatile int i = 0; i < 10000000; i++) {
			[[maybe_unused]]
			volatile int a = option_B;

			if (i % 10 == 0 && overrideSettings != &overrideASettings)
				overrideSettings = &overrideASettings;
			else
				overrideSettings = &overrideBSettings;

			if (i % 2000 == 0 && groupSettings != &groupASettings)
			    groupSettings = &groupASettings;
			else
			    groupSettings = &groupBSettings;

		}
		auto t2 = Clock::now();

		printf("Time: %ld\n", std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count());
	}

	{
		auto t1 = Clock::now();
		for (volatile int i = 0; i < 10000000; i++) {
			[[maybe_unused]]
			volatile int a = option_A;
		}
		auto t2 = Clock::now();

		printf("Time: %ld\n", std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count());
	}

	return 0;
}
