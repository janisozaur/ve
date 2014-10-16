#ifndef MOVETHREAD_H
#define MOVETHREAD_H

#include "MoveData.h"

#include <QThread>
#include <bluetooth/bluetooth.h>
#include <QFile>
#include <QColor>

class PSMoveForm;
class QSemaphore;

struct motion_dev {
	int index;
	bdaddr_t addr;
	enum { WIIMOTE, SIXAXIS, DS3, PSMOVE } type;
	int csk;
	int isk;
	int rgb[3];  // For PSMOVE
	time_t latest_refresh;  // For PSMOVE. 0 if inactive.
	struct motion_dev *next;
};

#define L2CAP_PSM_HIDP_CTRL 0x11
#define L2CAP_PSM_HIDP_INTR 0x13

#define HIDP_TRANS_GET_REPORT    0x40
#define HIDP_TRANS_SET_REPORT    0x50
#define HIDP_DATA_RTYPE_INPUT    0x01
#define HIDP_DATA_RTYPE_OUTPUT   0x02
#define HIDP_DATA_RTYPE_FEATURE  0x03

class MoveThread : public QThread
{
	Q_OBJECT
public:
	explicit MoveThread(QSemaphore *const semaphore, QObject *parent = 0);
	~MoveThread();

private:
	int l2cap_listen(const bdaddr_t *bdaddr, unsigned short psm) const;
	motion_dev *accept_device(int mCsk, int mIsk) const;
	void setup_device(motion_dev *dev) const;
	void parse_report(motion_dev *dev, unsigned char *r, int len);
	void psmove_parse_report(unsigned char *r, int len, int latest) const;
	void psmove_set_rgb(motion_dev *dev, int rgb[3]) const;

	motion_dev *mMoveControllerDev;
	int mCsk;
	int mIsk;
	int mMoveCsk;
	int mMoveIsk;
	bdaddr_t mAddr;
	QFile mMoveFile;
	PSMoveForm *mMoveForm;
	QColor mRGB;
	quint8 mRumble;
	bool mPrevMovePressed;
	QVector3D mMaxAcc, mMinAcc, mMaxMag, mMinMag;
	QSemaphore * const mSemaphore;

signals:
	void setButtons(MoveButtons buttons);
	void dataReceived(MoveData d);
	void setAxes(QVector3D one, QVector3D two, QVector3D three);
	void startClicked();
	void outputCurrent();

public slots:
	void activated(int fd);
	void readReport(int fd);
	void setRGB(QColor rgb);
	void setRumble(quint8 rumble);
	void moveWriteData();

protected:
	void run();
};

#endif // MOVETHREAD_H
