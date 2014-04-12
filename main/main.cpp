#include "StdAfx.h"
#include "ApplicationProcessMonitor.h"
#include "Utility.h"


int main( int argc, char** argv )
{
    Utility::kill_my_brothers();

    ApplicationProcessMonitor().run();

    return 0;
};
