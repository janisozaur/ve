#ifndef WIIMARKERTRACKER_H
#define WIIMARKERTRACKER_H

#include <QThread>
#include <QPointF>
#include <cwiid.h>

class QSemaphore;

class WiiMarkerTracker : public QThread
{
	Q_OBJECT
public:
	explicit WiiMarkerTracker(QSemaphore *const sem, QObject *parent = 0);
	~WiiMarkerTracker();
	bool connect(const int &timeout = 5);
	void disconnect();

protected:
	void run();

private:
	cwiid_wiimote_t *mWiimote;
	volatile bool mRunning;
	QSemaphore * const mSemaphore;

signals:
	void newPosition(QPointF position);

public slots:

};

#endif // WIIMARKERTRACKER_H
