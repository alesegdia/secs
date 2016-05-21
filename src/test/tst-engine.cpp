
#include <secs/secs.h>

class Component1
{
public:
	int a, b, c;
};

class Component2
{
public:
	int x, y, z;
};

class Component3
{
public:
	int r, s, t;
};

class MySystem : public secs::EntitySystem
{
public:
	MySystem( secs::EntityProcessor::Ptr processor )
		: m_processor(processor)
	{
		setNeededComponents<Component1, Component2>();
	}

	// EntitySystem interface
	void onAdded(const secs::Entity &e) {}
	void onRemoved(const secs::Entity &e) {}

	void process(const secs::Entity &e)
	{

	}

private:
	secs::EntityProcessor::Ptr m_processor;

};

int main( int argc, char** argv )
{
	secs::Engine world;
	secs::EntityProcessor& processor = world.processor();

	secs::Entity e1 = processor.addEntity();

	Component1& e1c1 = processor.addComponent<Component1>( e1 );
	Component2& e1c2 = processor.addComponent<Component2>( e1 );
	Component3& e1c3 = processor.addComponent<Component3>( e1 );


}
