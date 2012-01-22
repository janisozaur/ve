#include "WiiMarkerTracker.h"

#include <QSemaphore>

#include <QDebug>

WiiMarkerTracker::WiiMarkerTracker(QSemaphore *const sem, QObject *parent) :
	QThread(parent), mWiimote(nullptr), mRunning(false), mSemaphore(sem)
{
}

void WiiMarkerTracker::run()
{
	mRunning = connect();
	cwiid_mesg * buffer[CWIID_MAX_MESG_COUNT];

	for (int i = 0; i < CWIID_MAX_MESG_COUNT; i++) {
		buffer[i] = new cwiid_mesg;
	}
	int count;
	timespec timestamp;
	while (mRunning) {
		bool acquired = mSemaphore->tryAcquire(1, 100);
		if (!acquired) {
			continue;
		}
		cwiid_get_mesg(mWiimote, &count, buffer, &timestamp);
		for (int i = 0; i < count; i++) {
			if (buffer[i]->type == CWIID_MESG_IR) {
				for (int j = 0; j < CWIID_IR_SRC_COUNT; j++) {
					if (buffer[i]->ir_mesg.src[j].valid) {
						quint16 x = buffer[i]->ir_mesg.src[j].pos[CWIID_X];
						quint16 y = buffer[i]->ir_mesg.src[j].pos[CWIID_Y];

						float posX = 2.0f * (1.0f - (float)x / (float)CWIID_IR_X_MAX) - 1.0f;
						float posY = -2.0f * (1.0f - (float)y / (float)CWIID_IR_Y_MAX) - 1.0f;
						QPointF p(posX, posY);
						//qDebug() << "new pos" << p;

						emit newPosition(p);

						break;
					}
				}
			}
		}
	}

	for (int i = 0; i < CWIID_MAX_MESG_COUNT; i++) {
		delete buffer[i];
	}
}

bool WiiMarkerTracker::connect(const int &timeout)
{
	bdaddr_t addr;
	qWarning("Connecting to wiimote, press 1+2.\n");
	int result = cwiid_find_wiimote(&addr, timeout);
	if (result != 0) {
		qWarning("Failed to connect with wiimote!\n");
		return false;
	}

	mWiimote = cwiid_open(&addr, CWIID_FLAG_NONBLOCK);
	result = cwiid_command(mWiimote, CWIID_CMD_RPT_MODE, CWIID_RPT_IR);
	if (result != 0) {
		qWarning("Failed to enable IR! disconnecting!\n");
		disconnect();
		return false;
	}
	QTextStream out(stdout);
	out << "Wiimote connected successfully!" << endl;
	return true;
}

void WiiMarkerTracker::disconnect()
{
	if (mWiimote != nullptr) {
		cwiid_close(mWiimote);
	}
}

WiiMarkerTracker::~WiiMarkerTracker()
{
	mRunning = false;
	disconnect();
	wait(5000);
}
