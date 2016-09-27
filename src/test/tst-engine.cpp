#include <secs/secs.h>
#include <xstf.h>

struct Component1 { uint8_t a; };
struct Component2 { uint16_t a; };
struct Component3 { uint32_t a; };

static size_t SystemA_Counter = 0;
class SystemA : public secs::TypedEntitySystem<Component1, Component2>
{
public:
	void process(float delta, const secs::Entity &e, Component1& c1, Component2& c2 ) override
	{
		c1.a = 0xDE;
		c2.a = 0xDEAD;
		SystemA_Counter++;
	}
};

static size_t SystemB_Counter = 0;
class SystemB : public secs::TypedEntitySystem<Component1, Component3>
{
public:
	void process(float delta, const secs::Entity &e, Component1& c1, Component3& c3 ) override
	{
		c1.a = 0xDE;
		c3.a = 0xDEADBEEF;
		SystemB_Counter++;
	}
};

int main( int argc, char** argv )
{
	secs::Engine world;
	secs::EntityProcessor& processor = world.processor();


	XSTF_TESTING("sECS");
	XSTF_ENABLE_VERBOSE_CHECKS;

	{
		XSTF_TEST("System edits entity component");

		secs::Entity e1 = processor.addEntity();

		Component1& e1c1 = processor.addComponent<Component1>( e1 );
		Component2& e1c2 = processor.addComponent<Component2>( e1 );

		SystemA sa;
		SystemB sb;

		world.pushSystem(&sa);
		world.pushSystem(&sb);

		world.step(0);

		XSTF_ASSERT(e1c1.a == 0xDE, "typed system component modification");
	}

	{
		XSTF_TEST("System removes component");
	}

	return XSTF_TESTING_RESULT;

}
