// vim: ts=4:sw=4:noexpandtab

#include "dashel/dashel.h"
#include "enki/PhysicalEngine.h"
#include "enki/robots/e-puck/EPuck.h"
#include "viewer/Viewer.h"
#include <QObject>
#include <QApplication>
#include <string>
#include <sstream>
#include <iostream>
#include <cassert>

using namespace std;
using namespace Dashel;
using namespace Enki;


struct TCPInterface: public Hub, public QObject
{
	typedef vector<EPuck*> EPucks;
	EPucks epucks;
	World* world;

	TCPInterface(World* world):
		world(world)
	{
		Hub::connect("tcpin:port=54321");
		startTimer(100);
	}

	virtual void incomingData (Stream *stream)
	{
		// read command
		string cmdLine;
		while(true)
		{
			char c = stream->read<char>();
			if (c == '\n')
				break;
			cmdLine += c;
		}

		// process command
		istringstream iss(cmdLine);
		string cmdName;	iss >> cmdName;
		if (cmdName == "set")
		{
			unsigned robotCount; iss >> robotCount;
			for (unsigned i = 0; i < robotCount; ++i)
			{
				double vr, vl;
				unsigned id;
				double r, g, b;
				iss >> id;
				iss >> vl;
				iss >> vr;
				iss >> r;
				iss >> g;
				iss >> b;
				if (id >= epucks.size())
				{
					cerr << "Error, epuck id " << id << " does not exists, there are " << world->objects.size() << " epucks in the world" << endl;
					continue;
				}
				EPuck* epuck(epucks[id]);
				epuck->leftSpeed = vl;
				epuck->rightSpeed = vr;
				epuck->setColor(Color(r,g,b));
			}
		}
		else
			cerr << "Unknown command " << cmdName << endl;
	}

	void sendPoses()
	{
		for (StreamsSet::iterator it(dataStreams.begin()); it != dataStreams.end(); ++it)
			sendPoses(*it);
	}

	void sendPoses(Stream *stream)
	{
		ostringstream oss;
		oss << "poses ";
		oss << epucks.size() << " ";
		for (size_t i = 0; i < epucks.size(); ++i)
			oss << i << " " << epucks[i]->pos.x << " " << epucks[i]->pos.y << " ";
		oss << "\n";
		stream->write(oss.str().c_str(), oss.str().length());
		stream->flush();
	}

	void timerEvent(QTimerEvent * event)
	{
		step();
		sendPoses();
	}
};

struct MyViewer: public ViewerWidget
{
	MyViewer(World* world):
		ViewerWidget(world)
	{
		altitude = 40;
		pos = QPointF(20,0);
		yaw = 0;
	}
};

int main(int argc, char*argv[])
{
	if (argc < 2)
	{
		cerr << "Error, usage: " << argv[0] << " ROBOT_COUNT" << endl;
		return 1;
	}
	const unsigned robotCount(atoi(argv[1]));

	QApplication app(argc, argv);

	World world;
	TCPInterface tcpInterface(&world);
	for (unsigned i = 0; i < robotCount; ++i)
	{
		EPuck* o(new EPuck);
		o->pos = Vector(0, i*10.-(robotCount-1)*5.);
		world.addObject(o);
		tcpInterface.epucks.push_back(o);
	}
	MyViewer viewer(&world);
	viewer.show();
	
	return app.exec();
}
