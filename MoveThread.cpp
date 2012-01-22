#include "MoveThread.h"
#include "PSMoveForm.h"
#include "MainWindow.h"

#include <bluetooth/l2cap.h>
#include <QDateTime>
#include <QElapsedTimer>
#include <QFile>
#include <QSocketNotifier>
#include <QTimer>
#include <QMatrix4x4>
#include <QSemaphore>

#include <QDebug>

MoveThread::MoveThread(QSemaphore *const semaphore, QObject *parent) :
	QThread(parent), mMoveControllerDev(nullptr), mMoveCsk(0), mMoveIsk(0),
	mMoveForm(nullptr), mRumble(0), mPrevMovePressed(false), mSemaphore(semaphore)
{
}

MoveThread::~MoveThread()
{
}

void MoveThread::run()
{
	qDebug() << "move thread started";
	bdaddr_t bdAny = ((bdaddr_t){{0, 0, 0, 0, 0, 0}});
	mCsk = l2cap_listen(&bdAny, L2CAP_PSM_HIDP_CTRL);
	mIsk = l2cap_listen(&bdAny, L2CAP_PSM_HIDP_INTR);
	qDebug() << "csk: " << mCsk;
	qDebug() << "isk: " << mIsk;
	QFile cskFile;
	QFile iskFile;
	bool cskOpened = cskFile.open(mCsk, QIODevice::ReadWrite);
	bool iskOpened = iskFile.open(mIsk, QIODevice::ReadWrite);
	cskFile.waitForReadyRead(-1);
	qDebug() << "csk: bytesavailable" << cskFile.bytesAvailable();
	qDebug() << cskOpened << iskOpened;
	QSocketNotifier cskNot(mCsk, QSocketNotifier::Read);
	QSocketNotifier iskNot(mIsk, QSocketNotifier::Read);
	connect(&cskNot, SIGNAL(activated(int)), this, SLOT(activated(int)));
	connect(&iskNot, SIGNAL(activated(int)), this, SLOT(activated(int)));
	MainWindow *mw = dynamic_cast<MainWindow *>(this->parent());
	//mMoveForm = new PSMoveForm(mw);
	QTimer *timer = new QTimer(this);
	timer->setInterval(300);
	connect(timer, SIGNAL(timeout()), this, SLOT(moveWriteData()));
	timer->start();
	this->exec();
	while (1) {
		qDebug() << "csk: bytesavailable" << cskFile.bytesAvailable();
		qDebug() << "isk: bytesavailable" << iskFile.bytesAvailable();
		//QByteArray cskba = cskFile.read();
		//QByteArray iskba = iskFile.read();
		msleep(500);
	}
	bool nostdin = true;
	fd_set fds;
	int fdmax = 0;
	// connect
	{
		qDebug() << "move" << QDateTime::currentDateTime().toString();
		FD_ZERO(&fds);
		if ( ! nostdin ) {
			FD_SET(0, &fds);
		}
		if ( mCsk >= 0 ) {
			FD_SET(mCsk, &fds);
		}
		if ( mIsk >= 0 ) {
			FD_SET(mIsk, &fds);
		}
		if ( mCsk > fdmax ) {
			fdmax = mCsk;
		}
		if ( mIsk > fdmax ) {
			fdmax = mIsk;
		}
		if ( select(fdmax+1,&fds,NULL,NULL,NULL) < 0 ) {
			qFatal("select");
		}
		// Incoming connection ?
		if ( mCsk>=0 && FD_ISSET(mCsk,&fds) ) {
			mMoveControllerDev = accept_device(mCsk, mIsk);
			setup_device(mMoveControllerDev);
		}
		FD_SET(mMoveControllerDev->csk, &fds);
		if ( mMoveControllerDev->csk > fdmax ) {
			fdmax = mMoveControllerDev->csk;
		}
		FD_SET(mMoveControllerDev->isk, &fds);
		if ( mMoveControllerDev->isk > fdmax ) {
			fdmax = mMoveControllerDev->isk;
		}
		if ( select(fdmax+1,&fds,NULL,NULL,NULL) < 0 ) {
			qFatal("select");
		}
	}
	QElapsedTimer t;
	t.start();
	forever {
		qDebug() << "move forever" << QDateTime::currentDateTime().toString();
		qDebug() << "round trip in" << t.restart();
		bool acquired = mSemaphore->tryAcquire(1, 100);
		if (!acquired) {
			continue;
		}
		if ( select(fdmax+1,&fds,NULL,NULL,NULL) < 0 ) {
			qFatal("select");
		}
		if ( FD_ISSET(mMoveControllerDev->isk, &fds) ) {
			psmove_set_rgb(mMoveControllerDev, mMoveControllerDev->rgb);
		}
		if ( select(fdmax+1,&fds,NULL,NULL,NULL) < 0 ) {
			qFatal("select");
		}
		if ( FD_ISSET(mMoveControllerDev->isk, &fds) ) {
			unsigned char report[256];
			int nr = 1;
			nr = recv(mMoveControllerDev->isk, report, sizeof(report), 0);
			if ( nr <= 0 ) {
				qDebug("%d disconnected\n", mMoveControllerDev->index);
				close(mMoveControllerDev->csk);
				close(mMoveControllerDev->isk);
				delete mMoveControllerDev;
				mMoveControllerDev = nullptr;
			} else {
				if ( true /*verbose*/ ) {
					qDebug() << "RECV" << (int)report[0] << nr;
				}
				if ( report[0] == 0xa1 ) {
					if ( false /*output_binary*/ ) {
						/*if ( ! binary_skip ) {
							write(1, report, nr);
						} else {
							--binary_skip;
						}*/
					} else {
						parse_report(mMoveControllerDev, report+1, nr-1);
					}
					mMoveControllerDev->rgb[1] = report[6];
				}
			}
//            mMoveControllerDev->rgb[0] = qrand() % 255;
//            mMoveControllerDev->rgb[1] = qrand() % 255;
//            mMoveControllerDev->rgb[2] = qrand() % 255;
			//psmove_set_rgb(mMoveControllerDev, mMoveControllerDev->rgb);
		} else {
			qDebug() << "fd not set";
		}
	}
}

