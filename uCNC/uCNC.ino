#include "src/cnc.h"

void setup()
{
	// put your setup code here, to run once:
	cnc_init();
}

void loop()
{
	// put your main code here, to run repeatedly:
	serial_print_str("run");
	cnc_run();
}
