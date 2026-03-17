#include"Engine.h"

/*
Todo:
Use references to avoid races
*/

int main()
{
    Engine engine;

    //Serialise random seed
    srand(static_cast<unsigned>(time(NULL)));

    //Game loop
    while (engine.isOpen())
    {
        engine.Update();
        engine.Render();
    }

    //End of application
    return 0;
}