void hidp_trans(int csk, const char *buf, int len) {
	/*if ( verbose ) {
		dump("SEND", (unsigned char*)buf, len);
	}*/
	QElapsedTimer timer;
	timer.start();
	if ( send(csk, buf, len, 0) != len ) {
		qFatal("send(CTRL)");
	}
	unsigned char ack;
	int nr = recv(csk, &ack, sizeof(ack), 0);
	/*if ( verbose) {
		fprintf(stderr, "    result %d  %02x\n", nr, ack);
	}*/
	if ( nr!=1 || ack!=0 ) {
		qFatal("ack");
	}
	int msecs = timer.elapsed();
	qDebug() << "sending took" << msecs;
}

void MoveThread::psmove_set_rgb(motion_dev *dev, int rgb[3]) const {
	qDebug() << "setting rgb";
	char report[] = {
		HIDP_TRANS_SET_REPORT | HIDP_DATA_RTYPE_OUTPUT,
		2, 0, (char)rgb[0],(char)rgb[1],(char)rgb[2], 0, 0
	};
	hidp_trans(dev->csk, report, sizeof(report));
}

void MoveThread::psmove_parse_report(unsigned char *r, int len, int latest) const {
	qDebug() << "parsing report";
	if ( r[0] == 0x01 && len >= 49 ) {
		qDebug() << "ok";
		qDebug(" t=%02x %02x", r[5], r[6]);
		/*printf(" seq=%-2d", (r[4]&15)*2+(latest?1:0));
		int ai = latest ? 19 : 13;
		short aX = r[ai+0] + r[ai+1]*256 - 32768;
		short aY = r[ai+2] + r[ai+3]*256 - 32768;
		short aZ = r[ai+4] + r[ai+5]*256 - 32768;
		printf(" aX=%-6d aY=%-6d aZ=%-6d", aX,aY,aZ);
		int ri = latest ? 31 : 25;
		short gX = r[ri+0] + r[ri+1]*256 - 32768;
		short gY = r[ri+2] + r[ri+3]*256 - 32768;
		short gZ = r[ri+4] + r[ri+5]*256 - 32768;
		printf(" gX=%-6d gY=%-6d gZ=%-6d", gX,gY,gZ);
		short mX = (r[38]<<12) | (r[39]<<4);
		short mY = (r[40]<<8)  | (r[41]&0xf0);
		short mZ = (r[41]<<12) | (r[42]<<4);
		mY = - mY;  // Inconsistent sign conventions between acc+gyro and compass.
		printf(" mX=%-5d mY=%-5d mZ=%-5d", mX >> 4, mY >> 4, mZ >> 4);*/
	} else {
		qDebug() << "something terrible" << (int)r[0] << len;
	}
	//printf("\n");
}

void MoveThread::parse_report(motion_dev *dev, unsigned char *r, int len)
{
	char addr[20];
	ba2str(&dev->addr, addr);
	// Two samples per report on accel and gyro axes
	qDebug("%d %s PSMOVE ", dev->index, addr);
	psmove_parse_report(r, len, 0);
	qDebug("%d %s PSMOVE ", dev->index, addr);
	psmove_parse_report(r, len, 1);
	dev->rgb[1] = r[5];
}

