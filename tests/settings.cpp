#include <stdio.h>
#include "../utils/settings.h"
#include "../utils/shared_utils.h"

#include <atomic>
#include <thread>
#include <chrono>

typedef std::chrono::high_resolution_clock Clock;

SettingsGroup globalSettings;

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

	printf("Serializing Global Settings\n");
	globalSettings.PrintAllValues();
	printf("Serializing Group Settings\n");
	groupSettings->PrintAllValues();
	printf("Serializing Group A Settings\n");
	groupASettings.PrintAllValues();
	printf("Serializing Group B Settings\n");
	groupBSettings.PrintAllValues();
	printf("Serializing Override Settings\n");
	overrideSettings->PrintAllValues();
	printf("Serializing Override A Settings\n");
	overrideASettings.PrintAllValues();
	printf("Serializing Override B Settings\n");
	overrideBSettings.PrintAllValues();

	return 0;
}