int MoveThread::l2cap_listen(const bdaddr_t */*bdaddr*/, unsigned short psm) const
{
	int sock = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
	if (sock < 0) {
		qFatal("socket");
	}

	sockaddr_l2 addr;
	memset(&addr, 0, sizeof(addr));
	addr.l2_family = AF_BLUETOOTH;
	bdaddr_t myBdAddr;
	str2ba("00:0C:76:B0:E3:E0", &myBdAddr);
	addr.l2_bdaddr = ((bdaddr_t) {{0, 0, 0, 0, 0, 0}});
	//addr.l2_bdaddr = myBdAddr;
	addr.l2_psm = htobs(psm);
	if (bind(sock, (const sockaddr *)&addr, sizeof(addr)) < 0) {
		close(sock);
		qFatal("bind");
		return -1;
	}
	char str[19];
	ba2str(&addr.l2_bdaddr, str);
	qDebug() << "using " << str;

	if (listen(sock, 5) < 0) {
		qFatal("listen");
	}
	return sock;
}

motion_dev *MoveThread::accept_device(int csk, int isk) const
{
	qDebug("Incoming connection...\n");
	motion_dev *dev = new motion_dev;
	if ( ! dev ) {
		qFatal("malloc");
	}

	dev->csk = accept(csk, NULL, NULL);
	if ( dev->csk < 0 ) {
		qFatal("accept(CTRL)");
	}
	dev->isk = accept(isk, NULL, NULL);
	if ( dev->isk < 0 ) {
		qFatal("accept(INTR)");
	}

	sockaddr_l2 addr;
	socklen_t addrlen = sizeof(addr);
	if ( getpeername(dev->isk, (sockaddr *)&addr, &addrlen) < 0 ) {
		qFatal("getpeername");
	}
	dev->addr = addr.l2_bdaddr;

	dev->type = motion_dev::PSMOVE;

	return dev;
}

void MoveThread::setup_device(motion_dev *dev) const {
	/*if ( dump_readable ) {
		bluetooth_dump_readable_reports(dev->csk);
	}
	if ( dump_writable ) {
		bluetooth_dump_writable_reports(dev->csk);
	}
	if ( poll_report >= 0 ) {
		bluetooth_poll_report(dev->csk, poll_report);
	}*/

	char addr[20];
	ba2str(&dev->addr, addr);
	qDebug("New device %d %s is a %s\n", dev->index, addr, "PSMOVE");
}

void MoveThread::activated(int fd)
{
	//qDebug() << "activated:" << fd;
	if (fd == mCsk) {
		if (mMoveCsk == 0) {
			mMoveCsk = accept(mCsk, NULL, NULL);
			mMoveIsk = accept(mIsk, NULL, NULL);
			qDebug() << "movecsk:" << mMoveCsk << "moveisk:" << mMoveIsk;
			qDebug() << "move fds:" << mMoveCsk << mMoveIsk;
			qDebug("d\n");
			sockaddr_l2 addr;
			socklen_t addrlen = sizeof(addr);
			if ( getpeername(mMoveIsk, (sockaddr *)&addr, &addrlen) < 0 ) {
				qFatal("getpeername");
			}
			mAddr = addr.l2_bdaddr;
			qDebug() << "accepted";
			QFile f;
			f.open(fd, QIODevice::ReadOnly);
			qDebug() << "bytes available:" << f.bytesAvailable();
			QByteArray ba = f.read(f.bytesAvailable());
			QSocketNotifier *sn = new QSocketNotifier(mMoveCsk, QSocketNotifier::Read);
			//sn->setEnabled(true);
			connect(sn, SIGNAL(activated(int)), this, SLOT(readReport(int)));
			sn = new QSocketNotifier(mMoveIsk, QSocketNotifier::Read);
			mMoveFile.open(mMoveIsk, QIODevice::ReadOnly | QIODevice::Unbuffered);
			//sn->setEnabled(true);
			connect(sn, SIGNAL(activated(int)), this, SLOT(readReport(int)));
		} else {
			//qDebug() << "new report from" << fd;
		}
	}
}

void MoveThread::readReport(int fd)
{
	if (fd == mMoveCsk) {
		qDebug() << "move csk";
	} else {
		//qDebug() << "move isk" << mMoveFile.bytesAvailable();
		QByteArray tmp = mMoveFile.read(50);
		if (tmp.size() == 50 && quint8(tmp.at(0)) == 0xa1) {
			MoveButtons b;
			if (tmp.at(2) & 0x01) {
				b.buttonsPressed.append(MoveButtons::Select);
			}
			if (tmp.at(2) & 0x08) {
				b.buttonsPressed.append(MoveButtons::Start);
			}
			if (tmp.at(3) & 0x10) {
				b.buttonsPressed.append(MoveButtons::Triangle);
			}
			if (tmp.at(3) & 0x20) {
				b.buttonsPressed.append(MoveButtons::Circle);
			}
			if (tmp.at(3) & 0x40) {
				b.buttonsPressed.append(MoveButtons::Cross);
			}
			if (tmp.at(3) & 0x80) {
				b.buttonsPressed.append(MoveButtons::Square);
			}
			if (tmp.at(4) & 0x01) {
				b.buttonsPressed.append(MoveButtons::PS);
			}
			bool movePressed = false;
			if (tmp.at(4) & 0x08) {
				movePressed = true;
				b.buttonsPressed.append(MoveButtons::Move);
			}
			b.trigger = quint8(tmp.at(7));

			const qint16 ax =  tmp.at(20) + tmp.at(21)*256 - 32768;
			const qint16 az = -tmp.at(22) + tmp.at(23)*256 - 32768;
			const qint16 ay =  tmp.at(24) + tmp.at(25)*256 - 32768;
			const QVector3D acc(ax, ay, az);

			const qint16 gx = tmp.at(32) + tmp.at(33)*256 - 32768;
			const qint16 gy = tmp.at(34) + tmp.at(35)*256 - 32768;
			const qint16 gz = tmp.at(36) + tmp.at(37)*256 - 32768;
			const QVector3D gyro(gx, gy, gz);

			const qint16 mx = ((tmp.at(39)<<12) | (tmp.at(40)<<4)) << 4;
			const qint16 mz = ((tmp.at(41)<<8)  | (tmp.at(42)&0xf0)) << 4;
			const qint16 my = ((tmp.at(42)<<12) | (tmp.at(43)<<4)) << 4;
			const QVector3D mag(mx, my, -mz);

			/*if (ax > mMaxAcc.x()) {
				mMaxAcc.setX(ax);
			}
			if (ay > mMaxAcc.y()) {
				mMaxAcc.setY(ay);
			}
			if (az > mMaxAcc.z()) {
				mMaxAcc.setZ(az);
			}
			if (mx > mMaxMag.x()) {
				mMaxMag.setX(mx);
			}
			if (my > mMaxMag.y()) {
				mMaxMag.setY(my);
			}
			if (mz > mMaxMag.z()) {
				mMaxMag.setZ(mz);
			}

			if (ax < mMinAcc.x()) {
				mMinAcc.setX(ax);
			}
			if (ay < mMinAcc.y()) {
				mMinAcc.setY(ay);
			}
			if (az < mMinAcc.z()) {
				mMinAcc.setZ(az);
			}
			if (mx < mMinMag.x()) {
				mMinMag.setX(mx);
			}
			if (my < mMinMag.y()) {
				mMinMag.setY(my);
			}
			if (mz < mMinMag.z()) {
				mMinMag.setZ(mz);
			}*/
			if (movePressed && !mPrevMovePressed) {
				qDebug() << "acc max:" << mMaxAcc << "min:" << mMinAcc;
				qDebug() << "mag max:" << mMaxMag << "min:" << mMinMag;
			}
			mPrevMovePressed = movePressed;

			MoveData d;
			d.buttons = b;
			d.accelerometer = acc;
			d.gyro = gyro;
			d.mag = mag;
			d.battery = tmp.at(13);
			//qDebug() << "orig:" << b.buttonsPressed;
			emit setButtons(b);
			emit dataReceived(d);
		}
		//qDebug() << "read bytes count: " << tmp.size() << tmp.toHex();
	}
}

void MoveThread::setRGB(QColor rgb)
{
	mRGB = rgb;
}

void MoveThread::setRumble(quint8 rumble)
{
	mRumble = rumble;
}

void MoveThread::moveWriteData()
{
	if (mMoveCsk == 0) {
		return;
	}
	quint8 report[] = {
		HIDP_TRANS_SET_REPORT | HIDP_DATA_RTYPE_OUTPUT,
		2, 0, quint8(mRGB.red()), quint8(mRGB.green()), quint8(mRGB.blue()), 0,
		mRumble
	};
	static int w = 0;
	int bytesSent = send(mMoveCsk, report, sizeof(report), 0);
	if (bytesSent != sizeof(report)) {
		qFatal("failure while sending data\n");
	}
	unsigned char ack;
	int nr = recv(mMoveCsk, &ack, sizeof(ack), 0);
	if (nr != 1 || ack != 0) {
		qFatal("ack");
	}
}
